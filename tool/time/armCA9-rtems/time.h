// -*-Mode: C;-*-
/*!
*
* @brief   Functions for handling times
*
* @author  Anonymous -- REG/DRD - (someone@slac.stanford.edu)
*
* @date    March 29, 2011 -- Created
*
* $Revision: 3411 $
*
* @verbatim
*                               Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef TOOL_TIME_TIME_H
#define TOOL_TIME_TIME_H
#include <inttypes.h>
#include <time.h>
#include <rtems.h>
#include <rtems/counter.h>
#include <rtems/timespec.h>

#include "system/cpu.hh"

static inline unsigned TOOL_TIME_ticks(void)
{
  volatile unsigned *l = (volatile unsigned*)GBL_TIMER_LOWER_ADDR;
  return *l;
}

static inline void TOOL_TIME_ticks2(unsigned* up, unsigned* low)
{
  volatile unsigned *l = (volatile unsigned*)GBL_TIMER_LOWER_ADDR;
  volatile unsigned *u = (volatile unsigned*)GBL_TIMER_UPPER_ADDR;
  uint32_t up_old;
  do {
    up_old = *u;
    *low   = *l;
    *up    = *u;
  } while (up_old != *up);
}

static inline unsigned long long TOOL_TIME_lticks(void)
{
  volatile unsigned *l = (volatile unsigned*)GBL_TIMER_LOWER_ADDR;
  volatile unsigned *u = (volatile unsigned*)GBL_TIMER_UPPER_ADDR;
  uint32_t up_old, up, low;
  unsigned long long ll;
  do {
    up_old = *u;
    low    = *l;
    up     = *u;
  } while (up_old != up);
  ll   = up;
  ll <<= 32;
  ll |= low;
  return ll;
}

/* Do this as a macro so as not to impose a type on dT (32 vs 64 bit number) */
#define TOOL_TIME_t2uS(dT) (rtems_counter_ticks_to_nanoseconds(dT)/1000)

static inline struct timespec TOOL_TIME_t2s(unsigned long long dT)
{
  struct timespec t;

  rtems_timespec_from_ticks(dT,&t);

  return t;
}

#endif // TOOL_TIME_TIME_HH
