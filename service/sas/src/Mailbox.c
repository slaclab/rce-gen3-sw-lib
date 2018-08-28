/*
** ++
**  Package: SAS
**
**
**  Abstract: SAS mailbox allocation and input handling.
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

#include "sas/Sas.h"
#include "kvt/Kvt.h"
#include "map/Lookup.h"

#include "Map.h"
#include "Transaction.h"
#include "Interrupt.h"

//#define DEBUG

#ifdef DEBUG
#include <stdio.h>
#define debug_printf printf
#else
#define debug_printf(...)
#endif

#define PROFILE 

/*
** ++
**
** Mailbox access macros
**
** --
*/

#define MBX_ID(m)            (m->m.mid)

#define MBX_ENABLE(m)        (m->m.s->enabled |= (1<<m->m.idx))

#define MBX_ISENABLED(m)    ((m->m.s->enabled & (1<<m->m.idx)) ? 1 : 0)

/*
** ++
**
** Forward references for local (private functions)...
**
** --
*/

static Socket*  _lookupSocket(SAS_Name, SAS_Session);
static uint32_t _lookupApp(SAS_Name, SAS_Session);
static void     _bindMbxIntr(SAS_Session, Mbx, uint32_t intrBase);
static Mbx      _bindMbx(SAS_Session,
                         Socket*,
                         uint32_t*,
                         Handler,
                         uint8_t,
                         void*);
/*
** ++
**
** This function waits on input from session mailboxes.
**
**  session: a data structure located in OCM
**  mailbox: a data structure located in OCM
**
**  A mailbox is bound to a single source of input (AXI fifo).
**  A session maintains an array of up to 8 mailboxes.
**
**  source:  an AXI fifo referenced in a mailbox
**  input:   a value returned from a source
**  handler: a routine to execute with valid input
**
**  Function Execution:
**
**  States:
**    Continue - process inputs for all enabled mailboxes
**    Abort - abort input processing for all enabled mailboxes and exit
**    Disable - disable input processing for a mailbox and continue
**
**  1 Load each input to a floating point register
**
**  2 Process inputs:
**    Move input from floating point register to core register.
**    When input is valid:
**      Execute handler and check return code       
**        Latch a handler abort or disable state
**
**  3 If a handler latched an abort, exit function.
**
**  4 If any input was valid, return to 1.
**
**  5 Enable each source interrupt.
**
**  6 Block until a source interrupts.
**
**  7 Return to 1.
**    
**
** --
*/

