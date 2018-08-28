// -*-Mode: C;-*-
/**
@file
@brief ArchLinux kernel module for Zynq which manages DMA buffers.

This module creates the character device /dev/dmabuf which may be used
with ioctl() to allocate or free DMA buffers which are accessible from user
code via mmap(). The buffers will be physically contiguous, uncached,
page-aligned and an integral number of pages long.

A successful allocation sets both the DMA address and the size in the
buffer descriptor since the requested size will be rounded up to an
integral number of pages. Allocation may fail due to a lack of space
or a bad file descriptor.

Any process may use an allocated buffer by mapping it into the process
address space using mmap(). mmap() requires the size and DMA address
of the buffer. Mapping will fail of the address and size don't match
an existing buffer or if the fd is invalid. If a
process spawns a child using fork() or clone() then the child process
will inherit all of the parent's DMA buffer mappings at the same
virtual addresses without having to use mmap().

If a process doesn't need a buffer any more it can use munmap() to
remove the buffer from its virtual address space. If a process
terminates then any DMA buffers it has are implicitly unmapped.

The dmabuf module maintains a count of the number of mappings in use
for each buffer. Each successful call to mmap() of a given buffer or
the inheritance of a mapping increases the count by one. Each
unmapping of a buffer, implicit or explicit, decreases the count by
one.

A buffer whose mapping count is zero may be freed by using ioctl() as
shown above. The operation will fail if the fd is bad, if no buffer
matches the descriptor or if the matching buffer is still mapped into
some process.

Buffers last until explicitly freed. One can outlast the process that
created it. You just have to make sure to preserve the information in
the buffer descriptor so that processes can map the buffer.

If this module is unloaded then any existing buffers will be forgotten
and become unusable, resulting in a memory leak which can be rectified
only by rebooting.

If you have trouble allocating the DMA buffer space you need, you can
try using the kernel boot-time command-line parameter "cma". For
example "cma=16M" reserves 16 MB for DMA buffers.

Doxygen documentation for the public DMA buffer API is at located at the end
of this file.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

// Don't want the module code appearing in user-level docs.
/** @cond development */

/*
When loaded this module automatically creates the character device
/dev/dmabuf using alloc_chrdev_region(), class_create(), cdev_init(),
cdev_add(), and device_create().

The DMA address mask is set for 32-bit DMA addresses using
dma_set_coherent(). On the Zynq, DMA addresses are just ordinary
physical addresses which will fit inside a void* or an unsigned long;
there's no IOMMU and devices using DMA have no access to the
CPU MMUs either.

The module allocates and frees DMA buffers using
dma_allocate_coherent() and dma_free_coherent(), respectively. Each
extant buffer has an descriptor on a kernel list which records reference
count, physical address and size in bytes. Access to the list is
serialized with a spinlock.

The module maintains a many-to-one mapping of virtual memory areas
(VMAs) to DMA buffers. Each successful mmap() call creates a new VMA
with the private data pointer of the vm_area_struct set to the address
of the descriptor of the buffer that was mapped.

*/
#define DRIVER_AUTHOR "Stephen Tether <tether at slac.stanford.edu>"
#define DRIVER_DESC   "Character device driver used to allocate DMA buffers via mmap()."
#define DEVICE_NAME   "dmabuf"
#define CLASS_NAME    "rcedma"

#define DEBUG

#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/dma-mapping.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>

#include <asm-generic/uaccess.h>

/* A local copy of or link to tool/memory/armCA9-linux/dmabuf.h. */
#include "dmabuf.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_SUPPORTED_DEVICE(DEVICE_NAME);



/* Operations called by the kernel. */
/* Module management. */
static int  __init       dmabuf_init        (void);
static void __exit       dmabuf_cleanup     (void);
/* File operations. */
static int               dmabuf_open        (struct inode*, struct file *);
static int               dmabuf_release     (struct inode*, struct file *);
static long              dmabuf_ioctl       (struct file*,  unsigned, unsigned long);
static int               dmabuf_mmap        (struct file*,  struct vm_area_struct *);
/* Operations on Virtual Memory Areas. */
static void              dmabuf_vmopen      (struct vm_area_struct*);
static void              dmabuf_vmclose     (struct vm_area_struct*);

