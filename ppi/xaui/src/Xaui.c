/*
** ++
**  Package: XAUI
**	
**
**  Abstract: XAUI PPI Receiver.
**
**  Implementation of XAUI PPI receiver.
**
**  Author:
**      S. Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	000 - August 10, 2014
**
**  Revision History:
**	None.
**
** --
*/

#include <strings.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>   // printf...

#include <getopt.h>

#include <rtems/shell.h>

#include "startup/init.h"
#include "conversion/impl/BSWP.ih"
#include "task/Task.h"
#include "memory/mem.h"
#include "memory/resources.h"
#include "sas/Sas.h"
#include "map/Lookup.h"
#include "bsi/Bsi_Cfg.h"
#include "bsi/Bsi.h"
#include "elf/linker.h"
#include "svt/Svt.h"
#include "debug/print.h"

#include "xaui/Xaui.h"
#include "Xaui_p.h"

/*
**
*/

#define XAUI_BROADCAST  0xFFFFFFFFFFFF0000LL
#define XAUI_MULTICAST  0x0000000000010000LL
#define STP_MAC         0x000000C280010000LL
#define LINK_WAIT       100000   // link training wait in usec - 100ms

#define XAUI_PROT_COUNT 65536    // max supported ethernet protocols

/*
**
**
*/

typedef struct {
  SAS_Session       session;
  uint32_t          mbx_id;
  Xaui_Fd           pending;
  Xaui_RxProcessor  processor;
  void             *ctx;
  void             *rset;
  void             *resource;
} Device;

/*
** ++
**
**
** --
*/

typedef struct {
  SAS_IbMbx      ib_mbx;
  uint32_t       hdr_size;
  uint32_t       debug;
  uint64_t       mac64;
  uint64_t       mac48;
  Xaui_Protocol *prot_table;
} Driver;

/*
** ++
**
**
** --
*/

typedef struct {
  uint64_t   ib_frames;
  uint64_t   ib_errors;
  uint64_t   bcast_frames;
  uint64_t   mcast_frames;
  uint64_t   local_frames;
  uint64_t   stp_frames;
  uint64_t   ufo_frames;
  uint64_t   freed_headers;
  uint64_t   freed_payloads;
  uint64_t   rundown_errors;
  uint64_t   hdr_req;
  uint64_t   hdr_rundowns;
  uint64_t   hdr_bytes;
  uint64_t   dma_req;
  uint64_t   dma_rundowns;
  uint64_t   dma_bytes;
  uint32_t   inflight;
  uint32_t   retired;
  uint32_t   max_inflight;
  uint32_t   last_error;
} Context;

/*
**
*/

Xaui_Registers* _Xaui_Registers = (Xaui_Registers*)0;

/*
**
*/

static Driver*   _driver   =  (Driver*)0;
static Context*  _context  = (Context*)0;

/*
** Forward declarations for local functions...
*/

static uint32_t  _L3Dispatcher(SAS_Frame frame, void* context, SAS_IbMbx mbx);
static uint32_t  _L3Rundown(SAS_Message msg, Device*, SAS_Mbx mbx);
static uint32_t  _dispatch(SAS_Frame, void* context, SAS_IbMbx);
static uint32_t* _mac(void);
static int       _reformat(Xaui_Header* packet);
static void      _setup(uint32_t *mac);
static void      _link(uint32_t poll);
static void      _phyd(uint32_t poll);
static void      _dump(SAS_Frame, volatile SAS_Fd*);
static void      _addCommands(void);

/*
** ++
**
**
** --
*/

