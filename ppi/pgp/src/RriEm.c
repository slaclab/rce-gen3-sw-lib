/*
** ++
**  Package: PGP
**	
**
**  Abstract: PGP RRI Emulator Driver (RRIEM).
**
**  Implementation of PGP RRI Emulator driver.
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
#include <string.h>
#include <stdio.h>

#include "concurrency/BasicMutex.h"
#include "svt/Svt.h"
#include "elf/linker.h"
#include "sas/Sas.h"
#include "pgp/Pgp.h"
#include "pgp/Rri.h"
#include "pgp/RriEm.h"

#define HEADER_SIZE        3
#define RRI_VIRT_CHANNEL   0

#define RRIEM_FAULT (1 << RRI_INVADDRESS_OFFSET)

#define DST_OFFSET ((uint32_t)2)
#define DST_LENGTH ((uint32_t)6) 
#define DST_MASK   ((1 << DST_LENGTH) - 1)

#define TID_OFFSET ((uint32_t)8)
#define TID_LENGTH ((uint32_t)24) 
#define TID_MASK   ((1 << TID_LENGTH) - 1)

#define FAULT_OFFSET ((uint32_t)16)
#define FAULT_LENGTH ((uint32_t)2) 
#define FAULT_MASK   ((1 << FAULT_LENGTH) - 1)

#define ENCODE(dst, tid) ((dst << DST_OFFSET) | (tid << TID_OFFSET))

#define DST(transaction) ((transaction >> DST_OFFSET)   & DST_MASK)
#define TID(transaction) ((transaction >> TID_OFFSET)   & TID_MASK)
#define FAULT(fault)     ((fault       >> FAULT_OFFSET) & FAULT_MASK)

#define PAGE(addr)   ((addr >> PAGE_OFFSET) & PAGE_MASK)
#define OFFSET(addr) (addr & ADDR_MASK)
#define MAX_PAGES    64
#define PAGE_SIZE    ((1024*1024)) // 1Mbyte page size
#define PAGE_OFFSET  0x12 // bit offset to page ID
#define PAGE_MASK    ((1<<(RRI_ADDRESS_LENGTH-PAGE_OFFSET))-1)
#define ADDR_MASK    ((1<<PAGE_OFFSET)-1)

#define PLUGIN_PREFS_NAME     "SAS_PLUGIN_PREFS"

/*
**
**
*/

typedef struct RRIEM_Fd {
 void*            payload;     // Don't care
 uint32_t         size;        // Don't care
 SAS_MbxId        dst;         // Mailbox to fork a frame
 struct RRIEM_Fd  *pending;    // Frame descriptor pending processing
 Pgp_Header       hdr;         // PGP header words
 uint32_t         transaction; // First word of PGP packet  
 RRI_Instruction  instruction; // Second and third word of PGP packet
 uint32_t         fault;       // Last word of PGP packet
 SAS_Frame        frame;       // Frame corresponding to above descriptor.
 SAS_IbMbx        mbx;         // Mailbox 
} RRIEM_Fd;

/*
**
**
*/

typedef struct {
 SAS_Session     session;
 SAS_ObMbx       mbx;
 RRI_Result      result;
 uint32_t        socket;
 uint64_t        posted;
 uint64_t        processed;
 uint64_t        failures;
 uint32_t        *regmap[PGP_NUMOF_LANES][MAX_PAGES];
 RRIEM_Processor *processors[PGP_NUMOF_LANES][MAX_PAGES];
} Device;
   
/*
** Forward declarations for local functions...
*/

static SAS_IbOpcode _process(SAS_Frame, SAS_IbMbx, uint32_t);

static SAS_IbOpcode _fork(SAS_Frame, SAS_IbMbx, uint32_t);

static SAS_IbOpcode _emulate(SAS_Frame, SAS_IbMbx, uint32_t);

static RRIEM_Device _open(uint32_t socket, RRIEM_Cfg**);

static uint32_t _construct(Device*, uint32_t socket, RRIEM_Cfg**);

static uint32_t _wait(SAS_Message, void*, SAS_ObMbx);

static void _post(RRIEM_Device reference, 
                  uint32_t link, 
                  uint32_t operation,
                  uint32_t operand, 
                  uint32_t transaction);

/*
**
*/

static const char PLUGIN_PREFS[] = PLUGIN_PREFS_NAME;  // Name of SAS plugin prefs

static const char *_Plugins[PGP_NUMOF_PLUGINS] = {NULL,NULL,NULL};

RRIEM_Device _Devs[PGP_NUMOF_PLUGINS];