/* Operations used internally. */
typedef struct DmaBuffer DmaBuffer;        /* Buffer descriptor (internal). */
static DmaBuffer* dmabuf_lookup      (dma_addr_t, unsigned long);
static void       dmabuf_attach      (DmaBuffer      * pbuf);
static void       dmabuf_detach      (DmaBuffer      * pbuf);
static int        dmabuf_allocate    (DmaBuffer     ** pbuf, unsigned long);
static void       dmabuf_free        (DmaBuffer const* pbuf);
static int        dmabuf_mapBuffer   (DmaBuffer      * pbuf, struct vm_area_struct*);


/* Implement file operations open(), close(), ioctl() and mmap(). We
   use unlocked_ioctl only since we expect both the kernel and the
   userland code to be 32-bit.

*/
static struct file_operations const fops = {
  .owner           = THIS_MODULE,
  .open            = dmabuf_open,
  .release         = dmabuf_release,
  .unlocked_ioctl  = dmabuf_ioctl,
  .mmap            = dmabuf_mmap
};



/* Handle fork(), clone(), munmap() and exit(). */
static struct vm_operations_struct const dmabuf_vmops = {
  .open  = dmabuf_vmopen,
  .close = dmabuf_vmclose
};



/* Implement module init and finish. */
module_init(dmabuf_init);
module_exit(dmabuf_cleanup);




/* Kernel information related to the device. */
static struct class*  dmabuf_class  = NULL;
static dev_t          dmabuf_devnos = 0;               /* Major and minor device nos. */
static struct cdev    dmabuf_cdev   = {.owner = NULL}; /* Char device handle. */
static struct device* dmabuf_device = NULL;            /* Low-level device handle. */



/* The kernel-list of internal buffer descriptors and its spinlock.*/

static DEFINE_SPINLOCK(dmabuf_dataLock);

LIST_HEAD(dmabuf_buffers);

struct DmaBuffer {
  struct list_head  list;
  unsigned          refCount;   /* How many mappings of this buffer are live? */
  void*             vaddress;   /* The kernel virtual address of the buffer. */
  dma_addr_t        dmaAddress; /* The physical address of the buffer. */
  unsigned long     size;       /* Buffer size in bytes. */
};



/* Progress through module initialization. */
static int dmabuf_done   = 0;



/* Undo module initialization steps. */
static void dmabuf_undo(void) {
  switch (dmabuf_done) {
  case 4: device_destroy(dmabuf_class, dmabuf_devnos);
  case 3: cdev_del(&dmabuf_cdev);
  case 2: class_destroy(dmabuf_class);
  case 1: unregister_chrdev_region(dmabuf_devnos, 1);
  case 0: ;
  }
}



static int __init dmabuf_init(void)
{
  int stat = 0;
  /* Get a major device no. and one minor one. Adds a new line to /proc/devices. */
  { dev_t dev;
    stat = alloc_chrdev_region(&dev, 0/* First minor no.*/, 1/*No. of devices*/, DEVICE_NAME);
    if (stat < 0) {
      pr_warning("%s %s: alloc_chrdev_region() failed.\n", CLASS_NAME, DEVICE_NAME);
    }
    else {
      dmabuf_devnos = MKDEV(MAJOR(dev), 0);
      stat = 0;
    }
  }

  /* Create the device class which is visible in userspace as /sys/class/CLASS_NAME/. */
  if (stat >= 0) {
    dmabuf_done = 1;
    dmabuf_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(dmabuf_class)) {
      pr_warning("%s %s: class_create() failed.\n", CLASS_NAME, DEVICE_NAME);
      stat = PTR_ERR(dmabuf_class);
    }
  }

  /* Create the character device which is visible in userspace as
     /sys/class/CLASS_NAME/DEVICE_NAME/ and its contents. */
  if (stat >= 0)  {
    dmabuf_done = 2;
    cdev_init(&dmabuf_cdev, &fops);
    dmabuf_cdev.owner = THIS_MODULE;
    stat = cdev_add(&dmabuf_cdev, dmabuf_devnos, 1);
    if (stat) pr_warning("%s %s: cdev_add() failed.\n", CLASS_NAME, DEVICE_NAME);
  }

  /* Create the device node in /dev. Actually, this sends a message to the udev
     daemon udevd which does the work. The node will persist even after the
     module is removed and if it already exists when the module is loaded it will
     be re-used. */
  if (stat >= 0) {
    dmabuf_done = 3;
    dmabuf_device = device_create(dmabuf_class,
                                  NULL,       // No parent device.
                                  dmabuf_devnos, // Create /dev entry for major, minor.
                                  NULL,       // No private data.
                                  DEVICE_NAME);
    if (IS_ERR(dmabuf_device)) {
      pr_warning("%s %s: device_create() failed.\n", CLASS_NAME, DEVICE_NAME);
      stat = PTR_ERR(dmabuf_device);
    }
  }

  if (stat >= 0) {
    dmabuf_done = 4;
    dev_notice(dmabuf_device,
             "Major dev. no. %d, minor %d.\n", MAJOR(dmabuf_devnos), MINOR(dmabuf_devnos));
  }

  if (stat >= 0) {
    stat = dma_set_coherent_mask(dmabuf_device, DMA_BIT_MASK(32));
    if (stat) dev_warn(dmabuf_device, "32-bit DMA is not available.\n");
  }

  if (stat < 0) dmabuf_undo();
  return stat;
}