void SAS_Wait(SAS_Session s)
  {
  uint32_t active = s->enabled;

  static void* move_table[] = {
  &&mov_s16,&&mov_s17,&&mov_s18,&&mov_s19,
  &&mov_s20,&&mov_s21,&&mov_s22,&&mov_s23};

  if(!active) return;
  
  while(1)
    {
    uint32_t addr;
    
    __asm__ volatile("tst %0,#0x01\t\n"::"r" (active) : "cc");
    __asm__ volatile("ldrne %0,[%1]\t\n":"=r" (addr) : "r" (s->pend));
    __asm__ volatile("vldrne s16,[%0]\t\n"::"r" (addr) : "s16");
    
    __asm__ volatile("tst %0,#0x02\t\n"::"r" (active) : "cc");
    __asm__ volatile("ldrne %0,[%1,#4]\t\n":"=r" (addr) : "r" (s->pend));
    __asm__ volatile("vldrne s17,[%0]\t\n"::"r" (addr) : "s17");
    
    __asm__ volatile("tst %0,#0x04\t\n"::"r" (active) : "cc");
    __asm__ volatile("ldrne %0,[%1,#8]\t\n":"=r" (addr) : "r" (s->pend));
    __asm__ volatile("vldrne s18,[%0]\t\n"::"r" (addr) : "s18");
    
    __asm__ volatile("tst %0,#0x08\t\n"::"r" (active) : "cc");
    __asm__ volatile("ldrne %0,[%1,#12]\t\n":"=r" (addr) : "r" (s->pend));
    __asm__ volatile("vldrne s19,[%0]\t\n"::"r" (addr) : "s19");
    
    __asm__ volatile("tst %0,#0x10\t\n"::"r" (active) : "cc");
    __asm__ volatile("ldrne %0,[%1,#16]\t\n":"=r" (addr) : "r" (s->pend));
    __asm__ volatile("vldrne s20,[%0]\t\n"::"r" (addr) : "s20");
    
    __asm__ volatile("tst %0,#0x20\t\n"::"r" (active) : "cc");
    __asm__ volatile("ldrne %0,[%1,#20]\t\n":"=r" (addr) : "r" (s->pend));
    __asm__ volatile("vldrne s21,[%0]\t\n"::"r" (addr) : "s21");
    
    __asm__ volatile("tst %0,#0x40\t\n"::"r" (active) : "cc");
    __asm__ volatile("ldrne %0,[%1,#24]\t\n":"=r" (addr) : "r" (s->pend));
    __asm__ volatile("vldrne s22,[%0]\t\n"::"r" (addr) : "s22");
    
    __asm__ volatile("tst %0,#0x80\t\n"::"r" (active) : "cc");
    __asm__ volatile("ldrne %0,[%1,#28]\t\n":"=r" (addr) : "r" (s->pend));
    __asm__ volatile("vldrne s23,[%0]\t\n"::"r" (addr) : "s23");

    /*
     * The state variable:
     *   is initialized to zero
     *   increments with each valid input
     *   or'd with SAS_ABORT to signal exit
     *   or'd with SAS_DISABLE to disable a source
     *   when zero, will be used to enter a wait for interrupt state
     */
    uint32_t state = 0;

    /* check inputs and execute handlers */
    while(active)
      {
      /* get the mailbox index of the active source */
      uint32_t mbx = __builtin_ffs(active)-1;
      
      /* reserve r0 for the input value */
      register uint32_t input asm("r0");
      
      /* set the link register to the handler service block */
      __asm__ volatile goto ("adr lr,[%l0]\t\n"::::service);
      
      /* execute move block for the active mailbox */
      goto *move_table[mbx];
      
mov_s16:
      __asm__ volatile("vmov %0, s16\t\n":"=r" (input));
      __asm__ volatile("tst %0,#1\t\n"::"r" (input) : "cc");
      __asm__ volatile goto ("adrne lr,[%l0]\t\n"::::retire);      
      __asm__ volatile("mov pc,lr\t\n");
      
mov_s17:       
      __asm__ volatile("vmov %0, s17\t\n":"=r" (input));
      __asm__ volatile("tst %0,#1\t\n"::"r" (input) : "cc");
      __asm__ volatile goto ("adrne lr,[%l0]\t\n"::::retire);      
      __asm__ volatile("mov pc,lr\t\n");
      
mov_s18:       
      __asm__ volatile("vmov %0, s18\t\n":"=r" (input));
      __asm__ volatile("tst %0,#1\t\n"::"r" (input) : "cc");
      __asm__ volatile goto ("adrne lr,[%l0]\t\n"::::retire);      
      __asm__ volatile("mov pc,lr\t\n");
      
mov_s19:       
      __asm__ volatile("vmov %0, s19\t\n":"=r" (input));
      __asm__ volatile("tst %0,#1\t\n"::"r" (input) : "cc");
      __asm__ volatile goto ("adrne lr,[%l0]\t\n"::::retire);      
      __asm__ volatile("mov pc,lr\t\n");
      
mov_s20:       
      __asm__ volatile("vmov %0, s20\t\n":"=r" (input));
      __asm__ volatile("tst %0,#1\t\n"::"r" (input) : "cc");
      __asm__ volatile goto ("adrne lr,[%l0]\t\n"::::retire);      
      __asm__ volatile("mov pc,lr\t\n");
      
mov_s21:       
      __asm__ volatile("vmov %0, s21\t\n":"=r" (input));
      __asm__ volatile("tst %0,#1\t\n"::"r" (input) : "cc");
      __asm__ volatile goto ("adrne lr,[%l0]\t\n"::::retire);      
      __asm__ volatile("mov pc,lr\t\n");
      
mov_s22:       
      __asm__ volatile("vmov %0, s22\t\n":"=r" (input));
      __asm__ volatile("tst %0,#1\t\n"::"r" (input) : "cc");
      __asm__ volatile goto ("adrne lr,[%l0]\t\n"::::retire);      
      __asm__ volatile("mov pc,lr\t\n");
      
mov_s23:       
      __asm__ volatile("vmov %0, s23\t\n":"=r" (input));
      __asm__ volatile("tst %0,#1\t\n"::"r" (input) : "cc");
      __asm__ volatile goto ("adrne lr,[%l0]\t\n"::::retire);

service:
      /* execute mailbox input handler */
      state |= ((SAS_ABORT | SAS_DISABLE) & 
               s->mbx[mbx].handler(input,s->mbx[mbx].context,&s->mbx[mbx]));
      /* check for handler abort and disable */
      if(state++ & SAS_DISABLE)
        {
        s->enabled &= ~(1<<mbx);
        state &= ~SAS_DISABLE;
#ifdef PROFILE      
        if(s->profile) s->disable++;
#endif      
        }
        
retire:
#ifdef PROFILE        
      if(s->profile && !SAS_DECODE(INPUT_EMPTY,input))
        s->valid++;
      else if(s->profile)
        s->invalid++;
#endif
      /* mark the source as serviced */ 
      active &= ~(1<<mbx);
      }
      
    /* set active sources */
    active = s->enabled;
    
    /* exit on abort or all disabled */
    if((state & SAS_ABORT) || !active) return;
 
    /* wait if all inputs were invalid */
    state = !state ? s->srcs : 0;

    /* arm interrupt for enabled sources */
    while(state--)
      {
      if(active & (1<<state))
        *(uint32_t*)(s->axi+s->mbx[state].enab) = 1 << s->mbx[state].mask;
      /* wait for an input source to interrupt */
      if(!state) IntrWaitSync(&s->sync);
#ifdef PROFILE      
      if(s->profile) s->wait++;
#endif      
      }
    }
  }

