// -*-Mode: C;-*-
/**
@file
@brief Implement the functions declared in clockRate.h.
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
2014/06/18

@par Last commit:
\$Date: 2015-07-14 10:55:53 -0700 (Tue, 14 Jul 2015) $ by \$Author: smaldona $.

@par Credits:
SLAC
*/

#include <inttypes.h>
#include <stdio.h>

#include "system/gen/psclock.h"

#include "memory/mem.h"

#include "time/cpu/clockRate.h"

static uint32_t rate[] = {
  [ZYNQ_CLOCK_CPU_6X4X]   = 0,
  [ZYNQ_CLOCK_CPU_3X2X]   = 0,
  [ZYNQ_CLOCK_CPU_2X]     = 0,
  [ZYNQ_CLOCK_CPU_1X_REF] = 0,
  [ZYNQ_CLOCK_UART_REF]   = 0
};

static inline uint32_t roundToHz(uint64_t x) {
  uint32_t y = (uint32_t)((x + 128U) >> 8U);
  return y;
}

unsigned time_getZynqClockRate(ZynqClockName clkname) {
  if (rate[ZYNQ_CLOCK_CPU_6X4X] == 0) {
    const volatile uint32_t* const slcr = (const volatile uint32_t*)mem_mapSlcr();
    uint32_t const armPllCtrl  = slcr[0x100 >> 2];
    uint32_t const armClkCtrl  = slcr[0x120 >> 2];
    uint32_t const clk621True  = slcr[0x1c4 >> 2];
    uint32_t const ioPllCtrl   = slcr[0x108 >> 2];
    uint32_t const ddrPllCtrl  = slcr[0x104 >> 2];
    uint32_t const uartClkCtrl = slcr[0x154 >> 2];

    { // ----- CPU clocks -----
      // Extract the rate multiplier used by the ARM PLL.
      uint32_t const multiplier = (armPllCtrl >> 12) & 0x7f;
      // Extract the rate divisor used by the divider between the PLL and the clock ratio generator.
      uint32_t const divisor = (armClkCtrl >> 8) & 0x3f;
      // Calculate the ratio of CPU_6x4x to CPU_1x_REF.
      uint32_t const sixOrFour = clk621True ? 6 : 4;
      // Calculate rate into ratio generator in units of Hz/256.
      uint64_t const inputRate = (((uint64_t)ZYNQ_PSCLOCK_HZ << 8U) * multiplier) / divisor;
      // Set the other rate array values in Hz. We musn't retain the fractional bits since
      // we only know our input, PS_CLK, down to 1 Hz. Although the clock ratio generator
      // produces exact results in its divisions (it only needs to count input clocks), the
      // resulting rates may not be integral multiples of 1 Hz. Therefore the ratios of
      // these rounded rates may not be exactly those of the real rates.
      rate[ZYNQ_CLOCK_CPU_6X4X]   = roundToHz(inputRate);
      rate[ZYNQ_CLOCK_CPU_3X2X]   = roundToHz(inputRate >> 1U);
      rate[ZYNQ_CLOCK_CPU_2X]     = roundToHz((inputRate << 1U) / sixOrFour);
      rate[ZYNQ_CLOCK_CPU_1X_REF] = roundToHz(inputRate / sixOrFour);
    }

    { // ----- UART clock -----
      uint32_t const pllChoice  = (uartClkCtrl >> 4) & 0x03U;
      uint32_t const divisor    = (uartClkCtrl >> 8) & 0x3fU;
      uint32_t       pllCtrl    = 0;
      if (pllChoice == 3)      pllCtrl = ddrPllCtrl;
      else if (pllChoice == 2) pllCtrl = armPllCtrl;
      else                     pllCtrl = ioPllCtrl;
      uint32_t const multiplier = (pllCtrl >> 12) &  0x7fU;
      uint64_t const pllRate    = (((uint64_t)ZYNQ_PSCLOCK_HZ << 8U) * multiplier) / divisor;
      rate[ZYNQ_CLOCK_UART_REF] = roundToHz(pllRate);
    }
    
  }
  return rate[clkname];
}
