/*
** ++
**  Package: SAS
**	
**
**  Abstract: SAS system level routines.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include "memory/mem.h"
#include "map/Lookup.h"
#include "kvt/Kvt.h"
#include "svt/Svt.h"

#include "sas/Sas.h"

#include "Map.h"
#include "Transaction.h"
#include "Interrupt.h"

//#define DEBUG

#ifdef DEBUG
#define debug_printf printf
#else
#define debug_printf(...)
#endif

/*
**
** This constant specifies the maximum number of sockets that can be 
** managed by the kvt. Note that this value must be expressed as power of two... 
**
*/

#define MAX_SOCKET_ENTRIES 2 // As a power of 2, i.e., 2 = 4 entries 

/*
**
** This constant specifies the maximum number of user mailboxes that can be 
** managed by the kvt. Note that this value must be expressed as power of two... 
**
*/

#define MAX_APP_ENTRIES    4 // As a power of 2, i.e., 4 = 16 entries 

/*
** ++
**
** Local private variables
**
** --
*/

static uint32_t      _ocmNext  = 0;
static InterruptMap *_intrMap  = 0;

/*
** ++
**
** Forward references for local (private functions)...
**
** --
*/

/* called by SAS_Construct */
static SAS_Status _setupKvt(void);
static SAS_Status _configureInterrupts(void);

/* called by SAS_Construct */
static void _constructSessionList(void);
static void _constructRundwnList(void);
static void _enableSystemMetrics(void);

/* called by SAS_Plugin */
static SAS_Status _configureObDescriptors(Socket* s, const SAS_Attributes *attrs);
static SAS_Status _configureIbDescriptors(Socket* s, const SAS_Attributes *attrs);

/* called by SAS_Close */
static void _freeMbx(Mbx m, SAS_Session s);

/*
** ++
**
** This function performs the one time initialization
** of the SAS facility.
**
** --
*/

SAS_Status SAS_Construct(void)
  {
  SAS_Status error;

  debug_printf("OcmMap 0x%x reserved size %d bytes\n",(int)SasOcmMap,sizeof(OcmMap));

  /* clear context */
  memset((void*)SasOcmMap,0,sizeof(OcmMap));
    
  /* advance the ocm next available location pointer, align to word boundary  */
  _ocmNext = (_ocmNext+sizeof(OcmMap)+15) & ~0xf;
  
  /* install the SVT */
  _intrMap = InstallSvt();
  if(!_intrMap) return SAS_NO_SVT;
  
  /* SEMTODO: can I avoid copying the interrupt map to the ocm */
  memcpy((void*)&SasOcmMap->intrMap,(void*)_intrMap,sizeof(SasOcmMap->intrMap));
  
  /* create the socket and list kvts */
  error = _setupKvt();
  if(error) return error;
      
  /* populate the session free list */
  _constructSessionList();
  
  /* build the rundown free-list */
  _constructRundwnList();

  /* initialize the interrupt map */
  error = _configureInterrupts();
  if(error) return error;

  /* enable the system metrics interrupt sources */
  _enableSystemMetrics();
    
  /* reserve the socket descriptors pool in the ocm */
  SasOcmMap->descriptorFree = (_ocmNext+31) & ~0x1f;
  SasOcmMap->descriptorBase = SasOcmMap->descriptorFree;
  debug_printf("reserve descriptors at base 0x%x\n",SasOcmMap->descriptorBase);
    
  return SAS_SUCCESS;
  }

/*
** ++
**
** This function assigns a plug-in to a socket.
** This function is a system service and is not re-entrant!
**
** --
*/

SAS_Status SAS_Plugin(const SAS_Preferences* prefs)
  {
  uint32_t socket;
  KvtValue ret;
  SAS_Status error = SAS_SUCCESS;
    
  /* SEMTODO: socket to plugin binding will change with partial reconfiguration  */
  uint32_t slot = atoi(prefs->bitfile);
    
  /* validate the requested slot */
  if(slot >= MAX_SOCKETS) return SAS_BAD_SLOT;

  /* assign the axi offset for I/O */    
  socket = SOCKET_AXI_BASE+(SOCKET_AXI_SIZE*slot)+SOCKET_AXI_FIFO_BASE;
          
  /* bind the plug-in name to the socket axi in the kvt */
  debug_printf("Insert socket 0x%x in kvt 0x%x 0x%x\n",
               (int)socket,(int)SasOcm,(int)SasOcmMap->socketKvt);
  ret = KvtInsert(KvtHash(prefs->name), (void*)socket,(KvTable)(SasOcm+SasOcmMap->socketKvt));    
  if(ret)
    {
    /* configure socket ob descriptor pool */
    error = _configureObDescriptors((Socket *)(SasAxi+socket),prefs->attrs);

    /* configure socket ib descriptor pool */
    if(error == SAS_SUCCESS)
      error = _configureIbDescriptors((Socket *)(SasAxi+socket),prefs->attrs);

    if(error)
      KvtRemove(KvtHash(prefs->name),(KvTable)(SasOcm+SasOcmMap->socketKvt));
    else
      SasOcmMap->plugins[slot] = *prefs;  /* save the preferences */
      
    debug_printf("Plugin %s to slot %d\n",prefs->name,slot);
    }
  else error = SAS_ERR_KVT;
  
  return error;
  }