Xaui_RxDevice Xaui_RxOpen(uint32_t numof_buffers)
  {

  SAS_Session session = SAS_Open();

  if(!session) return (Xaui_RxDevice)0;

  Device *device = malloc(sizeof(Device));
  if(!device) return (Xaui_RxDevice)0;

  bzero(device,sizeof(Device));
    
  SAS_Mbx mbx = SAS_Bind((SAS_Handler)_L3Rundown, (void*)device, session);

  if(!mbx) return (Xaui_RxDevice)0;

  device->session   = session;            
  device->mbx_id    = SAS_Id(mbx);

  device->resource = mem_Region_alloc(MEM_REGION_UNCACHED, RESOURCE_SIZE*numof_buffers);
  if(!device->resource) return (Xaui_RxDevice)0;

  bzero(device->resource, RESOURCE_SIZE*numof_buffers);

  device->rset = mem_rsOpen(numof_buffers, device->resource, RESOURCE_STRIDE);
  if(!device->rset) return (Xaui_RxDevice)0;

  return (Xaui_RxDevice)device;
  }

/*
** ++
**
**
** --
*/

void Xaui_RxBind(uint32_t protocol, Xaui_RxDevice reference, Xaui_RxProcessor processor, void *ctx)
  {
  if(protocol >= XAUI_PROT_COUNT) return;
  
  Device *device    = (Device*)reference;  
  device->processor = processor;
  device->ctx       = ctx;
  
  Xaui_Protocol *prot = &_driver->prot_table[protocol];  
  prot->processor     = _L3Dispatcher;
  prot->ctx           = device;
  }

/*
** ++
**
**
** --
*/

Xaui_Fd* Xaui_Wait(Xaui_RxDevice reference)
  {
  
  Device *device = (Device*)reference;
  
  SAS_EnableWait(device->session);
  
  return &device->pending;
  }

/*
** ++
**
**
** --
*/


void Xaui_RxDebug(uint32_t level)
  {
  _driver->debug = level;
  }

/*
** ++
**
**
** --
*/

void Xaui_Free(Xaui_RxDevice reference, void* buf, uint32_t size)
  {
  Device *device = (Device*)reference;

  if(_driver->debug) printf("%s: size %d buf 0x%x\n",__func__,size,buf);
  
  if(size <= _driver->hdr_size)
    {
    buf = (void*)(((uint32_t)buf & 0x3FFF0) - sizeof(SAS_Fd));
    SAS_IbPost(SAS_IB_FREE, (SAS_Frame)buf, _driver->ib_mbx);          
    ++_context->freed_headers;
    if(_driver->debug) printf("%s: header size %d buf 0x%x hdrs %u free %u 0x%x\n",__func__,size,buf,_context->hdr_req,_context->freed_headers,rtems_task_self());

    }
  else
    {
    mem_rsFree(device->rset,buf);
    ++_context->freed_payloads;
    if(_driver->debug) printf("%s: payload size %d buf 0x%x hdrs %u free %u 0x%x\n",__func__,size,buf,_context->dma_req,_context->freed_payloads,rtems_task_self());
    }
  }

/*
** ++
**
**
** --
*/

uint64_t Xaui_Mac()
  {
  return _driver->mac64;
  }

/*
** ++
**
**
** --
*/

void Xaui_Bind(uint32_t protocol, SAS_IbHandler handler, void *ctx)
  {
  
  if(protocol >= XAUI_PROT_COUNT) return;
    
  Xaui_Protocol *prot = &_driver->prot_table[protocol];  
  prot->processor     = handler;
  prot->ctx           = ctx;
  }

/*
** ++
**
**
** --
*/

Xaui_Protocol *Xaui_Lookup(uint32_t protocol)
  {
  
  if(protocol >= XAUI_PROT_COUNT) return NULL;
    
  return &_driver->prot_table[protocol];
  }
  
/*
** ++
**
**
** --
*/

