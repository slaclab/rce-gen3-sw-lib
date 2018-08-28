// -*-Mode: C;-*-
/**
@file
@brief This is a PRIVATE interface to the Socket Abstraction Services (SAS) facility.  
This file provides private interrupt support.

@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
SAS

@author
S. Maldonado, SLAC (smaldona@slac.stanford.edu)

@par Date created:
2014/06/26

@par Last commit:
\$Date: $ by \$Author: $.

@par Revision number:
\$Revision: $

@par Credits:
SLAC
*/

#ifndef SAS_INTERRUPT_H
#define SAS_INTERRUPT_H

#include "xscugic_hw.h"  // Use the xilinx library for convenience macros

/* GIC interrupt control table index values */
#define INTR_ID_IDX        0 
#define INTR_MASK_IDX      1 
#define INTR_ENABLE_IDX    2 
#define INTR_DISABLE_IDX   3 
#define INTR_PEND_IDX      4 

/* general interrupt controller (GIC) control/status registers */
#define ICDISER1           0xF8F01104 // interrupt set-enable
#define ICDISER2           0xF8F01108 // interrupt set-enable

#define ICDICER1           0xF8F01184 // interrupt clear-enable
#define ICDICER2           0xF8F01188 // interrupt clear-enable

#define ICDISPR1           0xF8F01204 // interrupt set-pending
#define ICDISPR2           0xF8F01208 // interrupt set-pending

#define ICDICPR1           0xF8F01284 // interrupt clear-pending
#define ICDICPR2           0xF8F01288 // interrupt clear-pending

#define ICDABR1            0xF8F01304 // interrupt active
#define ICDABR2            0xF8F01308 // interrupt active

#define ICDICFR3           0xF8F01C0C // interrupt configuration
#define ICDICFR4           0xF8F01C10 // interrupt configuration
#define ICDICFR5           0xF8F01C14 // interrupt configuration

/* calculate an interrupt trigger configuration bitmask for a given interrupt */
#define INTR_CFG_OFFSET_CALC(irq,trigger) \
    (trigger << ((irq%16)*2))

/* calculate an interrupt set/disable bitmask for a given interrupt */
#define INTR_SET_OFFSET_CALC(irq) \
	(1 << (irq % 32))

/* count of interrupt IRQ lines */
#define IRQ_COUNT          16    

/* interrupt lines */
#define INTR_0             61
#define INTR_1             62
#define INTR_2             63
#define INTR_3             64
#define INTR_4             65
#define INTR_5             66
#define INTR_6             67
#define INTR_7             68
#define INTR_8             84
#define INTR_9             85
#define INTR_A             86
#define INTR_B             87
#define INTR_C             88
#define INTR_D             89
#define INTR_E             90
#define INTR_F             91

/* interrupt control access macros */
#define INTR_0_ENABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_ENABLE_SET_OFFSET,INTR_0)
#define INTR_1_ENABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_ENABLE_SET_OFFSET,INTR_1)
#define INTR_2_ENABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_ENABLE_SET_OFFSET,INTR_2)
#define INTR_3_ENABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_ENABLE_SET_OFFSET,INTR_3)
#define INTR_4_ENABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_ENABLE_SET_OFFSET,INTR_4)
#define INTR_5_ENABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_ENABLE_SET_OFFSET,INTR_5)
#define INTR_6_ENABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_ENABLE_SET_OFFSET,INTR_6)
#define INTR_7_ENABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_ENABLE_SET_OFFSET,INTR_7)
#define INTR_8_ENABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_ENABLE_SET_OFFSET,INTR_8)
#define INTR_9_ENABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_ENABLE_SET_OFFSET,INTR_9)
#define INTR_A_ENABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_ENABLE_SET_OFFSET,INTR_A)
#define INTR_B_ENABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_ENABLE_SET_OFFSET,INTR_B)
#define INTR_C_ENABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_ENABLE_SET_OFFSET,INTR_C)
#define INTR_D_ENABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_ENABLE_SET_OFFSET,INTR_D)
#define INTR_E_ENABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_ENABLE_SET_OFFSET,INTR_E)
#define INTR_F_ENABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_ENABLE_SET_OFFSET,INTR_F)

