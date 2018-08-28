/*
** ++
**  Package: PGP
**	
**
**  Abstract: PGP BDI Emulator Driver (BDIEM).
**
**  Implementation of PGP BDI Emulator driver.
**
**  Author:
**      S. Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	000 - Feb 09, 2017
**
**  Revision History:
**	None.
**
** --
*/

#include <stdlib.h>
#include <stdio.h>
#include <rtems.h>

#include "elf/linker.h"
#include "memory/mem.h"
#include "memory/resources.h"
#include "svt/Svt.h"
#include "sas/Sas.h"

#include "pgp/Pgp.h"
#include "pgp/Bdi.h"
#include "pgp/BdiEm.h"

#define HEADER_SIZE        1

#define SIZEOF_PPI_FRAME   2048  /* plugin firmware max frame length in bytes */

#define RUNDOWN_MSG_MASK   0xFFFFFFFC

#define MAX_RSET_RESOURCES 32

#define MAX_RSET_POOLS 1 /* use up to 2 page pools, 1 each allocated from non-cached then cached regions */

#define NONCACHED_POOL 0 /* this pool index is allocated from non-cached memory */
#define CACHED_POOL    1 /* this pool index is allocated from cached memory */

#define MAX_LINKS      6 /* max of 3 links can be emulated on 2 virtual channels */

#define RSET_MASK 0x1

#define PAGE_FREE 0x10
#define PAGE_POST 0x20

#define PAGE_EOF_BYTES 2

#define PLUGIN_PREFS_NAME "SAS_PLUGIN_PREFS"

/*
**
**
*/

typedef struct {
 void*           payload;      // Payload pool entry
 uint32_t        size;         // Size of payload == pgp->size
 SAS_MbxId       dst;          // Mailbox to fork a frame (SAS_Fd->mid)
 BDI_Page       *page;         // Page message for this frame - rundown (SAS_Fd->message)
 Pgp_Frame       pgp;          // PGP header words
} BDIEM_Fd;

/*
**
**
*/

typedef struct {
 uint8_t          link;
 uint8_t          vc;
 uint16_t         status;
 uint32_t         page_size;
 uint64_t         fails;
 uint64_t         post_fails;
 uint64_t         pages;
 uint64_t         posted;
 uint64_t         frames;
 uint64_t         discard;
 uint64_t         dmabytes;
 uint64_t         allocs;
 uint64_t         deallocs;
 uint32_t         max_if_pages;
 uint32_t         rset_cur;
 uint32_t         rset_next;
 uint32_t         stage_idx[MAX_RSET_POOLS];
 uint32_t         l2;
 BDI_Page*        stage[MAX_RSET_POOLS][MAX_RSET_RESOURCES];
 BDI_Page*        page;
 SAS_Session      session;
 SAS_ObMbx        mbx;
 void            *pool[MAX_RSET_POOLS];
 void            *rset[MAX_RSET_POOLS];
} Device;

/*
**
**
*/

typedef struct {
 Device          *devices[PGP_NUMOF_LINKS];
} Driver;

/*
** Forward declarations for local functions...
*/

static BDI_Page*    _alloc(Device *device);
static BDI_Page*    _paginate(Device *device);
static void         _post(Device *device, BDI_Page *page);
static void         _dealloc(Device *device, BDI_Page *page);
static void         _clear(Device *device);
static void         _wait(Device *device);
static void         _launch(Device *device);
static void         _stage(Device* device, BDI_Page *page);
static Device*      _open(uint32_t link, uint32_t vc, uint32_t size);
static SAS_IbOpcode _fork(SAS_Frame, uint32_t vc, SAS_IbMbx);
static uint32_t     _discard(Device *device, BDIEM_Fd* fd);
static uint32_t     _complete(SAS_Message, void *ctx, SAS_ObMbx);

/*
**
**
*/

static const char PLUGIN_PREFS[] = PLUGIN_PREFS_NAME;  // Name of SAS plugin prefs

static const char *_Plugins[PGP_NUMOF_PLUGINS] = {NULL,NULL,NULL};

static Driver *_Drivers[PGP_NUMOF_VCS];

static uint32_t _Links = 0;

/*
** ++
**
**
** --
*/

int const lnk_options = LNK_INSTALL;

