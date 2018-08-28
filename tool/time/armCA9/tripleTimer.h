// -*-Mode: C;-*-
/**
@file
@brief Interface to one of the Zynq triple-timer counters.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
DAT

@author
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2014/06/10

@par Last commit:
\$Date: 2015-07-14 10:55:53 -0700 (Tue, 14 Jul 2015) $ by \$Author: smaldona $.

@par Credits:
SLAC
*/
#if !defined(TOOL_TIME_TRIPLETIMER_H)
#define      TOOL_TIME_TRIPLETIMER_H

#include "time/cpu/clockRate.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
  TTC_CLOCK_INTERNAL = 0,
  TTC_CLOCK_EXTERNAL = 1,
  TTC_DONT_STOP      = 0,
  TTC_STOP           = 1,
  TTC_DONT_RESET     = 0,
  TTC_RESET          = 1
};


/** @brief Set up counter zero of TTC0.

    @param[in] log2Prescale The log of the prescale to use on the input clock.
    Min 0, max 16.

    @param[in] clock Selects the source of the counter
    clock. TTC_CLOCK_INTERNAL selects CPU_1x_REF, anything else selects an
    external clock. On a Zedboard this is EMIOTTC0CLKI0 since the MIO
    pins that might have been used for this are being used for
    Ethernet and USB.

    Post-conditions:
    - The counter is stopped and has a value of zero.
    - The counter will count up from zero.
    - All counter interrupts are disabled.
    - The counter is in overflow mode, meaning that it will roll over
      to zero again only after reaching its maximum value of 0xFFFF.
    - Prescaling is enabled if log2Prescale was not zero.
    - Waveform output is disabled.
    - The internal or external clock is selected.
    - The external clock, if selected, is set up using the SLCR registers
      MIO_PIN19, 31 and 43.
*/
void time_initTtc(unsigned log2Prescale, int clock);

/** @brief Start the counter, resetting it first if the argument is nonzero. */
void time_startTtc(int);

/** @brief Return the counter value, stopping the counter first if the argument is nonzero. */
unsigned time_getTtcCounter(int);

/** @brief Stop the counter, preserving its contents. */
void time_stopTtc(void);

/** @brief Return the rate of the internal clock signal connected the TTC. */
static inline unsigned time_getTtcInternalClockRate(void) {
  return time_getZynqClockRate(ZYNQ_CLOCK_CPU_1X_REF);
}

#ifdef __cplusplus
}
#endif

#endif