void _dump(SAS_Frame frame, volatile SAS_Fd *fd)
  {

  printf("\nXaui %s: frame 0x%x type 0x%x\n",__func__,frame,SAS_TypeOf(frame));
  printf("Xaui %s: hdr[0]  0x%08x\n",__func__,(int)fd->header[0]);
  printf("Xaui %s: hdr[1]  0x%08x\n",__func__,(int)fd->header[1]);
  printf("Xaui %s: hdr[2]  0x%08x\n",__func__,(int)fd->header[2]);
  printf("Xaui %s: hdr[3]  0x%08x\n",__func__,(int)fd->header[3]);
  printf("Xaui %s: hdr[4]  0x%08x\n",__func__,(int)fd->header[4]);
  printf("Xaui %s: hdr[5]  0x%08x\n",__func__,(int)fd->header[5]);
  printf("Xaui %s: hdr[6]  0x%08x\n",__func__,(int)fd->header[6]);
  printf("Xaui %s: hdr[7]  0x%08x\n",__func__,(int)fd->header[7]);
  printf("Xaui %s: hdr[8]  0x%08x\n",__func__,(int)fd->header[8]);
  printf("Xaui %s: hdr[9]  0x%08x\n",__func__,(int)fd->header[9]);
  printf("Xaui %s: hdr[10] 0x%08x\n",__func__,(int)fd->header[10]);
  printf("Xaui %s: hdr[11] 0x%08x\n",__func__,(int)fd->header[11]);
  printf("Xaui %s: hdr[12] 0x%08x\n",__func__,(int)fd->header[12]);
  printf("Xaui %s: hdr[13] 0x%08x\n",__func__,(int)fd->header[13]);
  printf("Xaui %s: hdr[14] 0x%08x\n",__func__,(int)fd->header[14]);
  printf("Xaui %s: hdr[15] 0x%08x\n",__func__,(int)fd->header[15]);
  }

/*
** ++
**
**
** --
*/
static void _L3Dump(SAS_Message msg, volatile SAS_Fd* fd, SAS_Mbx mbx)
  {
  printf("Xaui L3 rundown mbx %d message\n",(int)SAS_Id(mbx));
  printf("  error: 0x%x\n",SAS_Error(msg));
  printf("  input: 0x%x\n",(int)msg);
  printf("  payload 0x%x size %d\n",fd->payload,fd->size);

  uint32_t *pay = (uint32_t*)((uint32_t)fd->payload);
  printf("Xaui %s: pay[0]  0x%08x\n",__func__,(int)pay[0]);
  printf("Xaui %s: pay[1]  0x%08x\n",__func__,(int)pay[1]);
  printf("Xaui %s: pay[2]  0x%08x\n",__func__,(int)pay[2]);
  printf("Xaui %s: pay[3]  0x%08x\n",__func__,(int)pay[3]);
  printf("Xaui %s: pay[4]  0x%08x\n",__func__,(int)pay[4]);
  printf("Xaui %s: pay[5]  0x%08x\n",__func__,(int)pay[5]);
  printf("Xaui %s: pay[6]  0x%08x\n",__func__,(int)pay[6]);
  printf("Xaui %s: pay[7]  0x%08x\n",__func__,(int)pay[7]);

  printf("Xaui %s: pay[128]  0x%08x\n",__func__,(int)pay[128]);
  printf("Xaui %s: pay[129]  0x%08x\n",__func__,(int)pay[129]);
  printf("Xaui %s: pay[130]  0x%08x\n",__func__,(int)pay[130]);
  printf("Xaui %s: pay[131]  0x%08x\n",__func__,(int)pay[131]);  
  }
  
/*
** ++
**
**
** --
*/

