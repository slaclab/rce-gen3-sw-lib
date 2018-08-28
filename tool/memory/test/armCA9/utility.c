// -*-Mode: C;-*-
/**
@file
@brief Implement OS-independent auxiliary functions needed for tests.
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
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/memory/test/armCA9/utility.c $

@par Credits:
SLAC
*/



#include "utility.h"

/* Do a read probe just to get the PAR value for debugging. */
unsigned mem_getPar(uint32_t virt) {
  register unsigned parValue; /* Value read from the PAR. */
  asm volatile
    (
     "mcr   p15,0,%[virt],c7,c8,0  \n\t"   /* ATS1CPR */
     "isb                          \n\t"   /* Wait for result */
     "mrc   p15,0,%[par],c7,c4,0   \n\t"   /* Read PAR */
     : [par]  "=r"(parValue)
     : [virt] "r"  (virt)
     :
     );
  return parValue;
}