static BasicMutex _Mutex[PGP_NUMOF_PLUGINS] = {
   BASIC_MUTEX_INITIALIZER,
   BASIC_MUTEX_INITIALIZER,
   BASIC_MUTEX_INITIALIZER
   };

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
  if(!list) return RRIEM_TRANSLATE;
  
  /* bind PGP plugins to drivers */
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
    
  Pgp_Bind(_process,RRI_VIRT_CHANNEL,0); 
  
  return 0;
  }

/*
**
**
*/

static uint32_t _construct(Device *device, uint32_t socket, RRIEM_Cfg **svt)
  {
  
  uint32_t link,lane;
  int i;
  
  link = socket*PGP_NUMOF_LANES;
  
  bzero(&device->regmap[0],PGP_NUMOF_LANES*MAX_PAGES*sizeof(uint32_t*));
  
  for(lane=0;lane<PGP_NUMOF_LANES;lane++)
    {
    uint32_t bytes = 0;
    int j;
    const RRIEM_Cfg *cfg = svt[lane];
    if(cfg == NULL)
      continue;
   
    uint32_t **regmap = device->regmap[lane];
    
    for(j=0; j<cfg->count; j++)
      {    
      uint32_t page = PAGE(cfg->entry[j].reg);
      if(!regmap[page])
        {
        RRIEM_Processor **processors = device->processors[lane];
        regmap[page] = (uint32_t*)rtems_heap_allocate_aligned_with_boundary(PAGE_SIZE, \
                                                                           PAGE_SIZE, 0);
        if(!regmap[page]) return RRIEM_ALLOC;
        bzero(regmap[page],PAGE_SIZE);
        bytes += PAGE_SIZE;

        processors[page] = (RRIEM_Processor*)rtems_heap_allocate_aligned_with_boundary(PAGE_SIZE, \
                                                                           PAGE_SIZE, 0);
        if(!processors[page]) return RRIEM_ALLOC;
        bzero(processors[page],PAGE_SIZE);
        bytes += PAGE_SIZE;
        }
        
      uint32_t *base = regmap[page];
      uint32_t offset = OFFSET(cfg->entry[j].reg);
      base[offset] = cfg->entry[j].val;           
      }
    }  
    
  return 0;
  }
    
/*
**
**
*/

static RRIEM_Device _open(uint32_t socket, RRIEM_Cfg **cfg)
  {

  if(socket >= PGP_NUMOF_PLUGINS)
    return (RRIEM_Device)0;
    
  if(!_Plugins[socket])
    return (RRIEM_Device)0;
    
  Device* device = (Device*)malloc(sizeof(Device));

  if(!device) return (RRIEM_Device)0;
  
  bzero(device,sizeof(Device));
  
  _construct(device,socket,cfg);

  SAS_Session session  = SAS_Open();

  if(!session) return (RRIEM_Device)0;

  SAS_ObMbx mbx = SAS_ObBind(_Plugins[socket], _wait, (void*)device, session);

  if(!mbx) return (RRIEM_Device)0;

  device->session   = session;            
  device->mbx       = mbx;
  device->socket    = socket;

  _Devs[socket] = (RRIEM_Device)device;
  
  return (RRIEM_Device)device;
  }


/*
**
**
*/

static SAS_IbOpcode _process(SAS_Frame frame, SAS_IbMbx mbx, uint32_t arg)
  {

  uint32_t sock      = PGP_DECODE_SOCK(SAS_TypeOf(frame));
  uint32_t lane      = PGP_DECODE_LANE(SAS_TypeOf(frame));
  RRIEM_Fd *fd       = (RRIEM_Fd*)SAS_IbFd(frame, mbx);
  uint32_t operation = fd->instruction.operation;
  uint32_t operand   = fd->instruction.operand;
  uint32_t page      = PAGE(RRI_ADDRESS(operation));
  uint32_t offset    = OFFSET(RRI_ADDRESS(operation));
  Device *device     = _Devs[sock];

  if(!device) return SAS_IB_FREE;

  fd->fault = 0;
  
  uint32_t **regmap = device->regmap[lane];
    
  if(regmap[page])
    {   
    RRIEM_Processor **base = device->processors[lane];
    RRIEM_Processor processor = (RRIEM_Processor)0;

    if(base)
      processor = base[page][offset];
    
    if(processor)
      return _fork(frame,mbx,arg);
    else 
      return _emulate(frame, mbx, arg);    
    }
  else
    {
    operation = RRI_ENCODE3(RRI_OPCODE(operation),RRI_ADDRESS(operation),RRIEM_FAULT);  
    _post(device,SAS_TypeOf(frame),operation,operand,fd->transaction);
    return SAS_IB_FREE;
    }
    
  return SAS_IB_FREE;
  }
  
/*
**
**
*/