unsigned lnk_prelude(void* arg, Ldr_elf* elf)
  {

  SAS_Preferences **prefs;
    
  /* find the list of plugins to bind */
  SAS_Preferences **list = (SAS_Preferences**)Svt_Translate(PLUGIN_PREFS, SVT_SAS_TABLE);
  if(!list) return BDI_SVTERR;
  
  /* bind PGP plugins to _Drivers */
  for(prefs=list;*prefs;prefs++)
    {
    if((!strncmp((*prefs)->name,PGP0_PLUGIN_NAME,sizeof(PGP0_PLUGIN_NAME))) ||
       (!strncmp((*prefs)->name,PGP1_PLUGIN_NAME,sizeof(PGP1_PLUGIN_NAME))) ||
       (!strncmp((*prefs)->name,PGP2_PLUGIN_NAME,sizeof(PGP2_PLUGIN_NAME))))
      {
      uint32_t sock = strtoul((*prefs)->bitfile,0,0);
      if(sock < PGP_NUMOF_PLUGINS)
        _Plugins[sock] = (*prefs)->name;
      }
    }
      
  bzero(_Drivers,sizeof(Driver*)*PGP_NUMOF_VCS);
  
  return BDI_SUCCESS;
  }

/*
**
**
*/

static void _wait(Device *device)
  {

  /* wait for dma completion */
  while(1)
    SAS_EnableWait(device->session);
  }

/*
**
**
*/

static const char DEV0[] = "0";
static const char DEV1[] = "1";
static const char DEV2[] = "2";
static const char DEV3[] = "3";
static const char DEV4[] = "4";
static const char DEV5[] = "5";

static const char *_devids[] = {DEV0,DEV1,DEV2,DEV3,DEV4,DEV5};

static void _launch(Device *device)
  {  
          
  uint32_t tid;
  rtems_status_code sc;
  
  sc = rtems_task_create(rtems_build_name('B','E','M',*_devids[_Links++]),
                         80,
                         RTEMS_MINIMUM_STACK_SIZE * 10,
                         RTEMS_DEFAULT_ATTRIBUTES,
                         RTEMS_DEFAULT_MODES,
                         &tid);
  if (sc != RTEMS_SUCCESSFUL)
    rtems_panic("%s Can't create task: %s", __func__,rtems_status_text(sc));

  sc = rtems_task_start(tid, (rtems_task_entry)_wait, (int)device);
  if (sc != RTEMS_SUCCESSFUL)
    rtems_panic("%s Can't start task: %s", __func__,rtems_status_text(sc));
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

  _Drivers[vc] = driver;   
              
  return BDI_SUCCESS;
  }
    
/*
**
**
*/

