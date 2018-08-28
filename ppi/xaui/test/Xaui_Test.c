/*
** ++
**  Package: XAUI
**	
**
**  Abstract: XAUI Test Driver
**
**  Implementation of XAUI test driver. See the corresponding header (".h") file for a
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

#include <stdio.h>   // printf...
#include <stdlib.h>

#define __need_getopt_newlib
#include <getopt.h>

#include <rtems/shell.h>

#include "conversion/impl/BSWP.ih"
#include "sas/Sas.h"
#include "map/Lookup.h"
#include "bsi/Bsi_Cfg.h"
#include "bsi/Bsi.h"

#include "xaui/Xaui.h"

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

#define TYPE_NET 0

#define DUMP_RUNDOWN  0x1
#define DUMP_PAYLOAD  0x2

#define MAX_PAYLOAD_SIZE  XAUI_MAX_PAYLOAD

#define NUMOF_BUFFERS 64

#define ETHERNET_TYPE_XTST 0x1111
#define ETHERNET_TYPE_ARP  0x806
#define ETHERNET_TYPE_IPV4 0x800


/* @brief

Ethernet frame as delivered by the plug-in

*/

typedef struct {
  Xaui_Header hdr;
  uint32_t    size;      // sizeof frame data
  uint32_t    ctx;       // context argument
  uint8_t     data[];    // frame data
} Xaui_Frame;

#ifdef TIMING
static volatile uint32_t *gtc = (uint32_t*)GTC_REGISTER;
#endif

static const char XauiPlugin[] = XAUI_NAMEOF_PLUGIN;

static uint32_t _frames;

/*
** ++
**
**
** --
*/

const char USAGE_XAUI[] =
  "\n  Xaui Plugin Test Driver\n\
  Usage: xaui_test [OPTIONS] [args]\n\
    -g   <level>   Enable/disable debug output\n\
    -r   <3-16>    Set frame header length (8-byte units)\n\
    -p   <3-189>   Set frame payload length (8-byte units)\n\
    -f   <1-4096>  Set frame transmit count\n\
    -t   <mac>     Transmit xaui test frames\n\
    -y   <bytes>   Set raw frame payload bytes\n\
    -x   <mac>     Transmit raw xaui test frames\n\
    -o   <prot>    Override protocol handler\n\
    -n             Restore last protocol handler\n\
    -w             Wait for xaui test frames\n";

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
**
** --
*/

typedef struct {
  Xaui_RxDevice rx_device;
  Xaui_TxDevice tx_device;
  uint64_t      dst;
  uint32_t      debugLevel;
  uint32_t      frameCount;
  uint32_t      payloadSize;
  uint32_t      payloadBytes;  
  uint32_t      headerSize;
  uint32_t      maxHeader;
  uint64_t      dma_st;
  uint64_t      dma_et;
  uint64_t      dma_elp;
  uint64_t      dma_et1;
  uint64_t      dma_elp1;
  SAS_Session   session;
  SAS_ObMbx     mbx;
  Xaui_Protocol prot;
  uint32_t      prot_id;
  
} Context;

static Context* _context = (Context*)0;

/*
** ++
**
**
** --
*/

static uint32_t _dispatch(SAS_Frame frame, void* context, SAS_IbMbx mbx)
 { 
 printf("%s prot 0x%x override: frame 0x%x context 0x%x mbx 0x%x\n",__func__,_context->prot_id,frame,context,mbx);
 if(_context->prot.processor)
   return _context->prot.processor(frame, _context->prot.ctx, mbx);
 else
   return SAS_IbPayload(frame) ? SAS_IB_FLUSH : SAS_IB_FREE;   
 } 

/*
** ++
**
**
** --
*/

static void _override(uint32_t protocol)
  {
  Xaui_Protocol *save = Xaui_Lookup(protocol);
  _context->prot.processor = save->processor;
  _context->prot.ctx = save->ctx;
  
  Xaui_Bind(protocol,_dispatch,(void*)0xdeadbeef);  
  _context->prot_id = protocol;
  
  printf("%s 0x%x\n",__func__,protocol);
  }

/*
** ++
**
**
** --
*/

static void _restore(void)
  {
  Xaui_Bind(_context->prot_id,_context->prot.processor,_context->prot.ctx);
  printf("%s 0x%x\n",__func__,_context->prot_id);  
  }

/*
** ++
**
**
** --
*/

static uint32_t _setDebug(uint32_t level)
  {
  _context->debugLevel = level;
  }

/*
** ++
**
**
** --
*/

static uint32_t _setHeader(uint32_t size)
  {
  _context->headerSize = (size<<3)>_context->maxHeader ? _context->maxHeader : (size<<3);
  }

/*
** ++
**
**
** --
*/

static uint32_t _setPayload(uint32_t size)
  {
  _context->payloadSize = (size<<3)>MAX_PAYLOAD_SIZE ? MAX_PAYLOAD_SIZE : (size<<3);
  }

/*
** ++
**
**
** --
*/

