/*
** ++
**  Package: PGP
**	
**
**  Abstract: PGP RRI Driver.
**
**  Implementation of PGP RRI driver.
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
#include <string.h>
#include <stdio.h>

#include "elf/linker.h"
#include "sas/Sas.h"
#include "pgp/Pgp.h"
#include "pgp/Rri.h"

#define HEADER_SIZE        3
#define RRI_VIRT_CHANNEL   0

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

#define PLUGIN_PREFS_NAME "SAS_PLUGIN_PREFS"

/*
**
**
*/

typedef struct RRI_Fd {
 void*           payload;      // Don't care
 uint32_t        size;         // Don't care
 SAS_MbxId       dst;          // Mailbox to fork a frame
 struct RRI_Fd*  pending;      // Frame descriptor pending processing
 Pgp_Header      hdr;          // PGP header words
 uint32_t        transaction;  // First word of PGP packet  
 RRI_Instruction instruction;  // Second and third word of PGP packet
 uint32_t        fault;        // Last word of PGP packet
 SAS_Frame       frame;        // Frame corresponding to above descriptor.
 SAS_IbMbx       mbx;          // Mailbox 
} RRI_Fd;

/*
**
**
*/

typedef struct {
 SAS_Session     session;
 SAS_ObMbx       mbx;
 RRI_Processor   processor;
 RRI_Arg         arg;
 RRI_Result      result;
 uint64_t        posted;
 uint64_t        processed;
 uint64_t        failures;
} Device;

/*
** Forward declarations for local functions...
*/

SAS_IbOpcode RRI_Fork(SAS_Frame, SAS_IbMbx, uint32_t);

static void _post(RRI_Device reference, 
                  uint32_t link, 
                  uint32_t operation,
                  uint32_t operand, 
                  uint32_t tid,
                  uint32_t vc);
static uint32_t _wait(SAS_Message, void*, SAS_ObMbx);
static int      _synch(RRI_Result*, RRI_Arg);

static const char PLUGIN_PREFS[] = PLUGIN_PREFS_NAME;  // Name of SAS plugin prefs

static const char *Plugins[PGP_NUMOF_PLUGINS] = {NULL,NULL,NULL};

/*
** ++
**
**
** --
*/

#define RRI_ERROR 1

int const lnk_options = LNK_INSTALL;

unsigned lnk_prelude(void* arg, Ldr_elf* elf)
  {

  SAS_Preferences **prefs;
    
  /* find the list of plugins to bind */
  SAS_Preferences **list = (SAS_Preferences**)Svt_Translate(PLUGIN_PREFS, SVT_SAS_TABLE);
  if(!list) return RRI_ERROR;
  
  /* bind PGP plugins to drivers */
  for(prefs=list;*prefs;prefs++)
    {
    if((!strncmp((*prefs)->name,PGP0_PLUGIN_NAME,sizeof(PGP0_PLUGIN_NAME))) ||
       (!strncmp((*prefs)->name,PGP1_PLUGIN_NAME,sizeof(PGP1_PLUGIN_NAME))) ||
       (!strncmp((*prefs)->name,PGP2_PLUGIN_NAME,sizeof(PGP2_PLUGIN_NAME))))
      {
      uint32_t sock = strtoul((*prefs)->bitfile,0,0);
      if(sock < PGP_NUMOF_PLUGINS)
        Plugins[sock] = (*prefs)->name;
      }
    }
  
  Pgp_Bind(RRI_Fork,RRI_VIRT_CHANNEL,0); 
  
  return 0;
  }

/*
**
**
*/

RRI_Device RRI_Open(uint32_t socket)
  {

  if(socket >= PGP_NUMOF_PLUGINS)
    return (RRI_Device)0;
    
  if(!Plugins[socket])
    return (RRI_Device)0;
    
  Device* device = (Device*)malloc(sizeof(Device));

  if(!device) return (RRI_Device)0;
  
  bzero(device,sizeof(Device));

  SAS_Session session  = SAS_Open();

  if(!session) return (RRI_Device)0;

  SAS_ObMbx mbx = SAS_ObBind(Plugins[socket], _wait, (void*)device, session);

  if(!mbx) return (RRI_Device)0;

  device->session   = session;            
  device->mbx       = mbx;                
  device->processor = _synch;             

  return (RRI_Device)device;
  }

/*
**
**
*/

static int _synch(RRI_Result* result, RRI_Arg arg)
  { 
  return 0;
  } 

/*
**
**
*/

void RRI_Bind(RRI_Device reference, RRI_Processor processor, RRI_Arg arg)
  {

  Device* device = (Device*)reference;

  device->processor = processor;
  device->arg       = arg;

  return;
  }

/*
**
**
*/

uint32_t RRI_Read(RRI_Device device, uint32_t link, uint32_t address, uint32_t* value)
  {

  RRI_Post(device, link, RRI_ENCODE2(RRI_GET, address), 0, 0); 

  RRI_Result* result = RRI_Wait(device);

  *value = result->instr.operand;

  return RRI_FAULT(result->instr.operation);
  }  

/*
**
**
*/