/*
** ++
**
** This function enables all mailboxes and waits on session input
**
** --
*/

void SAS_EnableWait(SAS_Session s)
  {
  int i;
  
  for(i=0;i<s->srcs;i++)
    s->enabled |= 1<<i;
  
  SAS_Wait(s);    
  }

/*
** ++
**
** This function allocates and binds a rundown mailbox to a session.
**
** --
*/

SAS_Mbx SAS_Bind(SAS_Handler handler, SAS_Arg ctx, SAS_Session s)
  {
  if((s->srcs+1) > MAX_SESSION_INPUTS) return (SAS_Mbx)0;
  
  /* allocate a rundown mailbox offset */
  uint32_t offset = UTIL(s->axi)->rundwn;
  if(SAS_DECODE(INPUT_EMPTY,offset)) return (SAS_Mbx)0;
  offset = SAS_DECODE(FREELIST_VAL,offset);
  
  /* bind the mailbox to the session */
  Mbx m = _bindMbx(s, 
                   NULL,                                                                  
                   (uint32_t*)(s->axi+RUNDWN_AXI_BASE+(offset*sizeof(uint32_t))),         
                   (Handler)handler,                                                               
                   offset,                                                                
                   ctx);                                                                  
  
  /* bind the source interrupt to the mailbox */
  _bindMbxIntr(s,m,SAS_INTR_RUNDWN_BASE);
  
  return (SAS_Mbx)m;
  }

/*
** ++
**
** This function binds an inbound socket mailbox to a session.
**
** --
*/

