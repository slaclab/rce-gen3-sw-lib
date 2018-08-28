/*
** ++
**  Package: SAS
**	
**
**  Abstract: SAS socket specific routines.
**
**  Implementation of SAS facility. See the corresponding header (".h") file for a
**  more complete description of the functions implemented below.      
**
**  Author:
**      S. Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	000 - June 03, 2014
**
**  Revision History:
**	None.
**
** --
*/

#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include "memory/mem.h"
#include "kvt/Kvt.h"
#include "map/Lookup.h"
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

/** @brief

These are the socket state values.

*/

#define SOCKET_DISABLED   0
#define SOCKET_OFFLINE    1
#define SOCKET_ONLINE     3

static int  _lookupSocketOffset(SAS_Name, SAS_Session);
static void _setSocketState(SAS_Name, SAS_Session, uint32_t state);
static void _enableIbMetrics(SAS_Session, uint32_t socket);      
static void _enableObMetrics(SAS_Session, uint32_t socket);      

/*
** ++
**
** This function resolves the attributes associated with a socket.
**
** --
*/

const SAS_Attributes* SAS_GetAttributes(SAS_Name name, SAS_Session s)
  {
  int offset;
  offset = _lookupSocketOffset(name,s);
  if(offset == -1) return NULL;
  return OCMMAP(s->ocm)->plugins[offset].attrs;
  }


/*
** ++
**
** This function resolves the errors associated with a socket.
**
** --
*/

const SAS_Errors* SAS_GetErrors(SAS_Name name, SAS_Session s)
  {
  int offset = _lookupSocketOffset(name,s);
  if(offset == -1) return NULL;
  SocketMgmt *mgmt = (SocketMgmt*)(s->axi+SOCKET_AXI_BASE+(offset*SOCKET_AXI_SIZE));
  return (SAS_Errors*)&mgmt->errors;
  }

/*
** ++
**
** This function resolves the metrics associated with a socket.
**
** --
*/

const SAS_Metrics* SAS_GetMetrics(SAS_Name name, SAS_Session s)
  {
  int offset = _lookupSocketOffset(name,s);
  if(offset == -1) return NULL;
  return (SAS_Metrics*)&OCMMAP(s->ocm)->metricsTable[offset*METRICS_MAX_SOCKET];
  }

/*
** ++
**
** This function resolves the metrics associated with a socket.
**
** --
*/

const SAS_Faults* SAS_GetFaults(SAS_Name name, SAS_Session s)
  {
  int offset = _lookupSocketOffset(name,s);
  if(offset == -1) return NULL;
  return (SAS_Faults*)&OCMMAP(s->ocm)->faultTable[offset];
  }

/*
** ++
**
** This function returns a pointer to a plug-ins I/O registers.
**
** --
*/

void* SAS_Registers(SAS_Name name, SAS_Session s)
  {
  int socket = _lookupSocketOffset(name,s);
  if(socket == -1) return NULL;
  uint32_t region = OCMMAP(s->ocm)->plugins[socket].region;
  uint32_t offset = OCMMAP(s->ocm)->plugins[socket].offset;
  if(!region) return NULL;
  uint32_t addr = LookupRegion(region,offset);
  if(!addr)
    return NULL;
  else
    return (void*)(addr);
  }

/*
** ++
**
** This function sets a plug-in state to online.
**
** --
*/

void SAS_On(SAS_Name name, SAS_Session s)
  {
  _setSocketState(name,s,SOCKET_ONLINE);
  }

/*
** ++
**
** This function sets a plug-in state to offline.
**
** --
*/

void SAS_Off(SAS_Name name, SAS_Session s)
  {
  _setSocketState(name,s,SOCKET_OFFLINE);
  }

/*
** ++
**
** This function resets a plug-in by disabling it and.
** then setting it to the offline state.
**
** --
*/

void SAS_Reset(SAS_Name name, SAS_Session s)
  {
  
  /* SEMTODO: more complexity to be dealt with in reset */
  _setSocketState(name,s,SOCKET_DISABLED);
  _setSocketState(name,s,SOCKET_OFFLINE);
  }
  
/*
** ++
**
** This function sets a plug-in state to online.
**
** --
*/

static void _setSocketState(SAS_Name name, SAS_Session s, uint32_t state)
  {
  uint32_t offset;
      
  offset = (uint32_t)KvtLookup(KvtHash(name),(KvTable)(s->ocm+OCMMAP(s->ocm)->socketKvt));
  if(offset)
    {
    int socket = (offset-SOCKET_AXI_FIFO_BASE-SOCKET_AXI_BASE)/SOCKET_AXI_SIZE;
    if(state == SOCKET_ONLINE)
      {
      _enableIbMetrics(s,socket);
      _enableObMetrics(s,socket);
      }
    ((SocketMgmt*)(s->axi+(offset - SOCKET_AXI_FIFO_BASE)))->state = state;
    }
  }

/*
** ++
**
** This function returns the offset of a socket
**
** --
*/

static int _lookupSocketOffset(SAS_Name name, SAS_Session s)
  {
  int offset = (uint32_t)KvtLookup(KvtHash(name),(KvTable)(s->ocm+OCMMAP(s->ocm)->socketKvt));
  if(offset)
    return (offset-SOCKET_AXI_FIFO_BASE-SOCKET_AXI_BASE)/SOCKET_AXI_SIZE;
  else
    return -1;
  }

/*
** ++
**
** This function enables the inbound metrics interrupts
** for a given socket.
**
** --
*/

static void _enableIbMetrics(SAS_Session s, uint32_t socket)
  {
  InterruptCtl *intrCtl = (InterruptCtl*)(s->axi+INTR_GROUP_AXI_BASE);
  uint32_t enable = 0;
  
  enable |= 1<<OCMMAP(s->ocm)->intrMap[SAS_INTR_PPI_IBFULL_BASE+socket].source;
  
  /* now enable this interrupt set for the metrics group */
  intrCtl[SAS_IRQ_METRICS].enable = enable;
  }

/*
** ++
**
** This function enables the outbound metrics interrupts
** for a given socket.
**
** --
*/

static void _enableObMetrics(SAS_Session s, uint32_t socket)
  {
  InterruptCtl *intrCtl = (InterruptCtl*)(s->axi+INTR_GROUP_AXI_BASE);
  uint32_t enable = 0;
  
  enable |= 1<<OCMMAP(s->ocm)->intrMap[SAS_INTR_PPI_OBEMPTY_BASE+socket].source;
  enable |= 1<<OCMMAP(s->ocm)->intrMap[SAS_INTR_PPI_OBFULL_BASE+socket].source;
  
  /* now enable this interrupt set for the metrics group */
  intrCtl[SAS_IRQ_METRICS].enable = enable;
  }