static Device* _open(uint32_t link, uint32_t vc, uint32_t size)
  {

  Device* device = (Device*)malloc(sizeof(Device));

  if(!device) return (Device*)0; 

  bzero(device,sizeof(Device));

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

static BDI_Page* _alloc(Device *device)
  { 
  
  BDI_Page *page = (BDI_Page*)mem_rsAlloc(device->rset[device->rset_cur]);
  if(page)
    {
    device->page = page;

    page->size   = 0;
    page->status = 0;
    
    /* SEMTODO: this is a kludgy repurposing of an unused field */
    page->seq    = device->rset_cur;

    uint32_t deallocs = device->deallocs;

    if((++device->allocs - deallocs) > device->max_if_pages)
      device->max_if_pages = device->allocs-deallocs;
    }
  else
    {
    ++device->fails;
    device->status   = BDI_SEQ;
    }
    
  return page;
  }

/*
**
**
*/

static BDI_Page* _paginate(Device *device)
  {
  
  if(device->page) return device->page;
  
  if(_alloc(device)) return device->page;

  ++device->discard;   
  
  return NULL;
  }

/*
**
**
*/

static uint32_t _discard(Device *device, BDIEM_Fd *fd)
  {
  
  /* when in error, discard active page, flush until start of next PGP frame */
  if((device->status & BDI_SEQ) && !(fd->pgp.hdr.fuser & PGP_SOF))
    {
    if(device->page)
      {
      /* deallocate and clear device page */
      _dealloc(device,device->page);
      device->page = NULL;
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

static void _stage(Device *device, BDI_Page *page)
  {

  uint32_t cur = device->rset_cur;
  uint32_t *idx = &device->stage_idx[cur];

  /* stage page for post */
  device->stage[cur][(*idx)++] = page;

  /* toggle rset for page allocation */
  if((page->status & PGP_EOF) && (MAX_RSET_POOLS > 1))
    device->rset_next = device->rset_cur ^ 1;

  /* increment count of completed pages */
  ++device->pages;
  }
    
/*
**
**
*/

static uint32_t _complete(SAS_Message message, void* ctx, SAS_ObMbx mbx)
  {
  
  uint32_t ret = SAS_REENABLE;
        
  BDI_Page *page = (BDI_Page*)(message & RUNDOWN_MSG_MASK);

  Driver* driver = _Drivers[page->vc];
  
  Device* device = driver->devices[page->link];
  
  /* stage page for post, toggle rset for page allocation */
  _stage(device,page);
    
  return ret;
  }

/*
**
**
*/

static const char _post_error[] = "BDIEM frame alloc failure\n";

static void _post(Device *device, BDI_Page *page)    
  {

  SAS_ObMbx mbx   = device->mbx;
  
  SAS_Frame frame = SAS_ObAlloc(mbx); 
  if(!frame)
    {
    ++device->fails;
    printf(_post_error);
    return;
    }

  BDIEM_Fd* fd = (BDIEM_Fd*)SAS_ObFd(frame, mbx);

  fd->pgp.hdr.ctl   = 0;
  fd->pgp.hdr.fuser = 0;
  fd->pgp.hdr.luser = 0;
  
  if(page->status & PGP_SOF)
    fd->pgp.hdr.fuser = PGP_SOF;
    
  if(page->status & PGP_EOF)
    fd->pgp.hdr.ctl = PGP_EOF;    
    
  fd->pgp.hdr.dst = page->vc;
  
  fd->payload   = page->data;
  fd->size      = page->size;
  
  /* SEMTODO: do not run down outbound dma, fire and forget */
  SAS_ObPost(SAS_OB_PAYLOAD, SAS_ObSet(frame, PGP_DECODE_LANE(page->link), HEADER_SIZE), mbx);

  ++device->posted;

  return; 
  }

/*
**
**
*/

static void _dealloc(Device *device, BDI_Page *page)
  {
  
  uint32_t rset = page->seq & RSET_MASK;
  
  mem_rsFree(device->rset[rset], page);
  
  ++device->deallocs;
      
  return; 
  }

/*
**
**
*/

static void _clear(Device *device)
  {
  
  uint32_t cur = device->rset_cur;
  uint32_t *idx = &device->stage_idx[cur];
  
  /* deallocate all staged pages for current rset, then clear stage */  
  while(*idx > 0)
    _dealloc(device,device->stage[cur][--(*idx)]);
  
  bzero(device->stage[cur],sizeof(device->stage[cur]));
  
  return; 
  }

/*
**
**
*/

static uint32_t _sas(Device *device)
  {
  
  device->session = SAS_Open();

  if(!device->session)
    return BDI_SASERR;

  device->mbx = SAS_ObBind(_Plugins[PGP_DECODE_SOCK(device->link)], _complete, NULL, device->session);

  if(!device->mbx)
    return BDI_SASERR;

  return BDI_SUCCESS;  
  }

/*
**
**
*/

uint32_t BDIEM_Open(uint32_t vc,
                    uint32_t link,
                    uint32_t pages,
                    uint32_t size)
  {
  
  Driver *driver;
  Device *device;
  uint32_t status;
  uint32_t socket = PGP_DECODE_SOCK(link);
  uint32_t alloc_size;
  BDI_Page *page[MAX_RSET_RESOURCES];
  int i,p;

  if(vc >= PGP_NUMOF_VCS) 
    return BDI_VCERR;

  if(link >= PGP_NUMOF_LINKS) 
    return BDI_LNKERR;

  if(pages > MAX_RSET_RESOURCES)
    return BDI_INSFMEM;

  if((_Links+1) > MAX_LINKS)
    return BDI_LNKERR;
    
  /* initialize the virtual channel driver */
  if(!_Drivers[vc])
    {
    status = _init(vc);
    if(status) return status;
    }
    
  driver = _Drivers[vc];
  
  /* link driver can only be opened once */
  if(driver->devices[link])
    return BDI_LNKERR;
    
  device = _open(link,vc,size);
  if(!device)
    return BDI_INSFMEM;

  driver->devices[link] = device;

  /* allocate extra space for page header */
  alloc_size = sizeof(BDI_Page);

  uint32_t pow = 31-__builtin_clz(alloc_size);
  uint32_t stride = (alloc_size%(1<<pow)) ? pow+1 : pow;

  for(p=0;p<MAX_RSET_POOLS;p++)
    {
    device->pool[p] = (void*)rtems_heap_allocate_aligned_with_boundary(pages*(1<<stride), 0x20, 0);

    if(!device->pool[p])
      return BDI_INSFMEM;

    device->rset[p] = mem_rsOpen(pages, device->pool[p], stride);

    if(!device->rset[p])
      {
      mem_rsClose(device->rset[p]);
      return BDI_RSERR;  
      }

    /* allocate extra space for overflow frame */
    alloc_size = size + SIZEOF_PPI_FRAME;

    for(i=0;i<pages;i++)
      {
      page[i] = (BDI_Page*)mem_rsAlloc(device->rset[p]);    
      if(!page[i])
        return BDI_RSERR;

      /* SEMTODO: kludgy, allocate max from each region to support 2 rset pools */
      if(p == CACHED_POOL)
        page[i]->data = (void*)rtems_heap_allocate_aligned_with_boundary(alloc_size, 0x20, 0);
      else if(p == NONCACHED_POOL)
        page[i]->data = (void*)mem_Region_alloc(MEM_REGION_UNCACHED, alloc_size);

      if(!page[i]->data)
        return BDI_INSFMEM;
      }

    for(i=0;i<pages;i++)
      {
      mem_rsFree(device->rset[p], page[i]);
      }
   }

  status = _sas(device);
  if(status) return status;

  /* launch the page dma completion task */
  _launch(device);  
        
  return BDI_SUCCESS;
  }

/*
**
**
*/

uint32_t BDIEM_IbFrame(SAS_Frame frame, void *ctx, SAS_IbMbx mbx)
  {

  BDIEM_Fd* fd = (BDIEM_Fd*)SAS_IbFd(frame, mbx);

  uint32_t link = SAS_TypeOf(frame);
    
  uint32_t vc = fd->pgp.hdr.dst;
  
  Driver* driver = _Drivers[vc];

  uint32_t dmapay = SAS_IbPayload(frame);
          
  Device* device = driver->devices[link];
  
  ++device->frames;  
  
  /* discard frame under error conditions */
  if(_discard(device,fd))
    return dmapay ? SAS_IB_FLUSH : SAS_IB_FREE;

  /* discard frame for size errors */
  if(fd->pgp.hdr.size > SIZEOF_PPI_FRAME)
    {
    ++device->discard;
    return dmapay ? SAS_IB_FLUSH : SAS_IB_FREE;  
    }
  
  /* setup rset for page allocation and clear stage */
  if(fd->pgp.hdr.fuser & PGP_SOF)
    {
    device->rset_cur = device->rset_next;
    _clear(device);
    }
  
  /* discard frame for page allocation failure */
  BDI_Page *page = _paginate(device);
  if(!page)
    return dmapay ? SAS_IB_FLUSH : SAS_IB_FREE;
    
  /* establish dma pointer for page data */
  fd->payload = (void*)&page->data[page->size];
  
  page->size += fd->pgp.hdr.size;
    
  /* setup page sequence flags */  
  if(fd->pgp.hdr.fuser & PGP_SOF)
    page->status |= PGP_SOF;
    
  if(fd->pgp.hdr.ctl & PGP_EOF)
    {
    page->status |= PGP_EOF;
    
    /* SEMTODO: emulate extra bytes produced by front end */
    page->size   += PAGE_EOF_BYTES;
    }
    
  /* post at full page or EOF frame */
  uint32_t post = 
      ((page->size >= device->page_size) || 
       (fd->pgp.hdr.ctl & PGP_EOF)) ? 1 : 0;
    
  if(post)
    {    
    fd->page       = page; // rundown message
    fd->dst        = SAS_ObId(device->mbx); // rundown mailbox id
        
    page->link     = link;
    page->vc       = vc;
    page->status  |= device->status;
        
    device->page   = NULL;
    device->status = 0;
    }
      
  if(dmapay)
    {
    fd->size          = fd->pgp.hdr.size;
    device->dmabytes += fd->size;
    
    return (post ? SAS_IB_PAYLOAD_RUNDOWN : SAS_IB_PAYLOAD); 
    }
  else
    {
    fd->size = 0;        
    memcpy(fd->payload,&fd->pgp.payload[0],fd->pgp.hdr.size);
        
    return (post ? SAS_IB_RUNDOWN : SAS_IB_FREE);
    }
  }

/*
**
**
*/

void BDIEM_Process(uint32_t vc,
                   uint32_t link,
                   uint32_t flags,
                   uint32_t size,
                   void    *data)
  {
  
  Driver* driver = _Drivers[vc];
        
  Device* device = driver->devices[link];
        
  ++device->frames;

  /* discard frame for size errors */
  if(size > SIZEOF_PPI_FRAME)
    {
    ++device->discard;
    return;  
    }

  /* setup rset for page allocation and clear stage */
  if(flags & PGP_SOF)
    {
    device->rset_cur = device->rset_next;
    _clear(device);
    }
  
  /* discard frame for page allocation failure */
  BDI_Page *page = _paginate(device);
  if(!page) return;
    
  /* setup page sequence flags */  
  page->status |= flags;
  
  /* SEMTODO: performance hit, copy data to page buffer */
  memcpy(&page->data[page->size],data,size);

  page->size += size;
  
  /* SEMTODO: emulate extra bytes produced by front end */
  if(flags & PGP_EOF)
    page->size += PAGE_EOF_BYTES;
  
  /* post at full page or EOF frame */
  uint32_t post = 
      ((page->size >= device->page_size) || 
       (flags & PGP_EOF)) ? 1 : 0;
    
  if(post)
    {
    page->link     = link;
    page->vc       = vc;
    page->status  |= device->status;
        
    device->page   = NULL;
    device->status = 0;

    /* SEMTODO: kludgy, the cached pool must be flushed prior to post */
    if(device->rset_cur == CACHED_POOL)
      {
      /* flush the entire page from the cache */
      mem_storeDataCacheRange((uint32_t)page->data,
                              (uint32_t)page->data+device->page_size,
                               device->l2);
      }

    /* stage page for post, toggle rset allocator */
    _stage(device,page);
    }
  }

/*
**
**
*/

void BDIEM_Post(uint32_t vc, uint32_t link)
  {
  
  int i;
  
  Device *device = _Drivers[vc]->devices[link];

  uint32_t cur = device->rset_cur;
  uint32_t *idx = &device->stage_idx[cur];
  
  if(!(*idx)) return;
  
  /* post all staged pages for current rset */
  for(i=0; i<(*idx);i++)
    _post(device,device->stage[cur][i]);
  }

/*
**
**
*/

void BDIEM_Reset(uint32_t vc, uint32_t link)
  {
  
  if(vc >= PGP_NUMOF_VCS) return;
  
  if(!_Drivers[vc]) return;
    
  if(link >= PGP_NUMOF_LINKS) return;
  
  Driver *driver = _Drivers[vc];
  
  Device *device = driver->devices[link];
  
  if(!device) return;

  device->allocs        = 0;
  device->deallocs      = 0;
  device->pages         = 0;
  device->posted        = 0;
  device->post_fails    = 0;
  device->max_if_pages  = 0;
  device->fails         = 0;
  device->dmabytes      = 0;
  device->frames        = 0;
  device->discard       = 0; 
  }

/*
**
**
*/

static const char _line0[]   = "BDIEM driver statistics\n";
static const char _line1[]   = "  Link %d VC%d\n";
static const char _line2[]   = "    Allocated pages     %llu\n";
static const char _line3[]   = "    Deallocated pages   %llu\n";
static const char _line4[]   = "    Processed pages     %llu\n";
static const char _line5[]   = "    Posted pages        %llu\n";
static const char _line6[]   = "    Post failures       %llu\n";
static const char _line7[]   = "    Max inflight pages  %u\n";
static const char _line8[]   = "    Allocation failures %llu\n";
static const char _line9[]   = "    Dma bytes           %llu\n";
static const char _line10[]  = "    Received frames     %llu\n";
static const char _line11[]  = "    Discarded frames    %llu\n";

void BDIEM_Dump(uint32_t vc, uint32_t link)
  {
  
  if(vc >= PGP_NUMOF_VCS) return;
  
  if(!_Drivers[vc]) return;
        
  if(link >= PGP_NUMOF_LINKS) return;

  Driver *driver = _Drivers[vc];
  
  Device* device = driver->devices[link];
  
  if(!device) return;

  printf(_line0);
  printf(_line1, device->link,device->vc); 
  printf(_line2, device->allocs);
  printf(_line3, device->deallocs);
  printf(_line4, device->pages);
  printf(_line5, device->posted);
  printf(_line6, device->post_fails);
  printf(_line7, device->max_if_pages);
  printf(_line8, device->fails);
  printf(_line9, device->dmabytes);  
  printf(_line10,device->frames);
  printf(_line11,device->discard);  
  
  }
