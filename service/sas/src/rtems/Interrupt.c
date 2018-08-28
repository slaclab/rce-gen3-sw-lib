/*
** ++
**  Package: SAS
**	
**
**  Abstract: SAS RTEMS interrupt handling
**
**  Implementation of RTEMS interrupts. See the corresponding header (".h") file for a
**  more complete description of the functions implemented below.      
**
**  Author:
**      S. Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	000 - June 26, 2014
**
**  Revision History:
**	None.
**
** --
*/

#include <rtems.h>
#include <rtems/irq-extension.h>
#include <rtems/rtems/sem.h>

#include "sas/Sas.h"

#include "../Map.h"
#include "../Interrupt.h"
#include "../Transaction.h"

//#define DEBUG

#ifdef DEBUG
#define debug_printv dbg_printv
#include "debug/print.h"
#else
#define debug_printv(...)
#endif

/* This structure is used for disabling interrupt sources */

static InterruptCtl   *_intrCtl;

/*
 * This structure is used for looking up the task ID
 * assigned to an interrupt source 
 */
 
static InterruptTable *_intrTable;

/*
 * RTEMS requires a character string name
 * when registering interrupt handlers.
 */

static const char *IntrNames[IRQ_COUNT] = {
  "ppiIntr0","ppiIntr1","ppiIntr2","ppiIntr3",
  "ppiIntr4","ppiIntr5","ppiIntr6","ppiIntr7",
  "ppiIntr8","ppiIntr9","ppiIntrA","ppiIntrB",
  "ppiIntrC","ppiIntrD","ppiIntrE","ppiIntrF"};

/* These are the IRQ values for each interrupt */

static const unsigned IntrIrq[IRQ_COUNT] = {
  INTR_0,
  INTR_1,
  INTR_2,
  INTR_3,
  INTR_4,
  INTR_5,
  INTR_6,
  INTR_7,
  INTR_8,
  INTR_9,
  INTR_A,
  INTR_B,
  INTR_C,
  INTR_D,
  INTR_E,
  INTR_F  
};

/*
 * This structure is used for counting
 * metrics interrupts 
 */
 
static uint32_t *_metricsTable;

/*
 * This structure is used for counting
 * transfer faults 
 */


static FaultTable *_faultTable;

/*
 * This address is used for reading
 * transfer faults
 */

static volatile uint32_t *_transferFault;

/*
** ++
**
** This routine is the interrupt handler, which is executed
** in interrupt context.
**
** It disables the active interrupt sources and releases 
** the semaphore associated with each interrupt source.
**
** --
*/

void IntrHandleMbx(void *data)
  {
  uint32_t group = (uint32_t)data;
  uint32_t disabled;
  rtems_status_code status;
  
  debug_printv("INTERRUPT!!!! IntrHandler group 0x%x\n",(int)data);
  debug_printv("    enab  0x%x-0x%x\n",
               (int)&_intrCtl[group].enable,(int)_intrCtl[group].enable);  
  disabled = _intrCtl[group].disable;
  debug_printv("    disab 0x%x-0x%x\n",
               (int)&_intrCtl[group].disable,(int)disabled);
    
  while(disabled)
    {    
    /* wake each task with an active source, if configured to do so */
    uint32_t src = __builtin_ffsl(disabled) - 1;
    debug_printv("    FFS found src 0x%x\n",(int)src);
    BITCLR(src, disabled);
    if(_intrTable->sync[group][src])
      {
      debug_printv("    release task sem 0x%x group %d src %d\n",
                   (int)_intrTable->sync[group][src],(int)group,(int)src);
      rtems_semaphore_release(_intrTable->sync[group][src]);
      }
    }
  }

/*
** ++
**
** This routine is the metrics interrupt handler, which is executed
** in interrupt context.
**
** It disables the active interrupt sources and
** executes the metrics callback handler associated with the
** interrupt group.
**
** --
*/

void IntrHandleMetrics(void *data)
  {
  uint32_t group = (uint32_t)data;
  uint32_t disabled,enable;
  
  debug_printv("METRICS INTERRUPT!!!! IntrHandler group 0x%x\n",(int)data);
  debug_printv("    enab  0x%x-0x%x\n",
               (int)&_intrCtl[group].enable,(int)_intrCtl[group].enable);  
  disabled = _intrCtl[group].disable;
  debug_printv("    disab 0x%x-0x%x\n",
               (int)&_intrCtl[group].disable,(int)disabled);
  
  enable = disabled;
  while(disabled)
    {
    int src = __builtin_ffs(disabled)-1;
    _metricsTable[src]++;    
    BITCLR(src, disabled);
    }
  }

/*
** ++
**
** This routine is the faults interrupt handler, which is executed
** in interrupt context.
**
** It disables the active interrupt sources and
** executes the metrics callback handler associated with the
** interrupt group.
**
** --
*/

