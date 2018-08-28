/*
** ++
**  Package: PGP
**	
**
**  Abstract: PGP BDI Driver.
**
**  Implementation of PGP BDI driver.
**
**  Author:
**      S. Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	000 - November 09, 2015
**
**  Revision History:
**	None.
**
** --
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "elf/linker.h"
#include "memory/mem.h"
#include "memory/resources.h"

#include "sas/Sas.h"

#include "pgp/Pgp.h"
#include "pgp/Bdi.h"

#define SIZEOF_PPI_FRAME   2048  /* plugin firmware max frame length in bytes */

#define RUNDOWN_MSG_MASK   0xFFFFFFFC

#define MAX_RSET_RESOURCES 64

/*
**
**
*/

typedef struct {
 void*           payload;      // Payload pool entry
 uint32_t        size;         // Size of payload == pgp->size
 SAS_MbxId       dst;          // Mailbox to fork a frame (SAS_Fd->mid)
 uint32_t        page;         // Page message for this frame - rundown (SAS_Fd->message)
 Pgp_Frame       pgp;          // PGP frame
} BDI_Fd;

/*
**
**
*/

typedef struct {
 BDI_Page         page;
 struct BDI_Dma  *next;
 uint32_t         ovflow_bytes;
 uint8_t          ovflow[SIZEOF_PPI_FRAME];
} BDI_Dma;

/*
**
**
*/

typedef struct {
 SAS_Session      session;
 SAS_Mbx          mbx;
 BDI_Processor    processor;
 BDI_Arg          arg;
 BDI_Page        *pending;
 uint8_t          link;
 uint8_t          vc;
 uint16_t         status;
 uint32_t         sequence;
 uint32_t         page_size;
 uint64_t         fails;
 uint64_t         pages;
 uint64_t         frames;
 uint64_t         discard;
 uint64_t         dmabytes;
 uint64_t         split_dma;
 uint64_t         split_cp;
 uint64_t         allocs;
 uint64_t         deallocs; 
 uint32_t         max_if_pages;
 uint32_t         use_cached;
 uint32_t         l2;
 BDI_Dma*         dma;
 void            *pool;
 void            *rset;
} Device;

/*
**
**
*/

typedef struct {
 Device          *devices[PGP_NUMOF_LINKS];
} Driver;

static Driver *drivers[PGP_NUMOF_VCS];

/*
** Forward declarations for local functions...
*/

static BDI_Dma*     _paginate(Device *device);
static SAS_IbOpcode _fork(SAS_Frame, SAS_IbMbx, uint32_t);
static uint32_t     _discard(Device*, BDI_Fd*);
static uint32_t     _retire(Device *device, BDI_Fd *fd, uint32_t dmapay, uint32_t rundown);
static Device*      _open(uint32_t link, uint32_t vc, uint32_t size);
static uint32_t     _wait(SAS_Message, void*, SAS_Mbx);
static BDI_Dma*     _alloc(Device *device);
static int          _synch(BDI_Page*, BDI_Arg);

/*
** ++
**
**
** --
*/

int const lnk_options = LNK_INSTALL;

unsigned lnk_prelude(void* arg, Ldr_elf* elf)
  {
  
  bzero(drivers,sizeof(drivers));
  
  return BDI_SUCCESS;
  }

/*
** ++
**
**
** --
*/

static uint32_t _init(uint32_t vc)
  {
  
  Driver *driver = malloc(sizeof(Driver));
  if(!driver) return BDI_INSFMEM;

  bzero(driver,sizeof(Driver));

  drivers[vc] = driver;    

  Pgp_Bind(_fork,vc,vc);
      
  return BDI_SUCCESS;
  }
    
/*
**
**
*/

static Device* _open(uint32_t link, uint32_t vc, uint32_t size)
  {
  
  Driver* driver = drivers[vc];

  Device* device = (Device*)malloc(sizeof(Device));

  if(!device) return (Device*)0; 

  bzero(device,sizeof(Device));

  SAS_Session session  = SAS_Open();

  if(!session) return (Device*)0;
      
  SAS_Mbx mbx = SAS_Bind(_wait, NULL, session);

  if(!mbx) return (Device*)0;

  device->session   = session;            
  device->mbx       = mbx;                
  device->processor = _synch;             
  
  device->link      = link;               
  device->vc        = vc;
  device->page_size = size;
  device->l2        = mem_mapL2Controller();
  
  return device;
  }

/*
**
**
*/