SAS_IbMbx SAS_IbBind(SAS_Name name, SAS_IbHandler handler, SAS_Arg ctx, SAS_Session s)
  {
  if((s->srcs+1) > MAX_SESSION_INPUTS) return (SAS_IbMbx)0;
  
  /* lookup the socket */  
  Socket *sock = _lookupSocket(name,s);
  if(!sock) return (SAS_IbMbx)0;
    
  /* bind the mailbox to the session */
  Mbx m = _bindMbx(s, sock, (uint32_t*)&sock->ibPendList,(Handler)handler,SOCKID(sock,s->axi),ctx);
  
  /* bind the source interrupt to the mailbox */
  _bindMbxIntr(s,m,SAS_INTR_PPI_IBPEND_BASE);
  
  return (SAS_IbMbx)m;
  }

/*
** ++
**
** This function allocates and binds a rundown mailbox to a session.
**
** --
*/

SAS_ObMbx SAS_ObBind(SAS_Name name, SAS_ObHandler handler, SAS_Arg ctx, SAS_Session s)
  {
  if((s->srcs+1) > MAX_SESSION_INPUTS) return (SAS_ObMbx)0;
  
  /* lookup the socket */
  Socket *sock = _lookupSocket(name,s);
  if(!sock) return (SAS_ObMbx)0;
  
  /* allocate a rundown mailbox offset */
  uint32_t offset = UTIL(s->axi)->rundwn;
  if(SAS_DECODE(INPUT_EMPTY,offset)) return (SAS_ObMbx)0;
  offset = SAS_DECODE(FREELIST_VAL,offset);
  
  /* bind the mailbox to the session */
  Mbx m = _bindMbx(s, 
                   sock,
                   (uint32_t*)(s->axi+RUNDWN_AXI_BASE+(offset*sizeof(uint32_t))),
                   (Handler)handler,
                   offset,
                   ctx);
                   
  /* bind the source interrupt to the mailbox */  
  _bindMbxIntr(s,m,SAS_INTR_RUNDWN_BASE);
  
  return (SAS_ObMbx)m;
  }

/*
** ++
**
** This function binds an application mailbox to a session.
**
** --
*/

SAS_AppMbx SAS_AppBind(SAS_Name name, SAS_AppHandler handler, SAS_Arg ctx, SAS_Session s)
  {
  if((s->srcs+1) > MAX_SESSION_INPUTS) return (SAS_AppMbx)0;

  /* lookup the app mailbox index, 1 based */
  uint32_t offset = _lookupApp(name,s);
  if(!offset) return (SAS_AppMbx)0;
  
  offset = (offset>>1)-1;
  
  /* SEMTODO: remove workaround for missing fw app fifo interface */
  
  /* bind the mailbox to the session */
  Mbx m = _bindMbx(s,
                   NULL,                                                      
                   (uint32_t*)OCMMAP(s->ocm)->appTable[offset],             
                   (Handler)handler,                                                   
                   offset,                                                  
                   ctx);                                                      
  
  /* bind the source interrupt to the mailbox */
  _bindMbxIntr(s,m,SAS_INTR_APP_BASE);
  
  return (SAS_AppMbx)m;
  }

/*
** ++
**
** This function returns the ID associated with a mailbox.
**
** --
*/

SAS_MbxId SAS_Id(SAS_Mbx m)
  {
  return MBX_ID(m);
  }

/*
** ++
**
** This function returns the ID associated with a mailbox.
**
** --
*/

SAS_MbxId SAS_ObId(SAS_ObMbx m)
  {
  return MBX_ID(m);
  }

/*
** ++
**
** This function enables a mailbox input source.
**
** --
*/

void SAS_Enable(SAS_Mbx m)
  {  
  MBX_ENABLE(m);
  }

/*
** ++
**
** This function returns the enable status of a mailbox input source.
**
** --
*/

int SAS_IsEnabled(SAS_Mbx m)
  {
  return MBX_ISENABLED(m);
  }

/*
** ++
**
** This function enables a mailbox input source.
**
** --
*/