#define INTR_0_DISABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_DISABLE_OFFSET,INTR_0)
#define INTR_1_DISABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_DISABLE_OFFSET,INTR_1)
#define INTR_2_DISABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_DISABLE_OFFSET,INTR_2)
#define INTR_3_DISABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_DISABLE_OFFSET,INTR_3)
#define INTR_4_DISABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_DISABLE_OFFSET,INTR_4)
#define INTR_5_DISABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_DISABLE_OFFSET,INTR_5)
#define INTR_6_DISABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_DISABLE_OFFSET,INTR_6)
#define INTR_7_DISABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_DISABLE_OFFSET,INTR_7)
#define INTR_8_DISABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_DISABLE_OFFSET,INTR_8)
#define INTR_9_DISABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_DISABLE_OFFSET,INTR_9)
#define INTR_A_DISABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_DISABLE_OFFSET,INTR_A)
#define INTR_B_DISABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_DISABLE_OFFSET,INTR_B)
#define INTR_C_DISABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_DISABLE_OFFSET,INTR_C)
#define INTR_D_DISABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_DISABLE_OFFSET,INTR_D)
#define INTR_E_DISABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_DISABLE_OFFSET,INTR_E)
#define INTR_F_DISABLE XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_DISABLE_OFFSET,INTR_F)

#define INTR_0_CLEAR XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_PENDING_CLR_OFFSET,INTR_0) 
#define INTR_1_CLEAR XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_PENDING_CLR_OFFSET,INTR_1) 
#define INTR_2_CLEAR XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_PENDING_CLR_OFFSET,INTR_2) 
#define INTR_3_CLEAR XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_PENDING_CLR_OFFSET,INTR_3) 
#define INTR_4_CLEAR XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_PENDING_CLR_OFFSET,INTR_4) 
#define INTR_5_CLEAR XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_PENDING_CLR_OFFSET,INTR_5) 
#define INTR_6_CLEAR XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_PENDING_CLR_OFFSET,INTR_6) 
#define INTR_7_CLEAR XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_PENDING_CLR_OFFSET,INTR_7) 
#define INTR_8_CLEAR XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_PENDING_CLR_OFFSET,INTR_8) 
#define INTR_9_CLEAR XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_PENDING_CLR_OFFSET,INTR_9) 
#define INTR_A_CLEAR XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_PENDING_CLR_OFFSET,INTR_A) 
#define INTR_B_CLEAR XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_PENDING_CLR_OFFSET,INTR_B) 
#define INTR_C_CLEAR XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_PENDING_CLR_OFFSET,INTR_C) 
#define INTR_D_CLEAR XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_PENDING_CLR_OFFSET,INTR_D) 
#define INTR_E_CLEAR XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_PENDING_CLR_OFFSET,INTR_E) 
#define INTR_F_CLEAR XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_PENDING_CLR_OFFSET,INTR_F) 

#define INTR_0_MASK INTR_SET_OFFSET_CALC(INTR_0)
#define INTR_1_MASK INTR_SET_OFFSET_CALC(INTR_1)
#define INTR_2_MASK INTR_SET_OFFSET_CALC(INTR_2)
#define INTR_3_MASK INTR_SET_OFFSET_CALC(INTR_3)
#define INTR_4_MASK INTR_SET_OFFSET_CALC(INTR_4)
#define INTR_5_MASK INTR_SET_OFFSET_CALC(INTR_5)
#define INTR_6_MASK INTR_SET_OFFSET_CALC(INTR_6)
#define INTR_7_MASK INTR_SET_OFFSET_CALC(INTR_7)
#define INTR_8_MASK INTR_SET_OFFSET_CALC(INTR_8)
#define INTR_9_MASK INTR_SET_OFFSET_CALC(INTR_9)
#define INTR_A_MASK INTR_SET_OFFSET_CALC(INTR_A)
#define INTR_B_MASK INTR_SET_OFFSET_CALC(INTR_B)
#define INTR_C_MASK INTR_SET_OFFSET_CALC(INTR_C)
#define INTR_D_MASK INTR_SET_OFFSET_CALC(INTR_D)
#define INTR_E_MASK INTR_SET_OFFSET_CALC(INTR_E)
#define INTR_F_MASK INTR_SET_OFFSET_CALC(INTR_F)

/**  @brief

This function performs the one time initialization of
platform dependent interrupt services.
  
*/

SAS_Status IntrInit(void);

/**  @brief

This function performs the one time initialization of
OS dependent interrupt services.
  
*/

SAS_Status IntrRegister(void);

/**  @brief

This function performs the one time initialization of
the OS dependent SVT.
  
*/

InterruptMap *InstallSvt(void);

/**  @brief

This function performs the allocation of an
OS dependent interrupt synchronization object.
  
*/

SAS_Status IntrAllocSync(uint32_t *sync);

/**  @brief

This function performs a free of an OS dependent 
interrupt synchronization object.
  
*/

void IntrFreeSync(uint32_t *sync);

/**  @brief

This function performs synchronization on the 
OS dependent interrupt object.
  
*/

void IntrWaitSync(uint32_t *sync);

#endif
