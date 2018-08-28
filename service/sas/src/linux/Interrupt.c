/*
** ++
**  Package: SAS
**	
**
**  Abstract: SAS LINUX interrupt handling
**
**  Implementation of LINUX interrupts. See the corresponding header (".h") file for a
**  more complete description of the functions implemented below.
**  A subset of this file is a kernel module.
**
**  Author:
**      S. Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	000 - June 30, 2014
**
**  Revision History:
**	None.
**
** --
*/

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/signal.h>

#define uint64_t unsigned long long
#define uint32_t unsigned int
#define uint16_t unsigned short
#define uint8_t  unsigned char
#else
#include <stdint.h>
#include <unistd.h>
#endif

#include "sas/Sas.h"

#include "../Map.h"
#include "../Interrupt.h"
#include "../Transaction.h"

//#define DEBUG

#ifdef DEBUG
#define debug_printk printk
#else
#define debug_printk(...)
#endif

#ifdef __KERNEL__

/* SEMTODO: these values must be defined in a configuration header */
#define AXI_BASE_ADDRESS    0x40000000
#define AXI_MAP_SIZE        0x00100000

#define OCM_BASE_ADDRESS    0xFFFC0000
#define OCM_MAP_SIZE        0x00040000

uint32_t SasAxi;
uint32_t SasOcm;
OcmMap  *SasOcmMap = 0;

static int debug = 0;
module_param(debug,int,0);
MODULE_PARM_DESC(debug,"Debug print enable");

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
 * interrupts 
 */
 
static uint32_t *_metricsTable;

/*
 * This structure is used for counting
 * transfer faults 
 */


static FaultTable *_faultTable;

/*
 * This structure is used for reading
 * transfer faults
 */

static volatile uint32_t *_transferFault;

/*
** ++
**
** This routine is the mailbox interrupt handler, which is executed
** in interrupt context. 
** It disables the active interrupt sources and releases 
** the synchronization object associated with each interrupt source.
**
** --
*/