/*
** ++
**
** This function registers an application name with a fifo slot.
** This function is a system service and is not re-entrant!
**
** --
*/

SAS_Status SAS_AppRegister(const SAS_AppPreferences* attrs)
  {
  KvtValue ret;
  SAS_Status error = SAS_SUCCESS;
  
  /* SEMTODO: remove workaround for missing fw app fifo interface */
  /* validate the requested slot */
  if(attrs->slot >= MAX_APP_CHANNELS) return SAS_BAD_SLOT;
          
  SasOcmMap->appTable[attrs->slot] = attrs->offset;
  /* bind the plug-in name to the socket axi in the kvt */
  debug_printf("Insert app in kvt 0x%x 0x%x\n",(int)SasOcm,(int)SasOcmMap->appKvt);
  ret = KvtInsert(KvtHash(attrs->name), (void*)((attrs->slot+1)<<1),(KvTable)(SasOcm+SasOcmMap->appKvt));    
  if(!ret) error = SAS_ERR_KVT;

  return error;
  }

/*
** ++
**
** This function creates a new session.
** Each new session maintains its own memory map
** of the ocm and axi space.
**
** --
*/

SAS_Session SAS_Open(void)
  {
  /* map the axi */
  Axi axi = LookupAxiSocket();
  if(!axi) return SAS_OPEN_ERROR;

  /* map the ocm */
  Ocm ocm = LookupOcm();
  if(!ocm) return SAS_OPEN_ERROR;

  /* allocate a session struct offset */
  uint32_t offset = UTIL(axi)->session;
  if(SAS_DECODE(INPUT_EMPTY,offset))
    {
    debug_printf("Error allocating session offset\n");
    return SAS_OPEN_ERROR;
    }

  offset = SAS_DECODE(FREELIST_VAL,offset);
    
  /* ocm map the session */
  SAS_Session s = &((OcmMap*)(ocm))->sessions[offset];

  /* allocate OS specific interrupt synchronization */
  if(IntrAllocSync(&s->sync) != SAS_SUCCESS) return SAS_OPEN_ERROR;
  
  /* set the maps */
  s->ocm = ocm;
  s->axi = axi;
  
  /* initialize the input source array */
  s->srcs = 0;
  s->enabled = 0;
  
  debug_printf("Allocated session 0x%x-0x%x\n",(int)offset,(int)s);
  
  return s;
  }

/*
** ++
**
** This function closes a previously open session.
**
** --
*/

void SAS_Close(SAS_Session s)
  {
  /* release OS specific interrupt synchronization */
  if(s->sync)
    IntrFreeSync(&s->sync);
      
  /* iterate input list and free bound mailboxes */
  while(s->srcs)
    {
    _freeMbx(&s->mbx[--s->srcs],s);
    }

  /* free the session offset */
  uint32_t fval = 0;
  SAS_ENCODE(FREELIST_VAL,((int)s-offsetof(OcmMap,sessions)-SasOcm)/sizeof(struct _SAS_Session),fval);
  UTIL(s->axi)->session = fval;

  /* umap session axi */
  TeardownAxiSocket(s->axi);
  
  Ocm ocm = s->ocm;
  
  /* clear session memory */
  memset(s,0,sizeof(struct _SAS_Session));

  /* umap session ocm */
  TeardownOcm(ocm);
  }

/*
** ++
**
** This function converts a virtual address to physical.
**
** --
*/

void* SAS_VirtToPhy(void* addr)
  {
  return (void*)mem_getPhysWrite((uint32_t)addr);
  }

/*
** ++
**
** This function creates the socket and mailbox lookup key value tables.
**
** --
*/