static SAS_IbOpcode _fork(SAS_Frame frame, SAS_IbMbx mbx, uint32_t arg)
  {

  RRIEM_Fd *fd  = (RRIEM_Fd*)SAS_IbFd(frame, mbx);
  uint32_t sock = PGP_DECODE_SOCK(SAS_TypeOf(frame));
  Device   *dev = (Device*)_Devs[sock];

  fd->dst     = SAS_ObId(dev->mbx);
  fd->pending = fd;  
  fd->mbx     = mbx;
  fd->frame   = frame;
    
  return SAS_IB_RUNDOWN | SAS_IB_KEEP;
  } 
  
/*
**
**
*/

static SAS_IbOpcode _emulate(SAS_Frame frame, SAS_IbMbx mbx, uint32_t arg)
  {

  RRIEM_Fd *fd       = (RRIEM_Fd*)SAS_IbFd(frame, mbx);
  uint32_t operation = fd->instruction.operation;
  uint32_t operand   = fd->instruction.operand;
  uint32_t sock      = PGP_DECODE_SOCK(SAS_TypeOf(frame));
  uint32_t lane      = PGP_DECODE_LANE(SAS_TypeOf(frame));
        
  if(RRI_OPCODE(operation) == RRI_GET)
    RRIEM_Read(_Devs[sock],lane,RRI_ADDRESS(operation),&operand);
  else if(RRI_OPCODE(operation) == RRI_PUT)
    RRIEM_Write(_Devs[sock],lane,RRI_ADDRESS(operation),operand);
  else if(RRI_OPCODE(operation) == RRI_BIS)
    RRIEM_Bis(_Devs[sock],lane,RRI_ADDRESS(operation),operand);
  else if(RRI_OPCODE(operation) == RRI_BIC)
    RRIEM_Bic(_Devs[sock],lane,RRI_ADDRESS(operation),operand);

  operation = RRI_ENCODE2(RRI_OPCODE(operation),RRI_ADDRESS(operation));

  _post(_Devs[sock],SAS_TypeOf(frame),operation,operand,fd->transaction);
        
  return SAS_IB_FREE;
  } 

/*
**
**
*/

static uint32_t _wait(SAS_Message message, void* arg, SAS_ObMbx mbx)
  {

  RRIEM_Fd* fd = (RRIEM_Fd*)message;

  Device* device = (Device*)arg;

  device->result.link = SAS_TypeOf(fd->frame);     
  device->result.tid  = TID(fd->transaction);                       

  RRI_Instruction *instr = &device->result.instr;
  
  instr->operation = fd->instruction.operation;
  instr->operand   = fd->instruction.operand;

  uint32_t page    = PAGE(RRI_ADDRESS(instr->operation));
  uint32_t offset  = OFFSET(RRI_ADDRESS(instr->operation));
  
  uint32_t lane = PGP_DECODE_LANE(SAS_TypeOf(fd->frame));
  
  RRIEM_Processor **base = device->processors[lane];
  RRIEM_Processor processor = (RRIEM_Processor)0;

  if(base)
    processor = base[page][offset];
  
  if(processor)
    {    
    uint32_t fault = (*processor)(&device->result);
    instr->operation = RRI_ENCODE3(RRI_OPCODE(instr->operation),RRI_ADDRESS(instr->operation),fault);
    _post(device,
          SAS_TypeOf(fd->frame),
          instr->operation,
          instr->operand,
          fd->transaction);
    }
  else
    _emulate(fd->frame,fd->mbx,0);

  SAS_IbPost(SAS_IB_FREE, fd->frame, fd->mbx);

  device->processed++;

  return SAS_DISABLE | SAS_ABORT;
  }

/*
**
**
*/

static const char _post_error[] = "RRIEM frame alloc failure: operation 0x%x transaction 0x%x\n";

void _post(RRIEM_Device reference, 
           uint32_t link,          
           uint32_t operation,      
           uint32_t operand,       
           uint32_t transaction)            
  {

  Device* device = (Device*)reference;

  SAS_ObMbx mbx   = device->mbx;
  
  SAS_Frame frame = SAS_ObAlloc(mbx); 
  if(!frame)
    {
    device->failures++;
    printf(_post_error,operation,transaction);
    return;
    }

  RRIEM_Fd* fd = (RRIEM_Fd*)SAS_ObFd(frame, mbx);

  bzero(fd,sizeof(RRIEM_Fd));

  fd->hdr.fuser = PGP_SOF;
  fd->hdr.luser = 0;
  fd->hdr.ctl   = PGP_EOF;
  fd->hdr.dst   = RRI_VIRT_CHANNEL;
  fd->hdr.size  = 0;

  fd->transaction           = transaction;
  fd->instruction.operation = operation;
  fd->instruction.operand   = operand;
  fd->fault                 = 0;
  
  SAS_ObPost(SAS_OB_HEADER, SAS_ObSet(frame, PGP_DECODE_LANE(link), HEADER_SIZE), mbx);

  device->posted++;

  return; 
  }

