// -*-Mode: C;-*-
/**
@file
@brief Define macros for mutual exclusion granularity and alignment for ARM.

Defines EXCLUSION_GRANULARITY and EXCLUSION_ALIGNED as described in system/exclusion.h.

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
#if !defined(SYSTEM_CONFIGURATION_ARM_EXCLUSION_H)
#define      SYSTEM_CONFIGURATION_ARM_EXCLUSION_H

#include "system/cacheline.h"

// On modern ARM processors, at least in the "A" family such as
// Cortex-An, LDREX reserves the entire cache line containing the word
// referenced. See the Cache Type Register.

#define EXCLUSION_GRANULARITY CACHE_LINE_SIZE

#define EXCLUSION_ALIGNED CACHE_LINE_ALIGNED

#endif