static uint32_t _L3Rundown(SAS_Message msg, Device *device, SAS_Mbx mbx)
 { 
 volatile SAS_Fd *fd = (SAS_Fd*)((uint32_t)msg & RUNDOWN_MSG_MASK);

 ++_context->retired;
 
 if(!fd)
   {
   printf("%s: null xaui fd\n",__func__);   
   _context->rundown_errors++;
   return SAS_REENABLE;
   }

 uint32_t       size;
 Xaui_Header   *f     = (Xaui_Header*)&fd->header[0];   
 uint16_t       ftype = BSWP__swap16(f->type);          
 uint32_t      *pay   = (uint32_t*)&f->data[0];         
 Xaui_Protocol *prot  = &_driver->prot_table[ftype];    
 
 if(fd->size)
   {
   if(_driver->debug)
     _L3Dump(msg,fd,mbx);
   size = sizeof(Xaui_Header)+XAUI_MAX_PAYLOAD;
   device->pending.frame = (void*)((uint32_t)fd->payload-_driver->hdr_size);
   ++_context->dma_rundowns;
   }
 else
   {
   size = _driver->hdr_size;
   device->pending.frame = (void*)fd->header;
   ++_context->hdr_rundowns;
   }       

 if(SAS_Error(msg)) goto error;
 
 device->pending.size = size;
 device->pending.ctx  = prot->ctx;
 
 if(device->processor)
   device->processor(device->ctx, device->pending.frame, size);
 
 return SAS_DISABLE | SAS_ABORT;
 
 error:
 
 if(_driver->debug)   
   printf("%s: error in xaui fd 0x%x\n",__func__,msg);
 
 _context->rundown_errors++;
 _context->last_error = (uint32_t)fd;
 
 Xaui_Free((Xaui_RxDevice)device,device->pending.frame,size);
 
 return SAS_REENABLE;
 } 

/*
** ++
**
**
** --
*/

static uint32_t _dispatch(SAS_Frame frame, void* context, SAS_IbMbx mbx)
  {
  uint32_t opcode = SAS_IbPayload(frame) ? SAS_IB_FLUSH : SAS_IB_FREE;

  _context->ib_frames++;

  if(SAS_TypeOf(frame) != XAUI_FRAME_TYPE) goto rundown;

  volatile SAS_Fd* fd = SAS_IbFd(frame, mbx);

  if(SAS_IbError(frame))
    {
    _context->ib_errors++;
    _context->last_error = (uint32_t)fd;
    goto rundown;
    }

  /* check for mac match, multicast, or broadcast */
  if(!_reformat((Xaui_Header*)fd->header)) goto rundown; 

  if(_driver->debug & 0x2) _dump(frame,fd);

  /* find registered protocol */
  Xaui_Header *f  = (Xaui_Header*)&fd->header[0];
  uint16_t ftype = BSWP__swap16(f->type);     

  if(ftype >= XAUI_PROT_COUNT) goto rundown;

  Xaui_Protocol *prot = &_driver->prot_table[ftype];
  if(!prot->processor) goto rundown;    
 
  opcode = prot->processor(frame, prot->ctx, mbx);      
  
  rundown:

  SAS_IbPost(opcode, frame, mbx);
  
  return SAS_REENABLE;
  }

/*
** ++
**
**
** --
*/
static uint32_t _L3Header(volatile SAS_Fd* fd, uint32_t mid)
  {
  fd->size    = 0;
  fd->payload = NULL;
  fd->message = (SAS_Fd*)fd;
  fd->mid     = mid;

  ++_context->hdr_req;

  _context->hdr_bytes += _driver->hdr_size;

  return SAS_IB_RUNDOWN | SAS_IB_KEEP;  
  }

/*
** ++
**
**
** --
*/
static uint32_t _L3Payload(volatile SAS_Fd* fd, uint32_t mid, Device* device)
 {
 void *buffer = mem_rsAlloc(device->rset);
 if(!buffer)
   {
   printf("%s failure to allocate from frame pool\n",__func__);
   return SAS_IB_FLUSH;
   }

 /* offset by 2 bytes for proper IP header alignment */
 int i;
 uint8_t *src = (uint8_t*)&fd->header[0];
 uint8_t *dst = (uint8_t*)((uint32_t)buffer);
 for(i=0;i<_driver->hdr_size;i++)
   *dst++ = *src++;
 fd->size    = XAUI_MAX_PAYLOAD;
 fd->payload = (void*)((uint32_t)buffer+_driver->hdr_size);
 fd->message = (SAS_Fd*)fd;
 fd->mid     = mid;

 if(_driver->debug) printf("%s post dma to 0x%x %d bytes compl mbx %d\n",__func__,(int)fd->payload,fd->size,fd->mid);

 ++_context->dma_req;
 
 _context->dma_bytes += fd->size;

 return SAS_IB_PAYLOAD_RUNDOWN;
 }