/*
**
**
*/

uint32_t RRIEM_Read(RRIEM_Device device, uint32_t link, uint32_t reg, uint32_t* value)
  {
  
  Device *dev = (Device*)device;
  
  uint32_t lane = PGP_DECODE_LANE(link);
  
  uint32_t **regmap = dev->regmap[lane];
  
  uint32_t page = PAGE(reg);

  if(!regmap[page]) return RRIEM_FAULT;
  
  *value = regmap[page][OFFSET(reg)];

  return 0;    
  }  

/*
**
**
*/

uint32_t RRIEM_Write(RRI_Device device, uint32_t link, uint32_t reg, uint32_t value)
  {
  
  Device *dev = (Device*)device;
  
  uint32_t lane = PGP_DECODE_LANE(link);
  
  uint32_t **regmap = dev->regmap[lane];
  
  uint32_t page = PAGE(reg);

  if(!regmap[page]) return RRIEM_FAULT;
  
  regmap[page][OFFSET(reg)] = value;

  return 0;
  }  

/*
**
**
*/

uint32_t RRIEM_Bis(RRI_Device device, uint32_t link, uint32_t reg, uint32_t mask)
  {
  
  Device *dev = (Device*)device;
  
  uint32_t lane = PGP_DECODE_LANE(link);
  
  uint32_t **regmap = dev->regmap[lane];
  
  uint32_t page = PAGE(reg);

  if(!regmap[page]) return RRIEM_FAULT;
      
  regmap[page][OFFSET(reg)] |= mask;

  return 0;
  }  
 
/*
**
**
*/

uint32_t RRIEM_Bic(RRI_Device device, uint32_t link, uint32_t reg, uint32_t mask)
  {
  
  Device *dev = (Device*)device;
  
  uint32_t lane = PGP_DECODE_LANE(link);
  
  uint32_t **regmap = dev->regmap[lane];
  
  uint32_t page = PAGE(reg);

  if(!regmap[page]) return RRIEM_FAULT;
  
  regmap[page][OFFSET(reg)] &= ~mask;

  return 0;
  }  
  
/*
**
**
*/

void RRIEM_Bind(RRIEM_Device reference, uint32_t link, RRIEM_Processor processor, uint32_t reg)
  {
  
  Device *device         = (Device*)reference;
  uint32_t lane          = PGP_DECODE_LANE(link);
  uint32_t page          = PAGE(RRI_ADDRESS(reg));
  uint32_t offset        = OFFSET(RRI_ADDRESS(reg));
  RRIEM_Processor **base = device->processors[lane];
  
  if(base)
    base[page][offset] = processor;
  }

/*
**
**
*/

RRIEM_Device RRIEM_Open(uint32_t socket, RRIEM_Cfg **cfg)
  {
  
  if(socket >= PGP_NUMOF_PLUGINS)
    return (RRIEM_Device)0;

  Device *dev = _open(socket,cfg);
  if(!dev)
    return (RRIEM_Device)0;
      
  if(!BasicMutex_trylock(&_Mutex[socket]))
    return (RRIEM_Device)0;
        
  dev->posted    = 0;
  dev->processed = 0;
  dev->failures  = 0;
      
  return (RRIEM_Device)dev;
  }

/*
**
**
*/

RRI_Result* RRIEM_Wait(RRIEM_Device reference)
  {

  Device* device = (Device*)reference;
  
  SAS_EnableWait(device->session);

  return &device->result; 
  }

/*
**
**
*/

void RRIEM_Close(RRIEM_Device reference)
  {

  Device* device = (Device*)reference;

  BasicMutex_unlock(&_Mutex[device->socket]);
      
  return; 
  }

/*
**
**
*/

static const char _line0[]   = "RRIEM device on plugin %d virtual channel %d\n";
static const char _line1[]   = "  Posted frames    %llu\n";
static const char _line2[]   = "  Processed frames %llu\n";
static const char _line3[]   = "  Post failures    %llu\n";

void RRIEM_Dump(uint32_t socket)
  {
  
  if(socket >= PGP_NUMOF_PLUGINS)
    return;

  if(!_Plugins[socket]) return;

  Device* device = _Devs[socket];

  printf(_line0,socket,RRI_VIRT_CHANNEL);
  printf(_line1,device->posted);
  printf(_line2,device->processed);
  printf(_line3,device->failures);
  }
