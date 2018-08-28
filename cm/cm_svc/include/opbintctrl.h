/*  opbintctrl.h
 *
 *  This file contains definitions and declarations for the 
 *  Xilinx Off Processor Bus (OPB) Interrupt Controller
 *
 *  Author: Keith Robertson <kjrobert@alumni.uwaterloo.ca>
 *  COPYRIGHT (c) 2005 by Linn Products Ltd, Scotland
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 */

#ifndef _INCLUDE_OPBINTCTRL_H
#define _INCLUDE_OPBINTCTRL_H

#include <rtems.h>
#include <rtems/system.h>
#include <rtems/score/isr.h>
#ifdef __cplusplus
extern "C" {
#endif

/*
 * install a user vector for one of the external interrupt sources 
 */ 
rtems_status_code opb_intc_set_vector(rtems_isr_entry new_handler,
	unsigned vector, rtems_isr_entry *old_handler);
/*
 * activate the interrupt controller 
 */
rtems_status_code opb_intc_init();

#ifdef __cplusplus
}
#endif

#endif /*  _INCLUDE_OPBINTCTRL_H */