static int _synch(BDI_Page *page, BDI_Arg arg)
  { 
  return 0;  
  } 


/*
**
**
*/

static BDI_Dma* _alloc(Device *device)
  { 
  
  BDI_Dma *dma = (BDI_Dma*)mem_rsAlloc(device->rset);
  if(dma)
    {
    BDI_Page *page = &dma->page;
    
    if(device->use_cached)
      mem_invalidateDataCacheRange((uint32_t)page->data,
                                   (uint32_t)page->data+device->page_size+SIZEOF_PPI_FRAME,
                                    device->l2);
    
    dma->next  = NULL;
    dma->ovflow_bytes = 0;
    device->dma  = dma;
    page->size   = 0;
    page->status = 0;
    page->seq    = 0;

    uint32_t deallocs = device->deallocs;

    if((++device->allocs - deallocs) > device->max_if_pages)
      device->max_if_pages = device->allocs-deallocs;
    }
  else
    {
    ++device->fails;
    device->sequence = 0;
    device->status   = BDI_SEQ;
    }
    
  return dma;
  }

/*
**
**
*/

static BDI_Dma* _paginate(Device *device)
  {
  
  if(device->dma) return device->dma;
  
  if(_alloc(device)) return device->dma;

  ++device->discard;   
  
  return NULL;
  }

/*
**
**
*/

static uint32_t _discard(Device *device, BDI_Fd *fd)
  {
  
  /* when in error, discard active page, flush until start of next PGP frame */
  if((device->status & BDI_SEQ) && !(fd->pgp.hdr.fuser & PGP_SOF))
    {
    if(device->dma)
      {
      /* deallocate and clear device page */
      BDI_Free(&device->dma->page);
      device->dma = NULL;
      }
    
    ++device->discard;
    
    return 1;
    }
    
  return 0;  
  }
  
/*
**
**
*/

static SAS_IbOpcode _fork(SAS_Frame frame, SAS_IbMbx mbx, uint32_t arg)
  {

  BDI_Fd* fd = (BDI_Fd*)SAS_IbFd(frame, mbx);

  uint32_t link = SAS_TypeOf(frame);
  
  uint32_t vc = arg;
  
  Driver* driver = drivers[vc];

  uint32_t dmapay = SAS_IbPayload(frame);
  
  if(!driver)
    return dmapay ? SAS_IB_FLUSH : SAS_IB_FREE;  
    
  Device* device = driver->devices[link];
  
  if(!device)
    return dmapay ? SAS_IB_FLUSH : SAS_IB_FREE;
    
  ++device->frames;  
  
  /* discard frame under error conditions */
  if(_discard(device,fd))
    return dmapay ? SAS_IB_FLUSH : SAS_IB_FREE;
  
  /* discard frame for page allocation failure */
  BDI_Dma *dma = _paginate(device);
  if(!dma)
    return dmapay ? SAS_IB_FLUSH : SAS_IB_FREE;

  BDI_Page *page = &dma->page;
    
  /* establish dma pointer for page data */
  fd->payload = (void*)&page->data[page->size];
  
  /* setup page sequence flags */  
  if(fd->pgp.hdr.fuser & PGP_SOF)
    {
    device->sequence = 0;
    page->status |= BDI_FIRST;
    }
    
  if(fd->pgp.hdr.ctl & PGP_EOF)
    page->status |= BDI_LAST;
          
  /* setup current page overflow */
  if((page->size+fd->pgp.hdr.size) > device->page_size)
    {
    device->dma = NULL;
    
    BDI_Dma *next_dma = _paginate(device);
    if(!next_dma)
      return dmapay ? SAS_IB_FLUSH : SAS_IB_FREE;

    /* at the completion of this dma, copy overflow bytes to overflow buffer */
    dma->next = (struct BDI_Dma*)next_dma;
    next_dma->page.size = (page->size+fd->pgp.hdr.size)-device->page_size;
    ++device->split_dma;    
    }

  page->size += fd->pgp.hdr.size;
  
  /* fork at full page or EOF frame */
  uint32_t fork = 
      ((page->size >= device->page_size) || 
       (fd->pgp.hdr.ctl & PGP_EOF)) ? 1 : 0;
    
  if(fork)
    {
    fd->page       = (uint32_t)dma; // rundown message
        
    page->seq      = device->sequence++;
    page->link     = link;
    page->vc       = vc;
    page->status  |= device->status;
        
    if(!dma->next)
      device->dma  = NULL;
      
    device->status = 0;
    
    if(fd->pgp.hdr.ctl & PGP_EOF)
      device->sequence = 0;
    }
  
  return(_retire(device, fd, dmapay, fork));  
  }
  