static uint32_t _setFrames(uint32_t count)
  {
  if(count <= 500)
    _context->frameCount = count;
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
 
unsigned _wait(void)
  {
  Xaui_Wait(_context->rx_device);
  }

/*
** ++
**
**
** --
*/
 
unsigned _process(void* ctx, Xaui_Frame* frame, uint32_t size)
  {
  int i;
       
  uint32_t dsize = frame->size;
  uint32_t *pay = (uint32_t*)&frame->data[0];

  if(_context->debugLevel)
    printf("%s frame 0x%x frame size %d payload size %d ctx 0x%x\n",__func__,frame,size,dsize,ctx);  

  for(i=0;i<((dsize-sizeof(Xaui_Frame))/(sizeof(uint32_t)));i++)
    {
    if(_context->debugLevel & DUMP_PAYLOAD)
      printf("  payload[%d] 0x%x\n",i,pay[i]);
    }
  
  Xaui_Free(_context->rx_device,frame,size);
  }
  
/*
** ++
**
**
** --
*/
 
void _transmit(void)
 { 
 
 Xaui_Frame *frame;
 
 if(_context->headerSize)
   {
   frame = (Xaui_Frame*)Xaui_Alloc(_context->tx_device, _context->headerSize);
   if(!frame)
     {
     printf("%s: failure allocating Xaui frame\n",__func__);
     return;
     }
   frame->size = _context->headerSize;
   }
 else
   {
   frame = (Xaui_Frame*)Xaui_Alloc(_context->tx_device, _context->payloadSize);
   if(!frame)
     {
     printf("%s: failure allocating Xaui frame\n",__func__);
     return;
     }
   frame->size = _context->payloadSize;
   }
   
 Xaui_Header *packet = &frame->hdr;
 packet->dst  = BSWP__swap64(_context->dst);     
 packet->type = BSWP__swap16(ETHERNET_TYPE_XTST);

 frame->ctx   = _frames;

 uint16_t *pay = (uint16_t*)&frame->data;
 int i;
 
 for(i=0;i<((frame->size-sizeof(Xaui_Frame))/sizeof(uint16_t));i++)
   pay[i] = i;

 Xaui_Post(_context->tx_device,(Xaui_Header*)frame,frame->size);
 }

/*
** ++
**
**
** --
*/
 
unsigned _transmit_raw(void)
 { 
 uint32_t ret = SAS_REENABLE;
 int i;
 Xaui_Frame *frame;
 
 SAS_Frame list[500];
 
 for(i=0;i<_context->frameCount;i++)
   {
   frame = (Xaui_Frame*)rtems_heap_allocate_aligned_with_boundary(_context->payloadBytes,32,0);
   if(!frame)
     {
     printf("%s: failure allocating buffer size %d bytes\n",__func__,_context->payloadBytes);
     return SAS_ABORT;
     }
   Xaui_Header *packet = &frame->hdr;
   packet->dst  = BSWP__swap64(_context->dst);     
   packet->type = BSWP__swap16(ETHERNET_TYPE_XTST);

   SAS_Fd *fd;
   SAS_Frame sframe;

   sframe = SAS_ObAlloc(_context->mbx);
   fd    = SAS_ObFd(sframe, _context->mbx);

   fd->payload = (void*)frame;
   fd->size    = _context->payloadBytes; 
   fd->mid     = SAS_ObId(_context->mbx);
   fd->message = (void*)(frame);
   list[i] = SAS_ObSet(sframe, TYPE_NET, 0);
   
   rtems_cache_flush_multiple_data_lines((void*)frame,_context->payloadBytes);
   }

#ifdef TIMING 
 READ_GTC(_context->dma_st);
#endif      
  
 for(i=0;i<_context->frameCount;i++)
   {
   SAS_ObPost(SAS_OB_PAYLOAD_RUNDOWN, list[i], _context->mbx);
   }

 #ifdef TIMING 
 READ_GTC(_context->dma_et1);
 if(_context->dma_st>_context->dma_et1)
   _context->dma_elp1 += (0xffffffff-_context->dma_st) + _context->dma_et1;
 else
   _context->dma_elp1 += _context->dma_et1-_context->dma_st;
 #endif  

 printf("xaui post_raw %d frames %llu cycles avg %llu\n",_context->frameCount,_context->dma_elp1,_context->dma_elp1/_context->frameCount);
 _context->dma_elp1 = 0;
    
 return ret;
 }


/*
** ++
**
**
** --
*/

static uint32_t _rundown_raw(SAS_Message msg, void* ctx, SAS_ObMbx mbx)
 { 
 double mbyteps;
 
 
 if(_frames)
   --_frames;

 if(!_frames)
  {
  #ifdef TIMING 
  READ_GTC(_context->dma_et);
  if(_context->dma_st>_context->dma_et)
    _context->dma_elp += (0xffffffff-_context->dma_st) + _context->dma_et;
  else
    _context->dma_elp += _context->dma_et-_context->dma_st;
  #endif  

  if(SAS_ObError(msg))
    printf("  ob rundown error: 0x%x\n",SAS_ObError(msg));
   
  mbyteps = 1000000000.0/((_context->dma_elp*2.5)/(_context->frameCount*_context->payloadBytes))/(1024*1024);
  printf("xaui rundown_raw dma %d frames %d bytes in %llu cycles %f MByte/sec\n",\
  _context->frameCount,_context->frameCount*_context->payloadBytes,_context->dma_elp,mbyteps);
  _context->dma_elp = 0;
  }
 
 free((void*)msg);
 
 return SAS_ABORT; 
 } 

/*
** ++
**
**
** --
*/

static void _task(uint32_t arg)
 {
 while(1)
   SAS_Wait(_context->session);
 }

/*
** ++
**
**
** --
*/
 
unsigned _setup(void)
  {
  _context->payloadSize = 32;
  _context->headerSize  = 0;
  _context->frameCount  = 1;
  _context->debugLevel  = 3;
  _context->payloadBytes = 1500; 
               
   /* Setup callbacks */
  _context->rx_device = Xaui_RxOpen(NUMOF_BUFFERS);
  if(!_context->rx_device) return 1;
  Xaui_RxBind(ETHERNET_TYPE_XTST, _context->rx_device, (Xaui_RxProcessor)_process, NULL);  

  _context->tx_device = Xaui_TxOpen(NUMOF_BUFFERS);
  if(!_context->tx_device) return 1;
  Xaui_TxBind(_context->tx_device,NULL,NULL);

  _context->session = SAS_Open();
  
  if(!_context->session) 
    {
    printf("Error opening session\n");
    return 1;
    }

  const SAS_Attributes* attrs = (const SAS_Attributes*)SAS_GetAttributes(XAUI_NAMEOF_PLUGIN, _context->session);  
  _context->maxHeader = attrs->moh<<3;

  _context->mbx = SAS_ObBind(XauiPlugin, _rundown_raw, NULL, _context->session);

  if(!_context->mbx)
    {
    printf("Error binding obMbx 0x%x\n",(int)_context->mbx);
    return -1;
    }

  SAS_ObEnable(_context->mbx);

  rtems_status_code sc;
  rtems_id tid;

  sc = rtems_task_create(rtems_build_name('X','T','S','T'),
                         150,
                         RTEMS_MINIMUM_STACK_SIZE * 10,
                         RTEMS_DEFAULT_ATTRIBUTES,
                         RTEMS_DEFAULT_MODES,
                         &tid);

  sc = rtems_task_start(tid, (rtems_task_entry)_task, 0);
  
  return 0;
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
  int theOpt;

  if(argc == 1) return _usage();

  struct getopt_data getopt_reent;
  memset(&getopt_reent, 0, sizeof(getopt_data));
  
  while ( (theOpt = getopt_r(argc,argv,
                             "hr:p:f:g:t:wx:y:o:n", 
                             &getopt_reent)) != EOF )
    {  
    switch (theOpt)
      {
      case 'g': _setDebug(strtoul(getopt_reent.optarg, 0, 0));    break;
      case 'f': _setFrames(strtoul(getopt_reent.optarg, 0, 0));   break;
      case 'p': _setPayload(strtoul(getopt_reent.optarg, 0, 0));  break;
      case 'r': _setHeader(strtoul(getopt_reent.optarg, 0, 0));   break;
      case 't': 
        _context->dst = strtoull(getopt_reent.optarg, 0, 0);
        _frames       = _context->frameCount;
        while(_frames--) _transmit();
        break;
      case 'x': 
        _context->dst = strtoull(getopt_reent.optarg, 0, 0);
        _frames       = _context->frameCount;
        _transmit_raw();
        break;
      case 'y': _context->payloadBytes = strtoul(getopt_reent.optarg, 0, 0); break;
      case 'w': _wait(); break;
      case 'o': _override(strtoul(getopt_reent.optarg, 0, 0)); break;
      case 'n': _restore(); break;
      case 'h':
      default:  return _usage();
      }
    }
  }

/*
** ++
**
** This function registers XAUI commands with the rtems shell.
**
** --
*/

void _addCommands(void)
  {
  rtems_shell_cmd_t * retCmd;
  // Call the long form for adding this command.  This
  // allocates the structure space inside RTEMS

  retCmd = rtems_shell_add_cmd("xaui_test",
                               "ppi",                             
                               USAGE_XAUI,                             
                               (rtems_shell_command_t)_main);

  rtems_shell_add_cmd_struct( retCmd );
  }
  
/*
** ++
**
* This routine executes the constructor 
* for the XAUI test shareable library.
*
** --
*/

int lnk_prelude(void *prefs, void *elfHdr) 
  {  
  _context = (Context*)_allocate();

  if(!_context) return -1;

  /* initialize context */
  _setup();
  
  /* register shell commands */
  _addCommands();
  
  return 0;
  }
