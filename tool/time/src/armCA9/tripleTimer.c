// -*-Mode: C;-*-
/**
@file
@brief Operations on counter 0 of TTC0 on a Zynq.
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
\$Date: 2014-06-13 14:40:28 -0700 (Fri, 13 Jun 2014) $ by \$Author: tether $.

@par Credits:
SLAC
*/

// Xilinx TTC header.
#include "xttcps.h"

#include "time/cpu/tripleTimer.h"

// We'll use the first counter in TTC0
static const u16 ttcId =  XPAR_XTTCPS_0_DEVICE_ID;

// The description of the timer being used.
static XTtcPs timer;

void time_initTtc(unsigned log2Prescale, int clock) {
  if (log2Prescale > 16) log2Prescale = 16;

  // Look up the default configuration which contains the base address.
  XTtcPs_Config * const config = XTtcPs_LookupConfig(ttcId);
  int xstat;
  xstat = XTtcPs_CfgInitialize(&timer, config, config->BaseAddress);
  if (xstat == XST_DEVICE_IS_STARTED) {
    // We have just enough of the init. done to be able to use the stop operation.
    XTtcPs_Stop(&timer);
    // This time it will work.
    XTtcPs_CfgInitialize(&timer, config, config->BaseAddress);
  }

  // Set the mode of operation.
  int options = XTTCPS_OPTION_WAVE_DISABLE;
  if (clock != TTC_CLOCK_INTERNAL) {
    options |= XTTCPS_OPTION_EXTERNAL_CLK;
  }
  XTtcPs_SetOptions(&timer, options);

  // Set prescaling if requested.
  if (log2Prescale) XTtcPs_SetPrescaler(&timer, log2Prescale - 1);
}


void time_startTtc(int reset) {
  if (reset != TTC_DONT_RESET) XTtcPs_ResetCounterValue(&timer);
  XTtcPs_Start(&timer);
}


unsigned time_getTtcCounter(int stop) {
  if (stop != TTC_DONT_STOP) XTtcPs_Stop(&timer);
  return XTtcPs_GetCounterValue(&timer);
}

void time_stopTtc(void) {
  XTtcPs_Stop(&timer);
}


