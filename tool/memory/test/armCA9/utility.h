// -*-Mode: C;-*-
/**
@file
@brief Helpers for "mem" test code.
@verbatim
                               Copyright 2013
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
2013/06/21

@par Last commit:
\$Date: 2014-04-07 21:30:01 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3125 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/memory/test/armCA9/utility.h $

@par Credits:
SLAC
*/
#if !defined(TEST_MEMORY_UTILITY_H)
#define      TEST_MEMORY_UTILITY_H

#include <stdint.h>

/* @brief Perform ATS1CPR and read the Physical Address Register.*/
unsigned mem_getPar(uint32_t virt);
#endif