static SAS_Status _setupKvt(void)
  {
  KvTable kvt = KvtConstruct(MAX_SOCKET_ENTRIES,(void *)(SasOcm+_ocmNext));
  
  if(!kvt) return SAS_NO_KVT;
  
  debug_printf("Created socketKvt at 0x%x-0x%x\n",(int)kvt,(int)kvt-SasOcm);
  
  SasOcmMap->socketKvt = (int)kvt-SasOcm;
  
  /* advance the ocm next available location pointer, align to word boundary  */
  _ocmNext = (_ocmNext+KvtSizeof(MAX_SOCKET_ENTRIES)+15) & ~0xf;

  debug_printf("_setupKvt socket kvt 0x%x, next 0x%x\n",
               (int)SasOcmMap->socketKvt,(int)_ocmNext);  

  kvt = KvtConstruct(MAX_APP_ENTRIES,(void *)(SasOcm+_ocmNext));
  
  if(!kvt) return SAS_NO_KVT;
  
  SasOcmMap->appKvt = (int)kvt-SasOcm;
  
  /* advance the ocm next available location pointer, align to word boundary  */
  _ocmNext = (_ocmNext+KvtSizeof(MAX_APP_ENTRIES)+15) & ~0xf;

  debug_printf("_setupKvt mailbox kvt 0x%x, next 0x%x\n",
               (int)SasOcmMap->appKvt,(int)_ocmNext);  
  
  return SAS_SUCCESS;
  }

/*
** ++
**
** This routine populates the session free-list utility fifo.
**
** --
*/

static void _constructSessionList(void)
  {
  uint32_t fval;
  uint32_t i;
      
  /* session free list */
  for (i=0; i<MAX_SESSIONS; i++)
    {
    memset((void*)&SasOcmMap->sessions[i],0x0,sizeof(struct _SAS_Session ));
    fval = 0;
    SAS_ENCODE(FREELIST_VAL,i,fval);
    UTIL(SasAxi)->session = fval;
    }
      
  debug_printf("_constructSessionList complete\n");
  }

/*
** ++
**
** This routine initializes the rundown mailbox
** free-list utility fifo.
**
** --
*/

static void _constructRundwnList(void)
  {
  uint32_t fval = 0;
  uint32_t i;
    
  /* rundown mailbox free list */
  for (i=0; i<MAX_RUNDWN_CHANNELS; i++)
    {
    fval = 0;    
    SAS_ENCODE(FREELIST_VAL,i,fval);
    UTIL(SasAxi)->rundwn = fval;
    }

  debug_printf("_constructRundwnMbx complete\n");
  }
  
/*
** ++
**
** This function populates the outbound descriptor
** free-list fifos for a given socket.
** This function is a system service and is not re-entrant (descriptorFree)!
**
** --
*/

static SAS_Status _configureObDescriptors(Socket *s, const SAS_Attributes* attrs)
  {
  uint32_t instr = 0;
  int i;

  /* validate descriptor attrs */
  uint32_t size = attrs->obFd * (sizeof(SAS_Fd)+ (attrs->moh<<3));
  if((size+SasOcmMap->descriptorFree) > mem_Region_size(MEM_REGION_OCM))
    return SAS_INSF_OCM;

  /* fill the outbound descriptor free-lists by posting a
     descriptor free instruction to the outbound engine */

  /* SEMTODO: warning - descriptorFree is not thread safe */  
  debug_printf("socket 0x%x ob desc base 0x%x\n",(int)s,(int)SasOcmMap->descriptorFree);

  /* initialize the instruction */
  for(i=0; i<attrs->obFd; i++)
    {
    /* encode the instruction */    
    OB_INSTR_INIT(instr);
    SAS_ENCODE(OB_OPCODE,SAS_OB_FREE,instr);
    SAS_ENCODE(FRAME_DESC,SasOcmMap->descriptorFree,instr);
    
    /* post the instruction */
    s->obWorkList = instr;
    
    debug_printf("write ob addr 0x%x instr 0x%x base 0x%x\n",
                 (int)&s->obWorkList,(int)instr,(int)SasOcmMap->descriptorFree);
    
    /* advance to the next descriptor offset */
    SasOcmMap->descriptorFree = ((SasOcmMap->descriptorFree+sizeof(SAS_Fd)+(attrs->moh<<3))+31) & ~0x1f;    
    }
  debug_printf("_configureObDescriptors success base 0x%x\n",
               (int)SasOcmMap->descriptorFree);
  return SAS_SUCCESS;
  }

/*
** ++
**
** This function populates the inbound descriptor
** free-list fifos for a given socket.
** This function is a system service and is not re-entrant (descriptorFree)!
**
** --
*/

