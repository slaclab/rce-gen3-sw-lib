// -*-Mode: C;-*-
/**
@file
@brief Define the constant ZYNQ_PSCLOCK_HZ, the Zynq master clock fequency.

All the Zynq platforms currently in use by the DAT group use the same
frequency for PS_CLK, the clock signal from which all other clocks are
derived.

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

@par Credits:
SLAC
*/
#if !defined(CONFIGURATION_SYSTEM_PSCLOCK_H)
#define      CONFIGURATION_SYSTEM_PSCLOCK_H

enum {ZYNQ_PSCLOCK_HZ = 33333333};

#endif