static void __exit dmabuf_cleanup(void) {
  dmabuf_undo();
  pr_notice("%s %s: Module unloaded.\n", CLASS_NAME, DEVICE_NAME);
}



/* Called when a process opens the device. */
static int dmabuf_open(struct inode *inode, struct file *filp) {
  dev_dbg(dmabuf_device, "Open.\n");
  return 0;
}


/* Called when a process has closed its last fd referring to the device. */
static int dmabuf_release(struct inode *inode, struct file *filp) {
  dev_dbg(dmabuf_device, "Release.\n");
  return 0;
}


/* Called when a process calls ioctl() on our device. */
static long dmabuf_ioctl(struct file* filp, unsigned op, unsigned long arg) {
  /* Valid operation? */
  if (_IOC_TYPE(op) != DMA_K_MAGIC) return -ENOTTY;
  if (_IOC_NR(op)   >  DMA_K_FREE)  return -ENOTTY;

  /* get_user() and put_user() do access checking and are optimized
     for data items 1, 2, 4 or 8 bytes long.
  */
  switch (op) {
    dma_BufferDesc   data;
    DmaBuffer* pbuf;
    int        status;

  case DMA_K_ALLOCATE:
    if (  0 > (status=get_user(data, (dma_BufferDesc __user*)arg))  ) return status;
    if (  0 > (status=dmabuf_allocate(&pbuf, data.size))  )     return status;
    /* Begin critical section. */
    spin_lock(&dmabuf_dataLock);
    dmabuf_attach(pbuf);
    spin_unlock(&dmabuf_dataLock);
    /* End critical section. */
    data.dmaAddress = pbuf->dmaAddress;
    data.size       = pbuf->size;
    if (  0 > (status=put_user(data, (dma_BufferDesc __user*)arg))  ) return status;
    return 0;
    break;

  case DMA_K_FREE:
    status = 0;
    if (  0 > (status=get_user(data, (dma_BufferDesc __user*)arg))  ) return status;
    /* Begin critical section. */
    spin_lock(&dmabuf_dataLock);
    pbuf = dmabuf_lookup(data.dmaAddress, data.size);
    if      (!pbuf             ) status = -EINVAL;
    else if (0 < pbuf->refCount) status = -EAGAIN;
    else {
      dmabuf_detach(pbuf);
    }
    spin_unlock(&dmabuf_dataLock);
    /* End critical section. */
    if (status < 0)                         return status;
    dmabuf_free(pbuf);
    return 0;
    break;

  default: /* Should not get here. */
    return -ENOTTY;
  }
}


