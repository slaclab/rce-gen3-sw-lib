/*
** ++
**  Package: XAUI
**	
**
**  Abstract: XAUI Transmit Driver
**
**  Implementation of XAUI transmit driver. See the corresponding header (".h") file for a
**  more complete description of the functions implemented below.      
**
**  Author:
**      S. Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	000 - August 19, 2014
**
**  Revision History:
**	None.
**
** --
*/

#include <stdlib.h>
#include <stdio.h>   // printf...

#include "conversion/impl/BSWP.ih"
#include "memory/mem.h"
#include "memory/resources.h"
#include "sas/Sas.h"
#include "xaui/Xaui.h"

#include "Xaui_p.h"

#define TIMING
#ifdef TIMING
#include "../src/Pmu.h"
#endif  

/*
** ++
**
**
** --
*/

#define TYPE_NET          0
#define MAX_DEVICES       8
#define DUMP_RUNDOWN      0x4
#define DUMP_FRAME        0x2
#define DEBUG_TRANSMIT    0x1

#ifdef TIMING
static volatile uint32_t *gtc = (uint32_t*)GTC_REGISTER;
#endif

static const char XauiPlugin[] = XAUI_NAMEOF_PLUGIN;
 
/*
**
**
*/

typedef struct {
  SAS_ObMbx         mbx;
  uint32_t          mid;
  void             *rset;
  void             *resource;
  Xaui_TxProcessor  processor;
  void             *ctx;   
} Device;

/*
** ++
**
**
** --
*/

typedef struct {
  SAS_Session    session;
  SAS_ObMbx      mbx;
  uint64_t       dma_st;
  uint64_t       dma_et;
  uint64_t       dma_elp;
  uint32_t       tid;
  uint32_t       debugLevel;
  uint32_t       alloc;
  uint32_t       dealloc;
  uint32_t       posted;
  uint32_t       max_inflight;
  uint32_t       dma_bytes;
  uint32_t       header_frames; 
  uint32_t       freed_headers;
  uint32_t       payload_frames;
  uint32_t       freed_payloads;
  uint32_t       post_errors;
  uint32_t       hdr_rundown_errors;
  uint32_t       dma_rundown_errors;
  uint32_t       hdr_size;  
  uint32_t       devices;
  
} Context;

static Context* _context = (Context*)0;

/*
** ++
**
**
** --
*/

void _postSync(void)
  {
  SAS_Frame frame = SAS_ObAlloc(_context->mbx);
  if(!frame) return;

  /* post an empty frame to the first bound device mailbox */
  SAS_Fd*  fd = SAS_ObFd(frame,_context->mbx);
  fd->payload = (void*)1;                // payload value 1 indicates a sync frame
  fd->size    = 0;                       // size of 0 indicates a sync frame
  fd->mid     = SAS_ObId(_context->mbx); // send the frame to the first bound device mailbox   
  fd->message = (void*)((uint32_t)fd+sizeof(SAS_Fd));

  SAS_ObPost(SAS_OB_PAYLOAD_RUNDOWN, SAS_ObSet(frame, TYPE_NET, 1), _context->mbx);
  }
  
/*
** ++
**
**
** --
*/