uint32_t RRI_Write(RRI_Device device, uint32_t link, uint32_t address, uint32_t value)
  {

  RRI_Post(device,  link,(RRI_ENCODE2(RRI_PUT, address)), value, 0); 

  RRI_Result* result = RRI_Wait(device);

  return RRI_FAULT(result->instr.operation);
  }  

/*
**
**
*/

uint32_t RRI_Bis(RRI_Device device, uint32_t link, uint32_t address, uint32_t mask)
  {

  RRI_Post(device, link, RRI_ENCODE2(RRI_BIS, address), mask, 0); 

  RRI_Result* result = RRI_Wait(device);

  return RRI_FAULT(result->instr.operation);
  }  
 
/*
**
**
*/

uint32_t RRI_Bic(RRI_Device device, uint32_t link, uint32_t address, uint32_t mask)
  {

  RRI_Post(device, link, RRI_ENCODE2(RRI_BIC, address), mask, 0); 

  RRI_Result* result = RRI_Wait(device);

  return RRI_FAULT(result->instr.operation);
  }  
  
/*
**
**
*/

static const char _post_error[] = "RRI frame alloc failure: operation 0x%x tid 0x%x\n";

void RRI_Post(RRI_Device reference, uint32_t link, uint32_t operation, uint32_t operand, uint32_t tid)
  {
  _post(reference,link,operation,operand,tid,RRI_VIRT_CHANNEL); 
  }

/*
**
**
*/

void RRI_PostVc(RRI_Device reference, 
                uint32_t link, 
                uint32_t operation, 
                uint32_t operand, 
                uint32_t tid,
                uint32_t vc)
  {
  _post(reference,link,operation,operand,tid,vc); 
  }

/*
**
**
*/

SAS_IbOpcode RRI_Fork(SAS_Frame frame, SAS_IbMbx mbx, uint32_t arg)
  {

  RRI_Fd* fd = (RRI_Fd*)SAS_IbFd(frame, mbx);

  fd->dst     = DST(fd->transaction);
  fd->pending = fd;  
  fd->mbx     = mbx;
  fd->frame   = frame;

  if(!(fd->hdr.fuser & PGP_SOF))
    {
    printf("%s invalid PGP header fuser 0x%x ctl 0x%x size %d\n",\
            __func__,
            fd->hdr.fuser,
            fd->hdr.ctl,
            fd->hdr.size);
    return SAS_IB_FREE;
    }
    
  return SAS_IB_RUNDOWN | SAS_IB_KEEP;
  } 

/*
**
**
*/

void _post(RRI_Device reference, 
           uint32_t link,          
           uint32_t operation,      
           uint32_t operand,       
           uint32_t tid,           
           uint32_t vc)            
  {

  Device* device = (Device*)reference;

  SAS_ObMbx mbx   = device->mbx;
  
  SAS_Frame frame = SAS_ObAlloc(mbx); 
  if(!frame)
    {
    device->failures++;
    printf(_post_error,operation,tid);
    return;
    }

  RRI_Fd* fd = (RRI_Fd*)SAS_ObFd(frame, mbx);
  
  bzero(fd,sizeof(RRI_Fd));

  fd->hdr.fuser = PGP_SOF;
  fd->hdr.luser = 0;
  fd->hdr.ctl   = PGP_EOF;
  fd->hdr.dst   = vc;
  fd->hdr.size  = 0;

  fd->transaction           = ENCODE(SAS_ObId(mbx), tid);
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

static uint32_t _wait(SAS_Message message, void* arg, SAS_ObMbx mbx)
  {

  RRI_Fd* fd = (RRI_Fd*)message;

  uint32_t operation = fd->instruction.operation | (FAULT(fd->fault) << RRI_FAULT_OFFSET);
  uint32_t operand   = fd->instruction.operand;
  uint32_t tid       = TID(fd->transaction);

  Device* device = (Device*)arg;

  RRI_Instruction *instr = &device->result.instr;
  
  instr->operation = operation;
  instr->operand   = operand;
  
  device->result.link = SAS_TypeOf(fd->frame);     
  device->result.tid  = tid;                       
  
  SAS_IbPost(SAS_IB_FREE, fd->frame, fd->mbx);

  device->processed++;

  return (*device->processor)(&device->result, device->arg) ? SAS_REENABLE : SAS_DISABLE | SAS_ABORT;
  }

/*
**
**
*/

RRI_Result* RRI_Wait(RRI_Device reference)
  {

  Device* device = (Device*)reference;

  SAS_EnableWait(device->session);

  return &device->result; 
  }

/*
**
**
*/

void RRI_Close(RRI_Device reference)
  {

  Device* device = (Device*)reference;

  SAS_Close(device->session); 

  free((void*)device);

  return; 
  }

/*
**
**
*/

static const char _line0[]   = "RRI device on virtual channel %d\n";
static const char _line1[]   = "  Posted frames    %llu\n";
static const char _line2[]   = "  Processed frames %llu\n";
static const char _line3[]   = "  Post failures    %llu\n";

void RRI_Dump(RRI_Device reference)
  {
  Device* device = (Device*)reference;

  printf(_line0,RRI_VIRT_CHANNEL);
  printf(_line1,device->posted);
  printf(_line2,device->processed);
  printf(_line3,device->failures);
  }