/* Called when a VMA mapped using this device is copied to a new
   process by fork() or clone(). The copy has the same private data
   and user space virtual address as the original so we just need to
   increment the buffer's reference count.

   The reference count can't be zero when the critical section is
   entered since the original VMA has not yet been closed, assuming
   that this function is allowed to run to completion BEFORE fork() or
   clone() returns to the parent process. A non-zero reference count
   ensures that the buffer can't be freed out from under us.

   The assumption about fork()/clone() seems to be one we can depend
   on since System V shared-memory segments maintain reference counts
   in just this way and that code has been around for a good long
   while.

*/
static void dmabuf_vmopen(struct vm_area_struct* vma) {
  DmaBuffer* pbuf = (DmaBuffer*)vma->vm_private_data;
  if (!pbuf) {
    dev_err(dmabuf_device, "Missing DMA buffer descriptor in a VMA.\n");
    return;
  }
  /* Begin critical section. */
  spin_lock  (&dmabuf_dataLock);
  ++pbuf->refCount;
  spin_unlock(&dmabuf_dataLock);
  /* End critical section. */
  dev_dbg(dmabuf_device, "VM open.\n");
}


/* Called when a process unmaps a buffer by calling munmap() or by
   exiting. The VMA referring to the buffer is about to be destroyed
   so we need to decrement the buffer's reference count. We must also
   remove the VMA's pointer to the buffer descriptor or else the
   kernel will attempt to reference it (to deallocate it?) when the
   VMA is destroyed.

   The reference count can't be zero upon entry to the critical
   section since the reference by the given VMA still exists. By the
   same token the buffer can't be freed until we leave the critical
   section.
*/
static void dmabuf_vmclose(struct vm_area_struct* vma) {
  DmaBuffer* pbuf = (DmaBuffer*)vma->vm_private_data;
  if (!pbuf) {
    dev_err(dmabuf_device, "Missing DMA buffer descriptor in a VMA.\n");
    return;
  }

  dev_dbg(dmabuf_device, "VM close. Old ref count (%p) = %u.\n", pbuf, pbuf->refCount);
  /* Begin critical section. */
  spin_lock  (&dmabuf_dataLock);
  --pbuf->refCount;
  spin_unlock(&dmabuf_dataLock);
  /* End critical section. */
  dev_dbg(dmabuf_device, "New ref count (%p) = %u.\n", pbuf, pbuf->refCount);
  vma->vm_private_data = NULL;
}



/* Protection bits we don't want to meddle with. */
#define PROTECTION_MASK (L_PTE_VALID | L_PTE_DIRTY | L_PTE_USER)

/* mmap() implementation.

   The flags argument of mmap() is ignored. The mapping is always
   given the flags VM_DONTEXPAND, VM_DONTDUMP, VM_READ, VM_WRITE and
   VM_SHARED.

   Inside the first critical section we search the list of descriptors
   for a match of the dmaAddress and the size of the buffer,
   incrementing the reference count the matching descriptor if
   any. Incrementing the count will prevent the buffer from being
   freed while we're trying to map it.

   In case the mapping fails we need a second critical section which
   will undo the reference count increment.
*/
static int dmabuf_mmap(struct file *filp,  struct vm_area_struct *vma) {
  /* The offset argument given to mmap() is the DMA address of the
     desired buffer, which we recover from the page number offset in
     the VMA.
  */
  DmaBuffer const key = {
    .dmaAddress = (dma_addr_t)(vma->vm_pgoff << PAGE_SHIFT),
    .size       = vma->vm_end - vma->vm_start
  };
  DmaBuffer* pbuf = NULL;
  int status = 0;

  /* Begin critical section. */
  spin_lock  (&dmabuf_dataLock);
  pbuf = dmabuf_lookup(key.dmaAddress, key.size);
  if (pbuf) ++pbuf->refCount;
  spin_unlock(&dmabuf_dataLock);
  /* End critical section. */
  if (!pbuf) return -EINVAL;

  status = dmabuf_mapBuffer(pbuf, vma);

  if (status < 0) {
    /* Begin critical section. */
    spin_lock  (&dmabuf_dataLock);
    --pbuf->refCount;
    spin_unlock(&dmabuf_dataLock);
    /* End critical section. */
    return status;
  }

  dev_dbg(dmabuf_device,
           "Mapped buffer (%p) size %lu, physaddr 0x%08x, ref count %u.\n",
           pbuf,
           pbuf->size,
           pbuf->dmaAddress,
           pbuf->refCount);

  return 0;
}