void _post(const Xaui_Header* frame, uint32_t size, SAS_Message message, Device* device)
 {

 SAS_Fd   *fd;
 SAS_Frame sas_frame;
 SAS_ObMbx mbx;
 uint32_t  hdr_size;
 
 if(size > _context->hdr_size)
   {
   sas_frame    = SAS_ObAlloc(device->mbx);
   if(!sas_frame)
     {
     ++_context->post_errors;
     if(_context->debugLevel & DEBUG_TRANSMIT) printf("Xaui_Post: SAS_ObAlloc failure for size %d\n",size);
     return;
     }
   fd           = SAS_ObFd(sas_frame,device->mbx);   
   fd->payload  = (void*)frame;
   fd->size     = size;                   
   hdr_size     = 0;

   /* flush the entire payload from the cache */
   rtems_cache_flush_multiple_data_lines((void*)frame,
                                          size);
   
   _context->dma_bytes += size;
   
   if(_context->debugLevel & DEBUG_TRANSMIT) printf("%s: post payload frame 0x%x fd 0x%x buf 0x%x size %d\n",__func__,sas_frame,fd,frame,size);
   }
 else
   {
   fd          = (SAS_Fd*)((uint32_t)frame-sizeof(SAS_Fd));
   sas_frame   = (SAS_Frame)((uint32_t)fd & 0x3FFF0);
   fd->payload = NULL;
   fd->size    = 0;
   hdr_size    = (size>>3) + ((size%8) ? 1 : 0);

   if(_context->debugLevel & DEBUG_TRANSMIT)
     {
     printf("%s: post header frame 0x%x fd 0x%x buf 0x%x size %d hdr_words %d mod %d\n",__func__,sas_frame,fd,frame,size,hdr_size,(size%8));   
     if(_context->debugLevel & DUMP_FRAME)
       {
       uint32_t *pay = (uint32_t*)&fd->header[0];  
       printf("Xaui %s: hdr[0]  0x%08x\n",__func__,(unsigned)pay[0]);
       printf("Xaui %s: hdr[1]  0x%08x\n",__func__,(unsigned)pay[1]);
       printf("Xaui %s: hdr[2]  0x%08x\n",__func__,(unsigned)pay[2]);
       printf("Xaui %s: hdr[3]  0x%08x\n",__func__,(unsigned)pay[3]);
       printf("Xaui %s: hdr[4]  0x%08x\n",__func__,(unsigned)pay[4]);
       printf("Xaui %s: hdr[5]  0x%08x\n",__func__,(unsigned)pay[5]);
       printf("Xaui %s: hdr[6]  0x%08x\n",__func__,(unsigned)pay[6]);
       printf("Xaui %s: hdr[7]  0x%08x\n",__func__,(unsigned)pay[7]);
       }
     }
   }

 fd->mid     = device->mid;          
 fd->message = (void*)((uint32_t)fd+sizeof(SAS_Fd));

 if((_context->debugLevel & DUMP_FRAME) && fd->payload) 
  {
   uint32_t *pay = (uint32_t*)fd->payload;  
   printf("Xaui %s: payload 0x%x size %d mid %d\n",__func__,(int)fd->payload,(int)fd->size,(int)fd->mid);
   printf("Xaui %s: pay[0]  0x%08x\n",__func__,(unsigned)pay[0]);
   printf("Xaui %s: pay[1]  0x%08x\n",__func__,(unsigned)pay[1]);
   printf("Xaui %s: pay[2]  0x%08x\n",__func__,(unsigned)pay[2]);
   printf("Xaui %s: pay[3]  0x%08x\n",__func__,(unsigned)pay[3]);
   printf("Xaui %s: pay[4]  0x%08x\n",__func__,(unsigned)pay[4]);
   printf("Xaui %s: pay[5]  0x%08x\n",__func__,(unsigned)pay[5]);
   printf("Xaui %s: pay[6]  0x%08x\n",__func__,(unsigned)pay[6]);
   printf("Xaui %s: pay[7]  0x%08x\n",__func__,(unsigned)pay[7]);
  }

#ifdef TIMING 
 READ_GTC(_context->dma_st);
#endif      
 
 SAS_ObPost(SAS_OB_PAYLOAD_RUNDOWN, SAS_ObSet(sas_frame, TYPE_NET, hdr_size), device->mbx);
   
 ++_context->posted;
 
 return;
 }

/*
** ++
**
**
** --
*/

static uint32_t _rundown(SAS_Message msg, void* ctx, SAS_ObMbx mbx)
 {  
 Device *device = (Device*)ctx;
 
 int ret = SAS_REENABLE;
 
#ifdef TIMING 
  READ_GTC(_context->dma_et);
  if(_context->dma_st>_context->dma_et)
    _context->dma_elp += (0xffffffff-_context->dma_st) + _context->dma_et;
  else
    _context->dma_elp += _context->dma_et-_context->dma_st;
#endif  
 
 if(SAS_ObError(msg))
   {
   ++_context->dma_rundown_errors;
   if(_context->debugLevel & DUMP_RUNDOWN) printf("%s: xaui outbound dma completion error\n",__func__);
   }
   
 if(_context->debugLevel & DUMP_RUNDOWN)
   { 
   printf("Got outbound dma rundown %d message\n",(int)SAS_ObId(mbx));
   printf("  ob rundown error: 0x%x\n",SAS_ObError(msg));
   printf("  ob rundown input: 0x%x\n",(int)msg);
   }
       
 if(_context->debugLevel & DUMP_RUNDOWN) printf("xaui tx dma elp %llu\n",_context->dma_elp);
 _context->dma_elp = 0;

 SAS_Fd *fd = (SAS_Fd*)((uint32_t)(msg & RUNDOWN_MSG_MASK)-sizeof(SAS_Fd));
 
 if(!fd->size && fd->payload)
   {
   /* this is a sync frame so just return */
   if(_context->debugLevel & DUMP_RUNDOWN) printf("%s sync fd 0x%x size %d pay 0x%x\n",__func__,fd,fd->size,fd->payload);
   return ret;
   }
  
  /* execute callback */
 if(device->processor)
   device->processor(device->ctx);

 if(fd->size && fd->payload)
   {
   mem_rsFree(device->rset,fd->payload);
   ++_context->freed_payloads;
   }
 else  
  ++_context->freed_headers;     

 ++_context->dealloc;
  
 return ret; 
 } 