void IntrHandleFaults(void *data)
  {
  FaultTable *table;
  uint32_t err;
  int engine;
  int socket;
  int syndrome;
  uint32_t group = (uint32_t)data;
  uint32_t disabled,enable;

  debug_printv("FAULTS INTERRUPT!!!! IntrHandler group 0x%x\n",(int)data);
  debug_printv("    enab  0x%x-0x%x\n",
               (int)&_intrCtl[group].enable,(int)_intrCtl[group].enable);  
  disabled = _intrCtl[group].disable;               
  debug_printv("    disab 0x%x-0x%x\n",
               (int)&_intrCtl[group].disable,(int)disabled);
    
  debug_printv("Read transfer fault fifo 0x%x\n ",_transferFault);
  
  /* read the transfer fault fifo */
  err = *_transferFault;
    
  if(SAS_DECODE(INPUT_EMPTY,err)) return;

  debug_printv("transfer fault error 0x%x\n",(int)err);
  debug_printv("  %s engine\n",SAS_DECODE(FAULT_ENGINE,err) ? "outbound":"inbound");
  debug_printv("  socket   %d\n",SAS_DECODE(FAULT_SOCKET,err));
  debug_printv("  read     %d\n",SAS_DECODE(FAULT_READ,err));
  debug_printv("  write    %d\n",SAS_DECODE(FAULT_WRITE,err));
  debug_printv("  frame    %d\n",SAS_DECODE(FAULT_TRANSFER,err));
  debug_printv("  overflow %d\n",SAS_DECODE(FAULT_OVERFLOW,err));

  engine = SAS_DECODE(FAULT_ENGINE,err);
  socket = SAS_DECODE(FAULT_SOCKET,err);

  table = &_faultTable[socket];

  /* shift to the syndrome bits */
  err >>= FAULT_READ_OFFSET;
  
  syndrome = __builtin_ffs(err);
  if(!syndrome) return;
    
  if(!engine)
    table->ib[syndrome-1]++;  
  else
    table->ob[syndrome-1]++;
  
   /* now re-enable the interrupt sources */
  _intrCtl[group].enable = disabled;
  }

/*
** ++
**
** This routine registers interrupt handlers with RTEMS.
**
** --
*/

SAS_Status IntrSetCb(int interrupt, void *handler)
  {
  rtems_status_code status;
  int irq;
  
  if(interrupt >= IRQ_COUNT)
	{
	debug_printv("IntrSetCb invalid interrupt %d\n",interrupt);
	return SAS_NO_IRQ;
	}
  
  irq = IntrIrq[interrupt];
  
 /*
  * Connect a device driver handler that will be called when an
  * interrupt for the device occurs, the device driver handler performs
  * the specific interrupt processing for the device
  */
  /* Install interrupt handler and enable the vector */
  status = rtems_interrupt_handler_install(irq,
            IntrNames[interrupt],RTEMS_INTERRUPT_UNIQUE,
            handler,(void *)interrupt);
  if(status != RTEMS_SUCCESSFUL)
    {
    debug_printv("IntrSetCb %s failure to install irq %d handler err:0x%x\n",
                 IntrNames[interrupt],(int)irq,status);
    return SAS_NO_IRQ;
    }
  else
    debug_printv("IntrSetCb success irq 0x%x\n",(int)irq);
    
  return SAS_SUCCESS;
  }

/*
** ++
**
** This routine is the entry point to configure interrupt handlers.
**
** --
*/

SAS_Status IntrRegister(void)
  {
  int status;
  int i;
  RundwnMap *map;
  
  map = (RundwnMap*)(SasAxi+RUNDWN_AXI_BASE);
  
  _intrCtl       = (InterruptCtl*)(SasAxi+INTR_GROUP_AXI_BASE);
  _intrTable     = &SasOcmMap->intrTable;
  _metricsTable  = &SasOcmMap->metricsTable[0];
  _faultTable    = (FaultTable *)&SasOcmMap->faultTable[0];
  _transferFault = &map->fault;
      
  for(i=0; i<IRQ_COUNT; i++)
    {
    if(i == SAS_IRQ_METRICS)
      status = IntrSetCb(i,IntrHandleMetrics);
    else if(i == SAS_IRQ_FAULTS)
      status = IntrSetCb(i,IntrHandleFaults);
    else    
      status = IntrSetCb(i,IntrHandleMbx);
    if(status != SAS_SUCCESS) break;
    }
  return status;
  }

/*
** ++
**
** This routine allocates am RTEMS semaphore.
**
** --
*/

SAS_Status IntrAllocSync(uint32_t *sync)
  {
  rtems_status_code ret;  
  /* Create a semaphore to notify the task of interrupts */
  ret = rtems_semaphore_create(rtems_build_name('P', 'P', 'I','S'), 0,
                              RTEMS_SIMPLE_BINARY_SEMAPHORE , 0,
                              sync);
  if (ret != RTEMS_SUCCESSFUL)
    return SAS_NO_IRQ;
  
  debug_printv("Created RTEMS semaphore 0x%x\n",(int)*sync);
  
  return SAS_SUCCESS;
  }

/*
** ++
**
** This routine frees an allocated RTEMS semaphore.
**
** --
*/

void IntrFreeSync(uint32_t *sync)
  {
  rtems_semaphore_delete(*sync);
  debug_printv("Deleted RTEMS semaphore 0x%x\n",(int)*sync);
  *sync = 0;
  }

/*
** ++
**
** This function waits on an RTEMS semaphore.
**
** --
*/

void IntrWaitSync(uint32_t *sync)
  {
  rtems_semaphore_obtain(*sync, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
  return;
  }
