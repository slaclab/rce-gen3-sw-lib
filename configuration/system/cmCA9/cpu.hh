/*!
*
* @brief   Constants describing the CPU
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    July 29, 2013 -- Created
*
* $Revision: 3167 $
*
* @verbatim:
*                               Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef CONFIGURATION_SYSTEM_RCECA9_CPU_HH
#define CONFIGURATION_SYSTEM_RCECA9_CPU_HH

#define GBL_TIMER_LOWER_ADDR 0xF8F00200
#define GBL_TIMER_UPPER_ADDR 0xF8F00204

#define CPU_CLOCK_FREQ_NUM   2000
#define CPU_CLOCK_FREQ_DEN   3
#define CPU_CLOCK_FREQUENCY  666666687UL  /* Hz */

#endif /* CONFIGURATION_SYSTEM_CPU_HH */