/*
** ++
**
**
** --
*/

static uint32_t _L3Dispatcher(SAS_Frame frame, void* context, SAS_IbMbx mbx)
  { 
 
  Device *device = (Device*)context;
  volatile SAS_Fd* fd = SAS_IbFd(frame, mbx);

  uint32_t retired = _context->retired;

  if((++_context->inflight - retired) > _context->max_inflight)
    _context->max_inflight = _context->inflight - retired;
     
  if(SAS_IbPayload(frame))
    return _L3Payload(fd,device->mbx_id,device);
  else 
    return _L3Header(fd,device->mbx_id);
  } 

/*
** ++
**
**
** --
*/

static int _reformat(Xaui_Header* packet)
  {
  uint64_t dst = packet->dst;
  uint64_t src;
  
  if(_driver->debug)
    {
    src = packet->src[2];
    src <<= 16;
    src |= packet->src[1];
    src <<= 16;
    src |= packet->src[0];
    }
    
  if(dst == _driver->mac64)
    {
    _context->local_frames++;
    if(_driver->debug) printf("Xaui %s: local packet src 0x%llx dst 0x%llx this 0x%llx\n",__func__,src,dst,_driver->mac64);
    }
  else if(dst == XAUI_BROADCAST)
    {
    _context->bcast_frames++;
    if(_driver->debug) printf("Xaui %s: bcast packet src 0x%llx dst 0x%llx this 0x%llx\n",__func__,src,dst,_driver->mac64);
    } 
  else if(dst == STP_MAC)  
    {
    _context->stp_frames++;
    if(_driver->debug) printf("Xaui %s: stp packet src 0x%llx dst 0x%llx this 0x%llx\n",__func__,src,dst,_driver->mac64);
    }
  else if(dst & XAUI_MULTICAST)  
    {
    _context->mcast_frames++;
    if(_driver->debug) printf("Xaui %s: mcast packet src 0x%llx dst 0x%llx this 0x%llx\n",__func__,src,dst,_driver->mac64);
    }
  else  
    {
    _context->ufo_frames++;
    if(_driver->debug) printf("Xaui %s: ufo packet src 0x%llx dst 0x%llx this 0x%llx\n",__func__,src,dst,_driver->mac64);   
    }   
    
   if ((dst == _driver->mac64) || (dst == XAUI_BROADCAST) || (dst & XAUI_MULTICAST)) return 1;
   else return 0;
   } 

/*
** ++
**
**
** --
*/

static const char XauiPlugin[] = XAUI_NAMEOF_PLUGIN;
 
#define ARGC    2  // # of arguments passed to task... 
#define SESSION 0  // First argument is session handle and...
#define MBX     1  // second argument is XAUI mailbox handle

static const char XAUI_TASK_ATTRS[] = "XAUI_TASK_ATTRS";

int const lnk_options = LNK_INSTALL;
 