/*
** ++
**
**
** --
*/

static void _wait(uint32_t arg)
 {
 SAS_Session session = (SAS_Session)arg;
 SAS_Wait(session);
 }
  
/*
** ++
**
**
** --
*/

#define SIZEOF_CONTEXT (sizeof(Context))
 
static void* _allocate()
 {
 void* context = malloc(SIZEOF_CONTEXT);
        
 if(context) bzero(context, SIZEOF_CONTEXT);
  
 return context;
 }

/*
** ++
**
**
** --
*/
 
unsigned _setup(void)
  {
  _context->debugLevel = 0;
  
  return 0;               
  }
    
/*
** ++
**
* This routine executes the constructor 
* for the XAUI transmitter.
*
** --
*/
  
uint32_t Xaui_TxInit(const Task_Attributes *attrs)
  {  
  
  uint32_t error = XAUI_INSFMEM;
  
  _context = (Context*)_allocate();

  if(!_context) return error;
  
  error = XAUI_NOSESSION;

  SAS_Session session = SAS_Open();
 
  if(!session) return error;

  _context->session = session;

  const SAS_Attributes *sattr = SAS_GetAttributes(XauiPlugin, session);
 
  _context->hdr_size = sattr->moh << 3;

  /* initialize context */
  _setup();
 
  rtems_status_code sc;
  
  sc = rtems_task_create(rtems_build_name('X','A','T','X'),
                         attrs->priority-1,
                         attrs->stack_size,
                         attrs->attributes,
                         attrs->modes,
                         &_context->tid);
  if (sc != RTEMS_SUCCESSFUL)
    rtems_panic("%s Can't create task: %s", __func__,rtems_status_text(sc));
 
  return XAUI_SUCCESS;

  }

/*
** ++
**
**
** --
*/


void Xaui_TxDebug(uint32_t level)
  {
  _context->debugLevel = level;
  }


/*
** ++
**
* This routine allocates a transmit pool frame buffer.
*
*
** --
*/

Xaui_Header *Xaui_Alloc(Xaui_TxDevice reference, uint32_t size)
  {
  Device *device = (Device*)reference;
  void *buffer = NULL;
  
  if(size <= _context->hdr_size)
    {  
    /* frame will fit in a header */
    SAS_Frame frame = SAS_ObAlloc(device->mbx);
    if(!frame)
      {
      if(_context->debugLevel & DEBUG_TRANSMIT) printf("%s: failure to alloc header frame size %d max %d\n",__func__,size,_context->hdr_size);      
      return NULL;
      }
    
    SAS_Fd*   fd    = SAS_ObFd(frame,device->mbx);
    buffer          = (void*)((uint32_t)fd+sizeof(SAS_Fd));
    
    ++_context->header_frames;
    if(_context->debugLevel & DEBUG_TRANSMIT) printf("%s: alloc header frame 0x%x fd 0x%x buf 0x%x size %d\n",__func__,frame,fd,buffer,size);
    }
  else if(size <= (RESOURCE_SIZE-sizeof(Xaui_Header)))
    {
    buffer = mem_rsAlloc(device->rset);
    if(!buffer) return NULL;
    
    ++_context->payload_frames;
    if(_context->debugLevel & DEBUG_TRANSMIT) printf("%s: alloc payload buf 0x%x size %d\n",__func__,buffer,size);
    }
  else
    return buffer;    
      
  uint32_t deallocs = _context->dealloc;

  if((++_context->alloc - deallocs) > _context->max_inflight)
    _context->max_inflight = _context->alloc - deallocs;
  
  return (Xaui_Header*)buffer;
  }

/*
** ++
**
* This routine sets the XAUI dma handlers.
*
** --
*/

