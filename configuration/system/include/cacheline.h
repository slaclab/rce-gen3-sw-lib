// -*-Mode: C;-*-
/**
@file
@brief Define macros for (data) cache line size and alignment.

This header calls upon CPU-family-specific header files in order to define two macros.

CACHE_LINE_SIZE is just that, in bytes.

CACHE_LINE_ALIGNED expands to "__attribute__((aligned(N)))" where N is
the same value used for CACHE_LINE_SIZE. Alas, GCC won't allow us to
use "__attribute__((aligned(CACHE_LINE_SIZE))" in our code; it claims
that the value given to aligned() is non-constant.

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
#if !defined(SYSTEM_CONFIGURATION_CACHELINE_H)
#define      SYSTEM_CONFIGURATION_CACHELINE_H

#include "system/family/cacheline.h"

#endif