static SAS_Status _configureIbDescriptors(Socket *s, const SAS_Attributes* attrs)
  {
  uint32_t instr;
  int i;
    
  /* validate descriptor attrs */
  uint32_t size = attrs->ibFd * (sizeof(SAS_Fd)+(attrs->mib<<3));
  if((size+SasOcmMap->descriptorFree) > mem_Region_size(MEM_REGION_OCM))
    return SAS_INSF_OCM;
  
  /* fill the inbound descriptor free-lists by posting a
     descriptor free instruction to the inbound engine */

  /* SEMTODO: warning - descriptorFree is not thread safe */  
  debug_printf("socket 0x%x ib desc base 0x%x\n",(int)s,(int)SasOcmMap->descriptorFree);
     
  for(i=0; i<attrs->ibFd; i++)
    {    
    /* encode the instruction */
    instr = 0;
    SAS_ENCODE(IB_OPCODE,SAS_IB_FREE,instr);    
    SAS_ENCODE(FRAME_DESC,SasOcmMap->descriptorFree,instr);
    
    /* post the instruction */
    s->ibWorkList = instr;

    debug_printf("write ib addr 0x%x instr 0x%x base 0x%x\n",
                 (int)&s->ibWorkList,(int)instr,(int)SasOcmMap->descriptorFree);
    
    /* advance to the next descriptor offset */
    SasOcmMap->descriptorFree = ((SasOcmMap->descriptorFree+sizeof(SAS_Fd)+(attrs->mib<<3))+31) & ~0xf;
    }
  debug_printf("_configureIbDescriptors success base 0x%x\n",
               (int)SasOcmMap->descriptorFree);
  return SAS_SUCCESS;
  }

/*
** ++
**
** This function frees an allocated mailbox.
**
** --
*/

static void _freeMbx(Mbx m, SAS_Session s)
  {
  uint32_t fval = 0;
      
  if(m->intrBase == SAS_INTR_RUNDWN_BASE)
    {
    debug_printf("Free rundwn mbx %d\n",m->mid);
    SAS_ENCODE(FREELIST_VAL,m->mid,fval);
    UTIL(s->axi)->rundwn = fval;
    }
  }

/*
** ++
**
** This function configures the global interrupt mapping.
**
** --
*/

static SAS_Status _configureInterrupts(void)
  {
  SAS_Status error;
  
  InterruptRemap *remap = (InterruptRemap*)(SasAxi+INTR_MAP_AXI_BASE);
  int i;
  
  /* allocate and initialize the interrrupt synchronization table */
  memset((void*)&SasOcmMap->intrTable,0x0,sizeof(InterruptTable));

  /* invert the configuration map and apply the remap */ 
  for(i=0;i<SAS_INTR_COUNT;i++)
    {
    if(_intrMap[i].enable)
      remap->src[_intrMap[i].group][_intrMap[i].source] = INTR_MAP_ENAB_MASK | i;
    }
    
  /* initialize interrupt lines */
  error = IntrInit();
  if(error) return error;
  
  debug_printf("_configureInterrupts ok\n");
  
  return SAS_SUCCESS;
  }

/*
** ++
**
** This function enables the system metrics interrupt sources.
**
** --
*/

static void _enableSystemMetrics(void)
  {
  InterruptCtl *intrCtl = (InterruptCtl*)(SasAxi+INTR_GROUP_AXI_BASE);
  uint32_t enable = 0;
  int i;
  
  /* clear the metrics storage */
  memset((void*)&SasOcmMap->metricsTable[0],0,sizeof(SasOcmMap->metricsTable));
  memset((void*)&SasOcmMap->faultTable[0],0,sizeof(SasOcmMap->faultTable));
  
  for(i=0;i<MAX_UTIL_CHANNELS; i++)
    {
    /* enable the session free-list empty interrupt */
    enable |= 1<<_intrMap[SAS_INTR_UTIL_PEND_BASE+i].source;
    }

  for(i=0;i<MAX_UTIL_CHANNELS; i++)
    {
    /* enable the mailbox free-list empty interrupt */
    enable |= 1<<_intrMap[SAS_INTR_UTIL_EMPTY_BASE+i].source;
    }

  /* enable the rundown fault interrupt */  
  enable |= 1<<_intrMap[SAS_INTR_RUNDWNERR_PEND].source;
  
  /* now enable this interrupt set for the metrics group */
  intrCtl[SAS_IRQ_METRICS].enable = enable;
  
  /* now enable this interrupt set for the metrics group */
  intrCtl[SAS_IRQ_FAULTS].enable = enable;
  
  }