unsigned lnk_prelude(Task_Attributes* attributes, Ldr_elf* elf)
 {

 _driver = malloc(sizeof(Driver));        
 
 if(_driver)
   bzero(_driver, sizeof(Driver));
 else    
   return XAUI_INSFMEM;
 
 _context = malloc(sizeof(Context));
        
 if(_context)
   bzero(_context,sizeof(Context));
 else    
   return XAUI_INSFMEM;
  
 unsigned error = XAUI_NOSESSION;

 SAS_Session session = SAS_Open();
 
 if(!session) goto rundown;
 
 SAS_Off(XauiPlugin,session); 
  
 SAS_On(XauiPlugin,session); 

 _Xaui_Registers = (Xaui_Registers*)SAS_Registers(XauiPlugin, session);

 const SAS_Attributes *sattr = SAS_GetAttributes(XauiPlugin, session);
 
 _driver->hdr_size = sattr->mib << 3;
  
 error = XAUI_NOMBX;
 
 _driver->ib_mbx = SAS_IbBind(XauiPlugin, _dispatch, _driver, session); 

 if(!_driver->ib_mbx)
   {
   printf("Error binding Xaui ib mbx\n");
   goto rundown;
   }
   
 /* setup protocol table */
 _driver->prot_table = (Xaui_Protocol*)rtems_heap_allocate_aligned_with_boundary(sizeof(Xaui_Protocol)*XAUI_PROT_COUNT,32,0);
 if(!_driver->prot_table)
   {
   printf("Error allocating Xaui protocol table\n");
   goto rundown;
   }
   
 bzero(_driver->prot_table, sizeof(Xaui_Protocol)*XAUI_PROT_COUNT);
   
 _addCommands(); 

 if(!attributes)
   attributes = (Task_Attributes*)Svt_Translate(XAUI_TASK_ATTRS, SVT_SYS_TABLE);   
 if(!attributes) return XAUI_NOATTRS;
   
 Xaui_TxInit(attributes);

 /* configure the phy */
 _setup(_mac());
 
  /* launch the link daemon */
 _phyd(rtems_clock_get_ticks_per_second());

 const char* argv[ARGC];

 argv[SESSION] = (char*)session;
 argv[MBX]     = (char*)_driver->ib_mbx;
    
 rtems_id id;
 error = Task_Run(elf, attributes, ARGC, argv, &id);
 
 if(!error) return XAUI_SUCCESS;
 
 rundown:

 if(session) SAS_Close(session);
 
 return error;
 }
 
/*
** ++
**
**
** --
*/

static uint32_t* _mac(void)
 {
 
 Bsi bsi = LookupBsi();
 
 _driver->mac48 = BsiRead64(bsi, BSI_MAC_ADDR_OFFSET);
 _driver->mac64 = _driver->mac48 << 16;

 TeardownBsi(bsi);
 
 return (uint32_t*)&_driver->mac48;
 }

/*
** ++
**
**
** --
*/

#define PHYD_PRIORITY 90

static void _phyd(uint32_t poll)
  {
  rtems_status_code sc;
  rtems_id tid;

  sc = rtems_task_create(rtems_build_name('P','H','Y','D'),
                         PHYD_PRIORITY,
                         RTEMS_MINIMUM_STACK_SIZE * 10,
                         RTEMS_DEFAULT_ATTRIBUTES,
                         RTEMS_DEFAULT_MODES,
                         &tid);
  if (sc != RTEMS_SUCCESSFUL)
    rtems_panic("Can't create task: %s", rtems_status_text(sc));

  sc = rtems_task_start(tid, (rtems_task_entry)_link, poll);
  if (sc != RTEMS_SUCCESSFUL)
    rtems_panic("Can't start task: %s", rtems_status_text(sc));
  }    
  
/*
** ++
**
**
** --
*/
 
#define TEST_PATTERN 0
#define GAP          0xFF

static void _setup(uint32_t *mac)
 {
 
 Xaui_Registers* this = _Xaui_Registers;
  
 /* clear the core config */
 this->config.core     = 0;  
 
 /* take phy out of reset */  
 this->config.phy      = Xaui_DISABLE(XAUI_PHY_RESET, 0);  

 /* configure interface */ 
 this->config.pause    = Xaui_ENCODE(XAUI_PAUSE_GAP, XAUI_PAUSE_GAP_LENGTH, GAP, 0); 
 this->config.mac[0]   = mac[0];
 this->config.mac[1]   = mac[1];

 this->config.ctl = Xaui_ENCODE(XAUI_CTL_RX_SHIFT, XAUI_CTL_RX_SHIFT_LENGTH, XAUI_HDR_SHIFT, this->config.ctl); 
 this->config.ctl = Xaui_ENCODE(XAUI_CTL_TX_SHIFT, XAUI_CTL_TX_SHIFT_LENGTH, XAUI_HDR_SHIFT, this->config.ctl); 
 this->config.ctl = Xaui_ENABLE(XAUI_CTL_MAC_FILTER, this->config.ctl);
 this->config.ctl = Xaui_DISABLE(XAUI_CTL_IP_CSUM, this->config.ctl);
 this->config.ctl = Xaui_DISABLE(XAUI_CTL_TCP_CSUM, this->config.ctl);
 this->config.ctl = Xaui_DISABLE(XAUI_CTL_UDP_CSUM, this->config.ctl); 
 
 this->config.hdr_size = (_driver->hdr_size>>3) + ((_driver->hdr_size%8) ? 1 : 0) - 1;
  
 /* establish link, this call blocks */
 BOOT_STATE(BOOT_LINKTRAIN);
 _link(0);
 dbg_printv("Xaui: link up (10000/FULL)\n");
 }