void SAS_IbEnable(SAS_IbMbx m)
  {  
  MBX_ENABLE(m);
  }

/*
** ++
**
** This function returns the enable status of a mailbox input source.
**
** --
*/

int SAS_IbIsEnabled(SAS_IbMbx m)
  {
  return MBX_ISENABLED(m);
  }

/*
** ++
**
** This function enables a mailbox input source.
**
** --
*/

void SAS_ObEnable(SAS_ObMbx m)
  {
  MBX_ENABLE(m);
  }

/*
** ++
**
** This function returns the enable status of a mailbox input source.
**
** --
*/

int SAS_ObIsEnabled(SAS_ObMbx m)
  {
  return MBX_ISENABLED(m);
  }

/*
** ++
**
** This function enables a mailbox input source.
**
** --
*/

void SAS_AppEnable(SAS_AppMbx m)
  {  
  MBX_ENABLE(m);
  }

/*
** ++
**
** This function returns the enable status of a mailbox input source.
**
** --
*/

int SAS_AppIsEnabled(SAS_AppMbx m)
  {
  return MBX_ISENABLED(m);
  }

/*
** ++
**
** This function binds a mailbox to a session.
**
** --
*/

static Mbx _bindMbx(SAS_Session s, 
                        Socket *sock,        
                        uint32_t* pend,      
                        Handler handler,  
                        uint8_t mid,        
                        void *ctx)     
  {  
  uint32_t offset = s->srcs;
  
  Mbx m = &s->mbx[offset];
    
  /* assign a memory map */
  m->ocm = s->ocm;
      
  /* assign an axi address for the socket */
  m->socket = sock;

  /* install the callback handler routine */
  m->handler = handler;
 
  /* assign the mailbox structure index */
  m->idx = s->srcs;

  /* assign the mailbox ID */
  m->mid = mid;

  /* assign the handler context */
  m->context = ctx;
  
  /* bind the axi address for the pend fifo */
  m->pend = (uint32_t)pend;

  /* bind the session back to the mailbox */
  m->s = s;
  
  /* bind mailbox to session input sources */
  s->pend[s->srcs] = m->pend;
  s->srcs++;
    
  return m;
  }

/*
** ++
**
** This function binds an interrupt source to a mailbox.
**
** --
*/
    
static void _bindMbxIntr(SAS_Session s, Mbx m, uint32_t intrBase)
  {
  InterruptCtl *intrCtl = (InterruptCtl*)(s->axi+INTR_GROUP_AXI_BASE);

  /* get the static interrupt map for the source */
  uint32_t grp = OCMMAP(s->ocm)->intrMap[intrBase+m->mid].group;  
  uint32_t src = OCMMAP(s->ocm)->intrMap[intrBase+m->mid].source; 
    
  /* assign the axi offset to enable interrupts */
  m->enab = (uint16_t)((int)&intrCtl[grp].enable-s->axi);

  /* assign the interrupt source to create an enable bitmask */
  m->mask = src;
  
  /* assign the interrupt base */
  m->intrBase = intrBase;

  /* bind the session sync object to the mailbox interrupt source */
  OCMMAP(s->ocm)->intrTable.sync[grp][src] = s->sync;
  }

/*
** ++
**
** This function looks up and returns an application index.
**
** --
*/

static uint32_t _lookupApp(SAS_Name name, SAS_Session s)
  {
  return (uint32_t) KvtLookup(KvtHash(name),(KvTable)(s->ocm+OCMMAP(s->ocm)->appKvt));
  }

/*
** ++
**
** This function looks up and returns a socket handle.
**
** --
*/

static Socket* _lookupSocket(SAS_Name name, SAS_Session s)
  {
  uint32_t offset;
  offset = (uint32_t)KvtLookup(KvtHash(name),(KvTable)(s->ocm+OCMMAP(s->ocm)->socketKvt));
  return offset ? (Socket*)(s->axi+offset) : NULL;
  }