int IntrHandleMbx(int irq, void *data)
  {
  int group = (int)data;
  int disabled;
  
  debug_printk(KERN_INFO "INTERRUPT!!!! IntrHandler group 0x%x\n",(int)data);
  debug_printk(KERN_INFO "    enab  0x%x-0x%x\n",
               (int)&_ctl[group].enable,(int)_ctl[group].enable);  
  disabled = _intrCtl[group].disable;
  debug_printk(KERN_INFO "    disab 0x%x-0x%x\n",
               (int)&_ctl[group].disable,(int)disabled);
    
  while(disabled)
    {    
    /* wake each task with an active source, if configured to do so */
    int src = __builtin_ffsl(disabled) - 1;
    debug_printk(KERN_INFO "    FFS found src 0x%x\n",(int)src);
    BITCLR(src, disabled);
    
    /* SEMTODO: replace with pthread task wake */
    /* clear the pthread ID value */
    if(_intrTable->sync[group][src])
      {
      debug_printk(KERN_INFO "    release sync 0x%x group %d src %d\n",
                   (int)_intrTable->sync[group][src],(int)group,(int)src);
      _intrTable->sync[group][src] = 0;
      }
    }
   return IRQ_HANDLED;
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

int IntrHandleMetrics(void *data)
  {
  uint32_t group;
  uint32_t disabled,enable;
  
  group = (uint32_t)data;
  
  debug_printk(KERN_INFO "METRICS INTERRUPT!!!! IntrHandler group 0x%x\n",(int)data);
  debug_printk("    enab  0x%x-0x%x\n",
               (int)&_intrCtl[group].enable,(int)_intrCtl[group].enable);  
  disabled = _intrCtl[group].disable;
  debug_printk(KERN_INFO "    disab 0x%x-0x%x\n",
               (int)&_intrCtl[group].disable,(int)disabled);
  
  enable = disabled;
  while(disabled)
    {
    int src = __builtin_ffs(disabled)-1;
    _metricsTable[src]++;    
    BITCLR(src, disabled);
    }
  return IRQ_HANDLED;
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

int IntrHandleFaults(void *data)
  {
  FaultTable *table;
  uint32_t err;
  int engine;
  int socket;
  int syndrome;
  uint32_t group;
  uint32_t disabled;

  group = (uint32_t)data;

  debug_printk(KERN_INFO "FAULTS INTERRUPT!!!! IntrHandler group 0x%x\n",(int)data);
  debug_printk("    enab  0x%x-0x%x\n",
               (int)&_intrCtl[group].enable,(int)_intrCtl[group].enable);  
  disabled = _intrCtl[group].disable;
  debug_printk(KERN_INFO "    disab 0x%x-0x%x\n",
               (int)&_intrCtl[group].disable,(int)disabled);
  
  debug_printk(KERN_INFO "Read transfer fault fifo 0x%x\n ",_transferFault);
  
  /* read the transfer fault fifo */
  err = *_transferFault;
    
  if(!SAS_DECODE(INPUT_EMPTY,err))
    {
    debug_printk(KERN_INFO "transfer fault error 0x%x\n",(int)err);
    debug_printk(KERN_INFO "  %s engine\n",SAS_DECODE(FAULT_ENGINE,err) ? "outbound":"inbound");
    debug_printk(KERN_INFO "  socket   %d\n",SAS_DECODE(FAULT_SOCKET,err));
    debug_printk(KERN_INFO "  read     %d\n",SAS_DECODE(FAULT_READ,err));
    debug_printk(KERN_INFO "  write    %d\n",SAS_DECODE(FAULT_WRITE,err));
    debug_printk(KERN_INFO "  frame    %d\n",SAS_DECODE(FAULT_TRANSFER,err));
    debug_printk(KERN_INFO "  overflow %d\n",SAS_DECODE(FAULT_OVERFLOW,err));

    engine = SAS_DECODE(FAULT_ENGINE,err);
    socket = SAS_DECODE(FAULT_SOCKET,err);

    table = &_faultTable[socket];

    /* shift to the syndrome bits */
    err >>= FAULT_READ_OFFSET;

    syndrome = __builtin_ffs(err);
    if(syndrome)
      {
      if(!engine)
        table->ib[syndrome-1]++;  
      else
        table->ob[syndrome-1]++;
      }
    }

   /* now re-enable the interrupt sources */
  _intrCtl[group].enable = disabled;
    
  return IRQ_HANDLED;
  }

/*
** ++
**
** This routine registers interrupt handlers.
**
** --
*/

SAS_Status IntrSetCb(int interrupt, void *handler)
  {    
  int irq;
  int err;
  
  if(interrupt >= IRQ_COUNT)
	{
	debug_printk(KERN_INFO "IntrSetCb invalid interrupt %d\n",interrupt);
	return SAS_NO_IRQ;
	}
    
  irq = IntrIrq[interrupt];
  
  /*
   * Connect a device driver handler that will be called when an
   * interrupt for the device occurs, the device driver handler performs
   * the specific interrupt processing for the device
   */
   err = request_irq(irq, (irq_handler_t)handler,
				     IRQF_TRIGGER_RISING, IntrNames[interrupt], (void *)interrupt);
                     
  if(err)
    debug_printk(KERN_INFO "IntrSetCb %s failure to install irq %d handler err:0x%x\n",
                 IntrNames[interrupt],irq,err);
  else if(debug)
    debug_printk(KERN_INFO "IntrSetCb %s connected irq %d\n",
                 IntrNames[interrupt],IntrIrq[interrupt]);

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
  
  if(debug)
    debug_printk(KERN_INFO "IntrRegister on cpu%d\n",smp_processor_id()); 
        
  map = (RundwnMap*)(SasAxi+RUNDWN_AXI_BASE);
  
  _intrCtl       = (InterruptCtl*)(SasAxi+INTR_GROUP_AXI_BASE);
  _intrTable     = &SasOcmMap->intrTable;
  _metricsTable  = &SasOcmMap->metricsTable[0];
  _faultTable    = &SasOcmMap->faultTable[0];
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
** This routine initializes the LINUX kernel module.
**
** --
*/

int __init init_module(void)
  {
  SAS_Status status;
    
  if(debug)
    debug_printk(KERN_INFO "Initialize PPI interrupt module cpu%d\n",
                 smp_processor_id()); 

  /* Map the AXI space */
  SasAxi = (uint32_t)ioremap(AXI_BASE_ADDRESS,AXI_MAP_SIZE);
  if ((int)SasAxi == -1)
    {
    debug_printk(KERN_INFO "Failure maping the AXI memory.\n");
    return -1; 
    }

  if(debug)
    debug_printk(KERN_INFO "AXI memory mapped to 0x%x\n",(int)axi);

  /* Map the OCM space */
  SasOcm = (uint32_t)ioremap(OCM_BASE_ADDRESS,OCM_MAP_SIZE);
  if ((int)SasOcm == -1)
    {
    debug_printk(KERN_INFO "Failure maping the OCM memory.\n");
    return -1; 
    }
  SasOcmMap = (OcmMap*)SasOcm;

  if(debug)
    debug_printk(KERN_INFO "OCM memory mapped to 0x%x\n",(int)ocm);
   
  status = IntrRegister();
  if(status != SAS_SUCCESS)
    debug_printk(KERN_INFO "Error registering interrupts 0x%x\n",(int)status);
  
  return 0;
  }

/*
** ++
**
** This function counts transfer faults
**
** --
*/

void _handleFault(void)
  {
  }

void cleanup_module(void)
  {
  if(debug)
    debug_printk(KERN_INFO "Cleanup interrupt module\n");
  }

MODULE_AUTHOR("S.Maldonado smaldona@slac.stanford.edu");
MODULE_DESCRIPTION("RCE Protocol Plugin Interrupt Driver");
MODULE_LICENSE("GPL");

#else /* __KERNEL__ */

/*
** ++
**
** This function allocates a synchronization object.
**
** --
*/

SAS_Status IntrAllocSync(uint32_t *sync)
  {
  /* SEMTODO: implement linux shared semaphore */
  return SAS_SUCCESS;
  }

/*
** ++
**
** This function frees a an allocated synchronization object.
**
** --
*/

void IntrFreeSync(uint32_t *sync)
  {
  /* SEMTODO: free linux shared semaphore */
  }

/*
** ++
**
** This function polls on a synchronization object.
**
** --
*/

void IntrWaitSync(uint32_t *sync)
  {
  *sync = 1;
  while(*sync) usleep(1);  
  return;
  }

#endif /* __KERNEL__ */