/*
** ++
**
**
** --
*/
  
static void _link(uint32_t poll)
 {
 
 Xaui_Registers* this = _Xaui_Registers;
 
 uint32_t down = 0;
  
 while(1)
   {
   if(!Xaui_IS(XAUI_STATUS_RX_LINK, this->config.status))
     {
     if(poll && !down)
      {
      dbg_printv("\nXaui: link down\n");
      down = 1;
      BOOT_STATE(BOOT_LINKTRAIN);
      }
          
     /* assert reset */   
     uint32_t core = 0;
     core = Xaui_ENABLE(XAUI_CORE_RESET_LINK,   core); 
     core = Xaui_ENABLE(XAUI_CORE_RESET_FAULTS, core);
     this->config.core = core;

     /* wait for link training */
     usleep(LINK_WAIT);

     /* deassert reset */
     core = 0;
     core = Xaui_DISABLE(XAUI_CORE_RESET_LINK,   core); 
     core = Xaui_DISABLE(XAUI_CORE_RESET_FAULTS, core);
     this->config.core = core;
     }
   else if(!poll) break;
   else if(down) 
     {
     dbg_printv("Xaui: link up (10000/FULL)\n");
     down = 0;     
     BOOT_STATE(BOOT_SUCCESS);
     }         
     
   rtems_task_wake_after(poll);
   }
 
 return;
 } 
 
/*
** ++
**
**
** --
*/

void Task_Start(int argc, const char** argv)
 { 
 SAS_Session session = (SAS_Session)argv[SESSION];
 
 SAS_IbMbx   mbx     = (SAS_IbMbx)argv[MBX];
    
 SAS_IbEnable(mbx);
    
 SAS_Wait(session);
 
 return;
 }

/*
** ++
**
**
** --
*/

void Task_Rundown() 
 { 
 return;
 }

/*
** ++
**
**
** --
*/

void Xaui_Stats(void)
  {
  Xaui_RxStats(0);
  Xaui_TxStats(0);
  }

/*
** ++
**
**
** --
*/

static const char line0[]  = " Xaui Rx plugin statistics\n";
static const char line1[]  = "  rx frames       : %llu\n";
static const char line2[]  = "  local frames    : %llu\n";
static const char line3[]  = "  bcast frames    : %llu\n";
static const char line4[]  = "  mcast frames    : %llu\n";
static const char line5[]  = "  stp frames      : %llu\n";
static const char line6[]  = "  ufo frames      : %llu\n";
static const char line7[]  = "  header frames   : %llu\n";
static const char line8[]  = "  freed headers   : %llu\n";
static const char line9[]  = "  payload frames  : %llu\n";
static const char line10[] = "  freed payloads  : %llu\n";
static const char line11[] = "  error frames    : %llu\n";
static const char line12[] = "  rundown errors  : %llu\n";
static const char line13[] = "  last error fd   : %08x\n";
static const char line14[] = "  max inflight    : %u\n";
static const char line15[] = "  hdr req         : %llu\n";
static const char line16[] = "  hdr rundown     : %llu\n";
static const char line17[] = "  hdr bytes       : %llu\n";
static const char line18[] = "  dma req         : %llu\n";
static const char line19[] = "  dma rundown     : %llu\n";
static const char line20[] = "  dma bytes       : %llu\n";