Xaui_TxDevice Xaui_TxOpen(uint32_t numof_buffers)
  {
  if(_context->devices >= MAX_DEVICES) return (Xaui_TxDevice)0;
    
  Device *device = malloc(sizeof(Device));
  
  if(!device) return (Xaui_TxDevice)0;

  bzero(device,sizeof(Device));
  
  device->mbx = SAS_ObBind(XauiPlugin, _rundown, device, _context->session);

  if(!device->mbx) return (Xaui_TxDevice)0;
  
  device->mid = SAS_ObId(device->mbx);

  //device->resource = mem_Region_alloc(MEM_REGION_UNCACHED, RESOURCE_SIZE*numof_buffers);
  device->resource = (void*)rtems_heap_allocate_aligned_with_boundary(RESOURCE_SIZE*numof_buffers,0x20,0);

  if(!device->resource) return (Xaui_TxDevice)0;

  device->rset = mem_rsOpen(numof_buffers, device->resource, RESOURCE_STRIDE);
  if(!device->rset) return (Xaui_TxDevice)0;
    
  SAS_ObEnable(device->mbx);

  if(!_context->mbx)
    {
    _context->mbx = device->mbx;
    
    rtems_status_code sc = rtems_task_start(_context->tid, (rtems_task_entry)_wait, (int)_context->session);
    if (sc != RTEMS_SUCCESSFUL)
      rtems_panic("%s Can't start task: %s", __func__,rtems_status_text(sc));
    }
  else
    _postSync();    
    
  return (Xaui_TxDevice)device;
  }

/*
** ++
**
* This routine sets the XAUI dma handlers.
*
** --
*/

void Xaui_TxBind(Xaui_TxDevice reference, Xaui_TxProcessor processor, void *ctx)
  {
  Device *device = (Device*)reference;
  
  device->processor = processor;
  device->ctx       = ctx;
  }

/*
** ++
**
* This routine executes XAUI transmit.
*
** --
*/

void Xaui_Post(Xaui_TxDevice reference, Xaui_Header *frame, uint32_t size)
  {
  
  Device *device = (Device*)reference;
  
  if(size > RESOURCE_SIZE)
    {
    ++_context->post_errors;
    if(_context->debugLevel & DEBUG_TRANSMIT) printf("%s: invalid frame size %d\n",__func__,size);
    return;
    }

  SAS_Message msg = (SAS_Message)frame;

  if(_context->debugLevel & DEBUG_TRANSMIT) printf("%s: %d frame bytes\n",__func__,size);

  _post(frame, size, msg, device);
  }

/*
** ++
**
**
** --
*/

static const char line0[]  = " \nXaui Tx plugin statistics:\n";
static const char line1[]  = "  allocated       : %u\n";
static const char line2[]  = "  deallocated     : %u\n";
static const char line3[]  = "  posted          : %u\n";
static const char line4[]  = "  header frames   : %u\n";
static const char line5[]  = "  freed headers   : %u\n";
static const char line6[]  = "  payload frames  : %u\n";
static const char line7[]  = "  freed payloads  : %u\n";
static const char line8[]  = "  max inflight    : %u\n";
static const char line9[]  = "  dma bytes       : %u\n";
static const char line10[] = "  post errors     : %u\n";
static const char line11[] = "  header errors   : %u\n";
static const char line12[] = "  payload errors  : %u\n";

void Xaui_TxStats(int reset) 
 { 
 if(reset)
   {
   _context->alloc               = 0;
   _context->dealloc             = 0;
   _context->header_frames       = 0;
   _context->freed_headers       = 0;
   _context->payload_frames      = 0;
   _context->freed_payloads      = 0; 
   _context->posted              = 0;
   _context->max_inflight        = 0;
   _context->dma_bytes           = 0;
   _context->post_errors         = 0;
   _context->hdr_rundown_errors  = 0;
   _context->dma_rundown_errors  = 0;
   
   return;
   }
   
 printf(line0);
 printf(line1, _context->alloc);
 printf(line2, _context->dealloc);
 printf(line3, _context->posted);
 printf(line4, _context->header_frames);
 printf(line5, _context->freed_headers);
 printf(line6, _context->payload_frames);
 printf(line7, _context->freed_payloads);  
 printf(line8, _context->max_inflight);
 printf(line9, _context->dma_bytes);
 printf(line10,_context->post_errors);
 printf(line11,_context->hdr_rundown_errors);
 printf(line12,_context->dma_rundown_errors);
 
 return;
 }

