// -*-Mode: C++;-*-
/*!
* @file
* @brief   Function for suspending a thread for a fixed amount of time.
* @verbatim
*                               Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

#include "concurrency/Sleep.hh"

#include <rtems.h>

void tool::concurrency::usleep(unsigned us)
{

  unsigned usec_per_tick =
    rtems_configuration_get_microseconds_per_tick();
  unsigned ticks =
    static_cast<unsigned>(static_cast<float>(us)/usec_per_tick);
  rtems_task_wake_after(ticks);
}


void tool::concurrency::sleep(unsigned ms)
{
  uint32_t systemTicksPerSec;
  rtems_clock_get(RTEMS_CLOCK_GET_TICKS_PER_SECOND, &systemTicksPerSec);
  const uint32_t ticks = (uint32_t(systemTicksPerSec) *  ms + 999L) / 1000L;
  rtems_task_wake_after(ticks);
}