void Xaui_RxStats(int reset) 
 { 
 if(reset)
   {
   _context->ib_frames       = 0;
   _context->local_frames    = 0;
   _context->bcast_frames    = 0;
   _context->mcast_frames    = 0;
   _context->stp_frames      = 0;
   _context->ufo_frames      = 0;
   _context->freed_headers   = 0;
   _context->freed_payloads  = 0;
   _context->rundown_errors  = 0;
   _context->max_inflight    = 0;
   _context->ib_errors       = 0;
   _context->hdr_req         = 0;
   _context->hdr_rundowns    = 0;
   _context->hdr_bytes       = 0;
   _context->dma_req         = 0;
   _context->dma_rundowns    = 0;
   _context->dma_bytes       = 0;
   _context->last_error      = 0;
   return;
   }
   
 printf(line0);
 printf(line1,_context->ib_frames);
 printf(line2,_context->local_frames);
 printf(line3,_context->bcast_frames);
 printf(line4,_context->mcast_frames);
 printf(line5,_context->stp_frames);
 printf(line6,_context->ufo_frames);
 printf(line7,_context->hdr_req);
 printf(line8,_context->freed_headers);
 printf(line9,_context->dma_req);
 printf(line10,_context->freed_payloads);  
 printf(line11,_context->ib_errors); 
 printf(line12,_context->rundown_errors); 
 printf(line13,_context->last_error);
 printf(line14,_context->max_inflight); 
 printf(line15,_context->hdr_req);
 printf(line16,_context->hdr_rundowns);
 printf(line17,_context->hdr_bytes);
 printf(line18,_context->dma_req);
 printf(line19,_context->dma_rundowns);
 printf(line20,_context->dma_bytes);
 return;
 }

/*
** ++
**
**
** --
*/

const char USAGE_XAUI[] =
  "\n  Xaui Plugin Driver\n\
  Usage: xaui [OPTIONS] [args]\n\
    -d, --dump               Dump xaui registers\n\
    -s, --stats              Print frame statistics\n\
    -r, --reset              Reset frame statistics\n\
    -g, --debug <level>      Enable/disable debug output\n";

/*
** ++
**
**
** --
*/

void _usage(void)
  {
  optind = 0;
  printf("%s",USAGE_XAUI);
  }

/*
** ++
**
** This function executes the XAUI shell command
**
** --
*/
  
void _main(int argc, char** argv)
  {
  int optcnt = 0;
  if(argc == 1) return _usage();
  
  while (1)
    {
    int opt;
    static struct option opts[] = 
      {
        { (char*)"dump",      no_argument,       0, 'd' },     
        { (char*)"stats",     no_argument,       0, 's' },     
        { (char*)"reset",     no_argument,       0, 'r' },     
        { (char*)"debug",     required_argument, 0, 'g' },               
        { (char*)"help",      no_argument,       0, 'h' },
        { 0, 0, 0, 0 }
      };

    if ((opt = getopt_long(argc, argv, "dsrhg:", opts, NULL)) == -1)
      break;
    
    optcnt++;
    switch (opt)
      {
      case 'd': Xaui_Dump();                         break;      
      case 's': Xaui_RxStats(0); Xaui_TxStats(0);    break;            
      case 'r': Xaui_RxStats(1); Xaui_TxStats(1);    break;      
      case 'g': Xaui_RxDebug(strtoul(optarg, 0, 0));
                Xaui_TxDebug(strtoul(optarg, 0, 0)); break;
      case 'h':
      default:  return _usage();
      }
    }
    
  if(!optcnt) _usage();    
  optind = 0;         // Reset the option index to allow command to be repeated    
  }

/*
** ++
**
** This function registers XAUI commands with the rtems shell.
**
** --
*/

static void _addCommands(void)
  {
  rtems_shell_cmd_t * retCmd;
  // Call the long form for adding this command.  This
  // allocates the structure space inside RTEMS

  retCmd = rtems_shell_add_cmd("xaui",
                               "ppi",                             
                               USAGE_XAUI,                             
                               (rtems_shell_command_t)_main);

  rtems_shell_add_cmd_struct( retCmd );
  }