/*
**
**
*/

static uint32_t _retire(Device *device, BDI_Fd *fd, uint32_t dmapay, uint32_t rundown)
  {
        
  fd->dst = SAS_Id(device->mbx); // rundown mailbox id
    
  if(dmapay)
    {
    fd->size          = fd->pgp.hdr.size;
    device->dmabytes += fd->size;
    
    return (rundown ? SAS_IB_PAYLOAD_RUNDOWN : SAS_IB_PAYLOAD); 
    }
  else
    {
    fd->size = 0;        
    memcpy(fd->payload,&fd->pgp.payload[0],fd->pgp.hdr.size);
        
    return (rundown ? SAS_IB_RUNDOWN : SAS_IB_FREE);
    }
  } 
    
/*
**
**
*/

static uint32_t _wait(SAS_Message message, void* unused, SAS_Mbx mbx)
  {
        
  BDI_Dma *dma = (BDI_Dma*)(message & RUNDOWN_MSG_MASK);
  BDI_Page *page = &dma->page;

  Driver* driver = drivers[page->vc];
  
  Device* device = driver->devices[page->link];

  /* memcpy from temporary storage to current page */
  if(dma->ovflow_bytes)
    {
    memcpy(page->data,dma->ovflow,dma->ovflow_bytes);
    dma->ovflow_bytes = 0;
    ++device->split_cp;
    }

  /* memcpy overflow to next page temporary storage */
  if(dma->next)
    {
    BDI_Dma *next = (BDI_Dma*)dma->next;
    memcpy(next->ovflow,&page->data[device->page_size],page->size-device->page_size);
    next->ovflow_bytes = page->size-device->page_size;
    page->size = device->page_size;
    }

  device->pending = page;

  ++device->pages;
  
  return (*device->processor)(page, device->arg) ? SAS_REENABLE : SAS_DISABLE | SAS_ABORT;
  }

/*
**
**
*/

BDI_Page* BDI_Wait(uint32_t vc, uint32_t link)
  {

  if(vc >= PGP_NUMOF_VCS) return NULL;

  if(link >= PGP_NUMOF_LINKS) return NULL;
  
  Driver *driver = drivers[vc];
  
  if(!driver) return NULL;
  
  Device* device = driver->devices[link];
  
  if(!device) return NULL;
 
  SAS_EnableWait(device->session);
  
  return device->pending;
  }

/*
**
**
*/

uint32_t BDI_Open(uint32_t vc,
                  uint32_t link,
                  uint32_t pages,
                  uint32_t size,
                  uint32_t cached)
  {
  
  Driver *driver;
  Device *device;
  uint32_t status;
  uint32_t alloc_size;
  BDI_Dma *dma[MAX_RSET_RESOURCES];
  int i;

  if(vc >= PGP_NUMOF_VCS) 
    return BDI_VCERR;

  if(link >= PGP_NUMOF_LINKS) 
    return BDI_LNKERR;

  if(pages > MAX_RSET_RESOURCES)
    return BDI_INSFMEM;
  
  /* initialize the virtual channel driver */
  if(!drivers[vc])
    {
    status = _init(vc);
    if(status) return status;
    }
    
  driver = drivers[vc]; 
  device = _open(link,vc,size);
  if(!device)
    return BDI_INSFMEM;

  driver->devices[link] = device;

  /* allocate extra space for dma page header */
  alloc_size = sizeof(BDI_Dma);

  uint32_t pow = 31-__builtin_clz(alloc_size);
  uint32_t stride = (alloc_size%(1<<pow)) ? pow+1 : pow;

  device->pool = (void*)rtems_heap_allocate_aligned_with_boundary(pages*(1<<stride), 0x20, 0);

  if(!device->pool)
    return BDI_INSFMEM;

  device->rset = mem_rsOpen(pages, device->pool, stride);

  if(!device->rset)
    {
    mem_rsClose(device->rset);
    return BDI_RSERR;  
    }

  /* allocate extra space for overflow frame */
  alloc_size = size + SIZEOF_PPI_FRAME;

  if(cached) device->use_cached = 1;
  for(i=0;i<pages;i++)
    {
    dma[i] = (BDI_Dma*)mem_rsAlloc(device->rset);    
    if(!dma[i])
      return BDI_RSERR;
    
    if(cached)
      dma[i]->page.data = (void*)rtems_heap_allocate_aligned_with_boundary(alloc_size, 0x20, 0);
    else
      dma[i]->page.data = (void*)mem_Region_alloc(MEM_REGION_UNCACHED, alloc_size);
            
    if(!dma[i]->page.data)
      return BDI_INSFMEM;
    }

  for(i=0;i<pages;i++)
    mem_rsFree(device->rset, dma[i]);
    
  return BDI_SUCCESS;
  }

