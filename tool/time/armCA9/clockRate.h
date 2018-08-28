// -*-Mode: C;-*-
/**
@file
@brief Return various Zynq clock frequencies in Hz.
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
#if !defined(TOOL_TIME_CLOCKRATE_H)
#define      TOOL_TIME_CLOCKRATE_H

#ifdef __cplusplus
extern "C" {
#endif

/** @brief The names of the clocks known to time_getZynqClockRate(). */
typedef enum {
  /** @brief The fastest clock. Used for CPUs, SCU.*/
  ZYNQ_CLOCK_CPU_6X4X,

  /** @brief 1/2 of 6X4X. Used for OCM.*/
  ZYNQ_CLOCK_CPU_3X2X,

  /** @brief 1/3 or 1/2 of 6X4X. Uses for AXI. */
  ZYNQ_CLOCK_CPU_2X,

  /** @brief 1/2 of 2X. Used as the internal clock for most periperals, e.g., timer-counters. */
  ZYNQ_CLOCK_CPU_1X_REF,

  /** @brief The reference clock for both UARTs. */
  ZYNQ_CLOCK_UART_REF
} ZynqClockName;

/** @brief Return the rate of the indicated Zynq clock, in Hz. */
unsigned time_getZynqClockRate(ZynqClockName clock);

#ifdef __cplusplus
}
#endif

#endif
