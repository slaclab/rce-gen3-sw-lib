// -*-Mode: C;-*-
/**
@file
@brief Define macros for (data) cache line size and alignment for the ARM.

Defines CACHE_LINE_SIZE and CACHE_LINE_ALIGNED as described in system/cacheline.h.

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
2014/05/16

@par Last commit:
\$Date: 2014-05-30 14:01:16 -0700 (Fri, 30 May 2014) $ by \$Author: tether $.

@par Credits:
SLAC
*/
#if !defined(SYSTEM_CONFIGURATION_ARM_CACHELINE_H)
#define      SYSTEM_CONFIGURATION_ARM_CACHELINE_H

#define CACHE_LINE_SIZE 32

#define CACHE_LINE_ALIGNED __attribute__((aligned(32)))

#endif