/* Create page table entries for the requesting process that mark the
   DMA buffer as uncached RAM. A time-consuming operation that may
   even block, so it mustn't be called from within a critical section.

   If the operation succeeds we make the VMA refer to the descriptor
   of the mapped buffer and set the VMA open and close callbacks to
   the ones we define in this module.
*/
static int dmabuf_mapBuffer(DmaBuffer* pbuf, struct vm_area_struct* vma) {
  int status = 0;

  if (!pbuf) {
    dev_err(dmabuf_device, "Null pointer to buffer descriptor.\n");
    return -EINVAL;
  }

  vma->vm_flags = VM_DONTEXPAND | VM_DONTDUMP | VM_READ | VM_WRITE | VM_SHARED;
  vma->vm_page_prot
    = (vma->vm_page_prot & PROTECTION_MASK) | L_PTE_MT_UNCACHED | L_PTE_XN | L_PTE_SHARED;
  status = remap_pfn_range
    (vma,
     vma->vm_start,
     virt_to_phys(pbuf->vaddress) >> PAGE_SHIFT,
     vma->vm_end - vma->vm_start,
     vma->vm_page_prot
     );
  if (status < 0) {
    dev_warn(dmabuf_device, "remap_pfn_range() failed for DMA buffer.\n");
  }
  else {
    vma->vm_private_data = pbuf;
    vma->vm_ops = &dmabuf_vmops;
  }
  return status;
}



static int dmabuf_allocate(DmaBuffer **ppbuf, unsigned long size) {
  DmaBuffer* pbuf = vmalloc(sizeof(DmaBuffer));

  *ppbuf = NULL;
  if (!pbuf) {
    dev_warn(dmabuf_device, "Allocation of DMA buffer descriptor failed.\n");
    return -EAGAIN;
  }

  /* Must round up the size to be compatible with mmap(). */
  { unsigned long const rdown = (size>>PAGE_SHIFT) << PAGE_SHIFT;
    if (size > rdown) size = rdown + (1U << PAGE_SHIFT);
  }

  pbuf->refCount = 0;
  pbuf->size     = size;
  pbuf->vaddress = dma_alloc_coherent(dmabuf_device,
                                      pbuf->size,
                                      &pbuf->dmaAddress,
                                      GFP_KERNEL);
  if (!pbuf->vaddress) {
    dev_warn(dmabuf_device, "Allocation of DMA buffer failed.\n");
    vfree(pbuf);
    return -EAGAIN;
  }
  dev_dbg(dmabuf_device,
           "New buffer (%p) size %lu, physaddr 0x%08x, ref count %u.\n",
           pbuf,
           pbuf->size,
           pbuf->dmaAddress,
           pbuf->refCount);
  *ppbuf = pbuf;
  return 0;
}



/* Find the descriptor with the given DMA address and size; NULL if none. */
static DmaBuffer* dmabuf_lookup(dma_addr_t dmaAddress, unsigned long size) {
  struct list_head* pos;
  list_for_each(pos, &dmabuf_buffers) {
    DmaBuffer* const pbuf = list_entry(pos, DmaBuffer, list);
    if (pbuf->dmaAddress == dmaAddress && pbuf->size == size) return pbuf;
  }
  return NULL;

}

/* Remove a buffer descriptor from the list. */
static void dmabuf_detach(DmaBuffer* pbuf) {
  list_del(&pbuf->list);
}


/* Add a new entry to the front of the list. */
static void dmabuf_attach(DmaBuffer* pbuf) {
  list_add(&pbuf->list, &dmabuf_buffers);
}


static void dmabuf_free(DmaBuffer const* pbuf) {
  dma_free_coherent(dmabuf_device,
                    pbuf->size,
                    pbuf->vaddress,
                    pbuf->dmaAddress);
  vfree(pbuf);
}

// End of region excluded from user-level docs.
/** @endcond */

// ========== Public API documentation ==========

/** @class dma_BufferDesc
    @brief Descriptor for a DMA buffer. 

    This structure contains the properties inherent in the buffer
    itself and which are used to identify the buffer. It's independent
    of process-specific properties such as the virtual address
    mapping.

    @typedef dma_BufferDesc
    @brief Make the enum name a type name also (required only for C code).

    @var dma_BufferDesc::dmaAddress
    @brief Physical address of start of buffer

    @var dma_BufferDesc::size
    @brief Buffer size in bytes

*/

