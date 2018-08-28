/*
** ++
**  Package: SAS
**	
**
**  Abstract: SAS engine transactions.
**
**  Implementation of SAS facility. See the corresponding header (".h") file for a
**  more complete description of the functions implemented below.      
**
**  Author:
**      S. Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	000 - July 18, 2014
**
**  Revision History:
**	None.
**
** --
*/

#include <stdint.h>

#include "sas/Sas.h"

#include "Map.h"
#include "Transaction.h"

//#define DEBUG

#ifdef DEBUG
#include <stdio.h>
#define debug_printf printf
#else
#define debug_printf(...)
#endif

/*
** ++
**
** This function de-allocates an outbound frame descriptor.
**
** --
*/

void SAS_ObFree(SAS_Frame frame, SAS_ObMbx m)
  {
  uint32_t instr = frame;
  
  /* build the instruction */
  SAS_ENCODE(OB_OPCODE,SAS_OB_FREE,instr);
     
  /* post the instruction */
  m->m.socket->obWorkList = instr;
  }

/*
** ++
**
** This function allocates an outbound frame descriptor.
**
** --
*/

SAS_Frame SAS_ObAlloc(SAS_ObMbx m)
  {
  uint32_t instr;
  uint32_t fval = 0;

  /* allocate a descriptor offset */
  fval = m->m.socket->obFreeList;
  
  debug_printf("ObFd free-list read 0x%x 0x%x\n",
               (int)fval,(int)&m->m.socket->obFreeList);
  
  /* check for empty free-list condition */
  if(SAS_DECODE(INPUT_EMPTY,fval)) return SAS_ALLOC_ERROR;

  debug_printf("ObAlloc fd 0x%x 0x%x\n",
               (int)m->m.ocm+SAS_DECODE(FRAME_DESC,fval),(int)SAS_DECODE(FRAME_DESC,fval));
  
  OB_INSTR_INIT(instr);
  SAS_ENCODE(FRAME_DESC,SAS_DECODE(FRAME_DESC,fval),instr);
  
  /* return the frame handle */    
  return (SAS_Frame)instr;
  }

/*
** ++
**
** This function will encode and schedule an outbound instruction for execution.
**
** --
*/

void SAS_ObPost(SAS_ObOpcode opcode, SAS_Frame frame, SAS_ObMbx m)
  {
  uint32_t instr = frame;
    
  /* build the instruction */
  SAS_ENCODE(OB_OPCODE,opcode,instr);
  
  debug_printf("ObSchedule 0x%x reg 0x%x\n",(int)instr,(int)&m->m.socket->obWorkList);

  /* wait for write buffer to drain */
  __asm__ volatile("dmb":::"memory");
    
  /* post the instruction */
  m->m.socket->obWorkList = instr;
  }

/*
** ++
**
** This function tests for error in an outbound message.
**
** --
*/

uint32_t SAS_ObError(const SAS_Message input)
  {
  return SAS_DECODE(MBX_INPUT_ERROR,input);
  }

/*
** ++
**
** This function sets a frame type and size.
**
** --
*/

SAS_Frame SAS_ObSet(SAS_Frame frame, SAS_Type type, SAS_Size len)
  {
  uint32_t instr = frame;
  SAS_ENCODE(FRAME_TYPE,type,instr);
  SAS_ENCODE(FRAME_LEN,len,instr);
  return instr;
  }

/*
** ++
**
** This function returns the frame descriptor implicit in a frame handle.
**
** --
*/

SAS_Fd* SAS_ObFd(const SAS_Frame frame, SAS_ObMbx m)
  {
  return (SAS_Fd*)(m->m.ocm+SAS_DECODE(FRAME_DESC,frame));
  }

/*
** ++
**
** This function tests for a frame with an inbound payload.
**
** --
*/

uint32_t SAS_IbPayload(const SAS_Frame input)
  {
  return SAS_DECODE(IB_PAYLOAD,input);
  }

/*
** ++
**
** This function tests for a frame with an inbound error.
**
** --
*/

uint32_t SAS_IbError(const SAS_Frame input)
  {
  return SAS_DECODE(IB_ERROR,input);
  }

/*
** ++
**
** This function returns the frame descriptor implicit in a frame handle.
**
** --
*/

SAS_Fd* SAS_IbFd(const SAS_Frame frame, SAS_IbMbx m)
  {
  return (SAS_Fd*)(m->m.ocm+SAS_DECODE(FRAME_DESC,frame));
  }

/*
** ++
**
** This function will encode and schedule an inbound instruction for execution.
**
** --
*/

void SAS_IbPost(SAS_IbOpcode opcode,
                SAS_Frame frame,
                SAS_IbMbx m)
  {
  uint32_t instr = frame;
  
  /* build the instruction */
  IB_INSTR_INIT(instr);
  SAS_ENCODE(IB_OPCODE,opcode,instr);

  debug_printf("IbSchedule 0x%x reg 0x%x\n",(int)instr,(int)&m->m.socket->ibWorkList);
  
  /* wait for write buffer to drain */
  __asm__ volatile("dmb":::"memory");
  
  /* post the instruction */
  m->m.socket->ibWorkList = instr;
  }

/*
** ++
**
** This function tests for error in a rundown message.
**
** --
*/

uint32_t SAS_Error(const SAS_Message input)
  {
  return SAS_DECODE(MBX_INPUT_ERROR,input);
  }

/*
** ++
**
** This function returns the type associated with a frame
**
** --
*/

uint32_t SAS_TypeOf(const SAS_Frame input)
  {
  return SAS_DECODE(FRAME_TYPE,input);
  }

/*
** ++
**
** This function returns the size associated with a frame
**
** --
*/

uint32_t SAS_SizeOf(const SAS_Frame input)
  {
  return SAS_DECODE(FRAME_LEN,input);
  }
