// -*-Mode: C;-*-
/**
@file
@brief ArchLinux kernel module for access to high-mapped OCM (as ARM normal cached shared memory).

This module creates the character device /dev/ocm which may be used
with mmap() to map the 256 KB On-Chip Memory into process address space
as normal, cached and shared memory. Using /dev/mem doesn't work
for us as it will map the OCM, or any location above the top of RAM,
as uncached device memory. We need normal memory for LDREX and STREX
to work, cacheing for efficient access and shareability
for coherency among processors and firmware using the ACP.

@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

// No end-user docs for this code.
/** @cond development */

/*
When loaded this module automatically creates the character device
/dev/ocm using alloc_chrdev_region(), class_create(), cdev_init(),
cdev_add(), and device_create().

*/

#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mutex.h>


#define DRIVER_AUTHOR "Stephen Tether <tether@slac.stanford.edu>"
#define DRIVER_DESC   "Character device driver used to mmap() OCM."
#define DEVICE_NAME   "ocm"
#define CLASS_NAME    "rce"

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_SUPPORTED_DEVICE(DEVICE_NAME);

static int ocm_open      (struct inode *, struct file *);
static int ocm_release   (struct inode *, struct file *);
static int ocm_mmap      (struct file *,  struct vm_area_struct *);

static struct file_operations const fops = {
  .owner   = THIS_MODULE,
  .open    = ocm_open,
  .release = ocm_release,
  .mmap    = ocm_mmap
};


static struct class*  ocm_class  = NULL;
static dev_t          ocm_devnos = 0;
static struct cdev    ocm_cdev   = {.owner = NULL};
static struct device* ocm_device = NULL;
static int            ocm_done   = 0;


static void ocm_undo(void) {
  switch (ocm_done) {
  case 4: device_destroy(ocm_class, ocm_devnos);
  case 3: cdev_del(&ocm_cdev);
  case 2: class_destroy(ocm_class);
  case 1: unregister_chrdev_region(ocm_devnos, 1);
  case 0: ;
  }
}


static int __init ocm_init(void)
{
  int stat = 0;
  /* Get a major device no. and a range of minor ones. */
  { dev_t dev;
    stat = alloc_chrdev_region(&dev, 0/* First minor no.*/, 1/*No. of devices*/, DEVICE_NAME);
    if (stat < 0) {printk(KERN_WARNING "alloc_chrdev_region() failed.\n");}
    else          {ocm_devnos = MKDEV(MAJOR(dev), 0); stat = 0;}
  }

  if (!stat) {
    ocm_done = 1;
    /* Create a device class. */
    ocm_class = class_create(THIS_MODULE, "rce");
    if (IS_ERR(ocm_class)) {
      printk(KERN_WARNING "class_create() failed.\n");
      stat = PTR_ERR(ocm_class);
    }
  }

  if (!stat)  {
    ocm_done = 2;
    cdev_init(&ocm_cdev, &fops);
    ocm_cdev.owner = THIS_MODULE;
    stat = cdev_add(&ocm_cdev, ocm_devnos, 1);
    if (stat) printk(KERN_WARNING "cdev_add() failed.\n");
  }

  if (!stat) {
    ocm_done = 3;
    ocm_device = device_create(ocm_class,
                                  NULL,       // No parent device.
                                  ocm_devnos, // Create /dev entry for major, minor.
                                  NULL,       // No private data.
                                  DEVICE_NAME);
    if (IS_ERR(ocm_device)) {
      printk(KERN_WARNING "device_create() failed.\n");
      stat = PTR_ERR(ocm_device);
    }
  }

  if (!stat) {
    ocm_done = 4;
    printk(KERN_ALERT "/dev/%s is device %d, %d.\n",
           DEVICE_NAME, MAJOR(ocm_devnos), MINOR(ocm_devnos));
  }

  if (stat) ocm_undo();
  return stat;
}

static void __exit ocm_cleanup(void) {
  ocm_undo();
  printk(KERN_ALERT "/dev/%s is de-registered and the module is unloaded.\n", DEVICE_NAME);
}


module_init(ocm_init);
module_exit(ocm_cleanup);


static int ocm_open(struct inode *inode, struct file *filp) {
  return 0;
}


static int ocm_release(struct inode *inode, struct file *filp) {
  return 0;
}


#define OCM_PHYSADDR 0xfffc0000UL
#define OCM_SIZE     65536UL

/* Protection bits we don't want to meddle with. */
#define PROTECTION_MASK (L_PTE_VALID | L_PTE_DIRTY | L_PTE_USER)

/* This mmap() implementation ignores the offset and flags arguments;
   it always starts a shared mapping at the beginning of the OCM. The
   user will still have to use the right size argument because the OS
   uses it to determine how large a virtual address range to allocate
   for the user's process, and that happens before this function is
   called.  Private mappings aren't allowed; silly COW. */
static int ocm_mmap(struct file *filp,  struct vm_area_struct *vma) {
  unsigned long const size = vma->vm_end - vma->vm_start;
  if (size != OCM_SIZE) {
    printk(KERN_WARNING "Invalid OCM size given to mmap().\n");
    return -EINVAL;
  }
  vma->vm_page_prot = (vma->vm_page_prot & PROTECTION_MASK) | L_PTE_MT_WRITEALLOC | L_PTE_XN | L_PTE_SHARED;
  if(remap_pfn_range(vma, vma->vm_start, OCM_PHYSADDR >> PAGE_SHIFT, OCM_SIZE, vma->vm_page_prot)) {
    printk(KERN_WARNING "remap_pfn_range() failed for OCM.\n");
    return -EAGAIN;
  }
  return 0;
}

// End of region excluded from user-level docs.
/** @endcond */
