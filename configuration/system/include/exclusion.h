// -*-Mode: C;-*-
/**
@file

@brief Define macros for the hardware mutual-exclusion
granularity and the corresponding alignment.

This header calls upon CPU-family-specific header files in order to
define two macros.

EXCLUSION_GRANULARITY is the number of bytes reserved by the
instructions that lock or flag or reserve memory locations. We assume
that an int or unsigned int is being locked so on our supported
platforms this number is >= 4.

EXCLUSION_ALIGNED expands to "__attribute__((aligned(N)))" where N
is the same value used for EXCLUSION_GRANULARITY. Alas, GCC won't
allow us to use "__attribute__((aligned(EXCLUSION_GRANULARITY))" in our
code; it claims that the value given to aligned() is
non-constant.

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
#if !defined(SYSTEM_CONFIGURATION_EXCLUSION_H)
#define      SYSTEM_CONFIGURATION_EXCLUSION_H

#include "system/family/exclusion.h"

#endif