/** @name ioctl() command numbers for /dev/dmabuf
    @{
    @var DMA_K_MAGIC
    @brief Key identifying the device driver
    @details See Documentation/ioctl/ioctl-number.txt in the kernel source tree.
    I've picked 'c' as the magic number since it seems to be used only
    on x86 and s390 systems, not on ARM.

    @var DMA_K_ALLOCATE
    @brief Op code: allocate a buffer

    @var DMA_K_FREE
    @brief Op code: free a buffer
    @}
*/



/** @fn int dma_open()
    @brief Open the DMA buffer device and get a file descriptor.
    @return A valid fd or -1 with errno set.

    The file descriptor so obtained obeys the usual rules concerning
    inheritance by child processes. You can get several different file
    descriptors for the same process though there seems to be no
    reason to do so.
*/

/** @fn void dma_close(int fd)
    @brief Close a file descriptor gotten from dma_open()
    @param[in] fd the file descriptor
*/

/** @fn int dma_allocateBuffer(int fd, dma_BufferDesc* desc)
    @brief Allocate a DMA buffer. 
    @param[in] fd  A file descriptor gotten from dma_open().
    @param[in,out] desc A pointer to a buffer descriptor. Before
    calling, set the size field of the descriptor to the desired size
    in bytes of the buffer.
    @retval 0 on success
    @retval -1 on failure with errno set.
    An errno value of EAGAIN means that there wasn't enough
    DMA buffer space available. EINVAL or ENOTTY mean a bad file
    descriptor or one not obtained from dma_open().

    Once allocated a DMA buffer will persist until the dmabuf kernel
    module is unloaded, the system is rebooted, or the buffer is freed
    using dma_freeBuffer(). The information placed in the descriptor,
    DMA address and size, is required to identify the buffer in calls
    to this API.

    DMA buffers are always physically contiguous.
*/

/** @fn int dma_freeBuffer(int fd, const dma_BufferDesc* desc)
    @brief Free a DMA buffer. 
    @param[in] fd  A file descriptor gotten from dma_open().
    @param[in] desc A pointer to a buffer descriptor that
    was prepared by dma_allocateBuffer().
    @retval 0 on success
    @retval -1 on failure with errno set.
    An errno value of EINVAL means that the buffer doesn't
    exist or the fd is invalid. ENOTTY means that the fd was not
    obtained from dma_open().

    If the call succeeds then the buffer descriptor becomes
    invalid.
*/



/** @fn dma_mapBuffer(int fd, const dma_BufferDesc* desc)
    @brief Map a DMA buffer into the process address space. 
    @param[in] fd  A file descriptor gotten from dma_open().
    @param[in] desc A pointer to a buffer descriptor that
    was prepared by dma_allocateBuffer().
    @retval A valid pointer on success
    @retval NULL on failure with errno set.
    An errno value of EINVAL means that the buffer doesn't
    exist or the fd is invalid. ENOTTY means that the fd was not
    obtained from dma_open().

    If the call succeeds then the buffer becomes accessible at
    the virtual address returned. Buffers are always mapped as uncached.

    @note If a process has a buffer mapped at a given virtual address
    and then spawns a child process, the child process will inherit
    the mapping at the same virtual address unless the user specifies
    otherwise in the system call that makes the new process.  The
    inherited mapping is independent of the original and the new
    process may pass it on to a child of its own or unmap it without
    affecting the original mapping.

    @note It's possible to have multiple mappings of the same buffer
    active at the same time in the same process though it seems a
    waste of process address space and an invitation to confusion.

*/


/** @fn dma_unmapBuffer(const dma_BufferDesc* desc, void* vaddr)
    @brief Unmap a DMA buffer that was mapped by dma_mapBuffer().
    @param[in] desc A pointer to a buffer descriptor that
    was prepared by dma_allocateBuffer().
    @param[in] vaddr The virtual address returned by dma_mapBuffer().
    @retval 0 on success
    @retval -1 on failure with errno set.
    An errno value of EINVAL means that the buffer doesn't exist, that
    the buffer wasn't mapped or that the fd is invalid.  ENOTTY means
    that the fd was not obtained from dma_open().

    If the call succeeds then the buffer becomes inaccessible and
    the virtual addresses it was occupying in the process
    become invalid.
*/
