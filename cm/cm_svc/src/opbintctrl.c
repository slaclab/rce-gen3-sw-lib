/*  opbintctrl.c
 *
 *  This file contains definitions and declarations for the
 *  Xilinx Off Processor Bus (OPB) Interrupt Controller
 *
 *  Author: Keith Robertson <kjrobert@alumni.uwaterloo.ca>
 *  COPYRIGHT (c) 2005 Linn Products Ltd, Scotland.
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 */


#include "cm_svc/opbintctrl.h"
#include <rtems.h>
//#include <rtems/libio.h>
#include <rtems/bspIo.h>
#include <bsp/vectors.h>
//#include <bsp/ppc_exc_bspsupp.h>

#include <stdlib.h>
#include <assert.h>


/* Maximum number of IRQs.  Defined in vhdl model */
#define OPB_INTC_IRQ_MAX	16

/* Interrupt Status Register */
#define OPB_INTC_ISR		0x0
/* Interrupt Pending Register (ISR && IER) */
#define OPB_INTC_IPR		0x4
/* Interrupt Enable Register */
#define OPB_INTC_IER		0x8
/* Interrupt Acknowledge Register */
#define OPB_INTC_IAR		0xC
/* Set Interrupt Enable (same as read/mask/write to IER) */
#define OPB_INTC_SIE		0x10
/* Clear Interrupt Enable (same as read/mask/write to IER) */
#define OPB_INTC_CIE		0x14
/* Interrupt Vector Address (highest priority vector number from IPR) */
#define OPB_INTC_IVR		0x18
/* Master Enable Register */
#define OPB_INTC_MER		0x1C

/* Master Enable Register: Hardware Interrupt Enable */
#define OPB_INTC_MER_HIE	0x2

/* Master Enable Register: Master IRQ Enable */
#define OPB_INTC_MER_ME		0x1

#define gOpbIntcBase            0xD1000FC0
/*
 *  ISR vector table to dispatch external interrupts
 */
rtems_isr_entry opb_intc_vector_table[OPB_INTC_IRQ_MAX];

/*
 * Acknowledge/Clear a specific vector number
 */
RTEMS_INLINE_ROUTINE void
awk_vector(unsigned vector)
{
  *(volatile unsigned*)(gOpbIntcBase+OPB_INTC_IAR) = 1 << vector;
}

/*
 * Enable a specific vector number
 * vector is a number counting from 0
 */
RTEMS_INLINE_ROUTINE void
enable_vector(unsigned vector)
{
  *(volatile unsigned*)(gOpbIntcBase+OPB_INTC_SIE) = 1 << vector;
}

/*
 * Disable a specific vector number
 * vector is a number counting from 0
 */
RTEMS_INLINE_ROUTINE void
disable_vector(unsigned vector)
{
  *(volatile unsigned*)(gOpbIntcBase+OPB_INTC_CIE) = 1 << vector;
}

/*
 * Set IER state.  Used to (dis)enable a mask of vectors.
 * If you only have to do one, use enable/disable_vector.
 */
RTEMS_INLINE_ROUTINE void
set_ier(unsigned mask)
{
  *(volatile unsigned*)(gOpbIntcBase+OPB_INTC_IER) = mask;
}

/*
 * Used to awknowledge a mask of vectors.
 * If you only have to do one, use awk_vector.
 */
RTEMS_INLINE_ROUTINE void
awk_mask(unsigned mask)
{
  *(volatile unsigned*)(gOpbIntcBase+OPB_INTC_IAR) = mask;
}

/*
 * Get Highest Priority Pending Vector (aka read IVR)
 */
RTEMS_INLINE_ROUTINE unsigned
get_vector()
{
  volatile unsigned vector = *(volatile unsigned*)(gOpbIntcBase+OPB_INTC_IVR);
  return vector;
}

/*
 * Enable Hardware Interrupt Enable
 * This turns off support for software settable interrupts.
 */
RTEMS_INLINE_ROUTINE void
enable_hie()
{
  volatile unsigned value = *(volatile unsigned*)(gOpbIntcBase+OPB_INTC_MER);
  value |= OPB_INTC_MER_HIE;
  *(volatile unsigned*)(gOpbIntcBase+OPB_INTC_MER) = value;
}

/*
 * Master IRQ (Dis)Enable
 */
RTEMS_INLINE_ROUTINE void
set_master_enable(unsigned value)
{
  volatile unsigned v = *(volatile unsigned*)(gOpbIntcBase+OPB_INTC_MER);
  v |= value ? OPB_INTC_MER_ME : 0;
  *(volatile unsigned*)(gOpbIntcBase+OPB_INTC_MER) = v;
}

/*
 *  ISR Handler: this is called from the primary exception dispatcher
 */

int
opb_intc_isr(BSP_Exception_frame *frame, unsigned vector)
{
  unsigned opb_vector;
  rtems_isr_entry handler;

  opb_vector = get_vector();
  if(opb_vector > OPB_INTC_IRQ_MAX) {
    printk("opb_intc_isr: Invalid vector number, %d.  Greater than max: %d\n", opb_vector, OPB_INTC_IRQ_MAX);
    /*         assert(0); */
    return 0;
  }
  handler = opb_intc_vector_table[opb_vector];
  if(handler) {
    (handler)(opb_vector);
  } else {
    printk("opb_intc_isr: Vector: %d called but no installed handler\n", opb_vector);
    assert(0);
  }
  awk_vector(opb_vector);
  return 0;
}

/*
 * install a user vector for one of the external interrupt sources
 * vector is the number of the vector counting from 0.
 */
rtems_status_code
opb_intc_set_vector(rtems_isr_entry new_handler, unsigned vector,
		    rtems_isr_entry *old_handler)
{
  /*
   *  We put the actual user ISR address in 'opb_intc_vector_table'.  This will
   *  be used by the _opb_intc_isr so the user gets control.
   */

  if(vector < OPB_INTC_IRQ_MAX) {
    *old_handler = opb_intc_vector_table[vector];
    if(new_handler) {
      opb_intc_vector_table[vector] = new_handler;
      enable_vector(vector);
    } else {
      disable_vector(vector);
      opb_intc_vector_table[vector] = 0;
    }
    return RTEMS_SUCCESSFUL;
  }
  return RTEMS_INVALID_NUMBER;
}

/*
 * activate the interrupt controller
 */
rtems_status_code opb_intc_init(void)
{
  /*
   * connect all exception vectors needed
   */
  ppc_exc_set_handler(ASM_EXT_VECTOR, opb_intc_isr);

  /* Clear any currently set interrupts */
  awk_mask(0xFFFFFFFF);

  /* mask off all interrupts */
  set_ier(0x0);

  /* Turn on normal hardware operation of interrupt controller */
  enable_hie();
  /* Enable master interrupt switch for the interrupt controller */
  set_master_enable(1);

  /* install exit handler to close opb_intc when program atexit called */
  /* atexit(opb_intc_exit);*/

  return RTEMS_SUCCESSFUL;
}