/*
**
**
*/

void BDI_Bind(uint32_t vc, uint32_t link, BDI_Processor processor, BDI_Arg arg)
  {
  if(vc >= PGP_NUMOF_VCS) return;

  if(link >= PGP_NUMOF_LINKS) return;
  
  Driver *driver = drivers[vc];
  if(!driver) return;
  
  Device *device = driver->devices[link];
  if(device) return;
  
  device->processor = processor;
  device->arg       = arg;

  return;
  }
 
/*
**
**
*/

void BDI_Free(BDI_Page *page)
  {
  
  Driver *driver = drivers[page->vc];

  Device* device = driver->devices[page->link];
    
  mem_rsFree(device->rset, page);
  
  ++device->deallocs;
      
  return; 
  }

/*
**
**
*/

void BDI_Close(void)
  {
  
  BDI_Dma *dma;
  int i,j;
  
  for(i=0;i<PGP_NUMOF_VCS;i++)
    {
    Driver *driver = drivers[i];
    if(driver)
      {
      Pgp_Unbind(i);

      for(j=0;j<PGP_NUMOF_LINKS;j++)
        {  
        Device *device = driver->devices[j];
        if(device)
          {
          SAS_Close(device->session);
          while(dma = (BDI_Dma*)mem_rsAlloc(device->rset))
            {
            if(device->use_cached)
              free(dma->page.data);
            }
          mem_rsClose(device->rset);
          free(device->pool);
          free(device);
          }
        }
      
      free(driver);  
      drivers[i] = NULL;
      }
    }
  return; 
  }

/*
**
**
*/

void BDI_Reset(uint32_t vc, uint32_t link)
  {
  if(vc >= PGP_NUMOF_VCS) return;
  
  if(!drivers[vc]) return;
    
  if(link >= PGP_NUMOF_LINKS) return;
  
  Driver *driver = drivers[vc];
  
  Device *device = driver->devices[link];
  
  if(!device) return;

  device->allocs       = 0;
  device->deallocs     = 0;
  device->pages        = 0;
  device->max_if_pages = 0;
  device->fails        = 0;
  device->dmabytes     = 0;
  device->frames       = 0;
  device->discard      = 0;  
  device->split_dma    = 0;  
  device->split_cp     = 0;  
  }

/*
**
**
*/

static const char _line0[]   = "BDI driver statistics\n";
static const char _line1[]   = "  Link %d VC%d\n";
static const char _line2[]   = "    Allocated pages     %llu\n";
static const char _line3[]   = "    Deallocated pages   %llu\n";
static const char _line4[]   = "    Processed pages     %llu\n";
static const char _line5[]   = "    Max inflight pages  %u\n";
static const char _line6[]   = "    Allocation failures %llu\n";
static const char _line7[]   = "    Dma bytes           %llu\n";
static const char _line8[]   = "    Received frames     %llu\n";
static const char _line9[]   = "    Discarded frames    %llu\n";
static const char _line10[]  = "    Frame splits (dma)  %llu\n";
static const char _line11[]  = "    Frame splits (cp)   %llu\n";

void BDI_Dump(uint32_t vc, uint32_t link)
  {
  if(vc >= PGP_NUMOF_VCS) return;
  
  if(!drivers[vc]) return;
        
  if(link >= PGP_NUMOF_LINKS) return;

  Driver *driver = drivers[vc];
  
  Device* device = driver->devices[link];
  
  if(!device) return;

  printf(_line0);
  printf(_line1, device->link,device->vc); 
  printf(_line2, device->allocs);
  printf(_line3, device->deallocs);
  printf(_line4, device->pages);
  printf(_line5, device->max_if_pages);
  printf(_line6, device->fails);
  printf(_line7, device->dmabytes);  
  printf(_line8, device->frames);
  printf(_line9, device->discard);  
  printf(_line10,device->split_dma);  
  printf(_line11,device->split_cp);  
  
  }
