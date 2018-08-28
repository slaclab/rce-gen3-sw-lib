// -*-Mode: C;-*-
/**
@file
@brief Test cache-line storing to main memory.
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
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/memory/test/armCA9/testFlush.c $

@par Credits:
SLAC
*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>


#include "memory/mem.h"

#include "utility.h"

void mem_testFlush(void) {
  printf("\n"
         "Flush test. Alter the contents of a small buffer,\n"
         "flush it to memory, then disable the caches and see\n"
         "whether all the data is in main memory. Repeat 100 times\n"
         "then summarize the results.\n\n"
         );
  static char buffer[250];
  static const char* const merlin[2] = {
    "Higitus Figitus zumbabazing\n"
    "I want your attention ev'rything!\n"
    "We're packing to leave come on let's go\n"
    "books are always the first you know.\n",

    "Hockety pockety wockety wack\n"
    "abracabra dabra nack\n"
    "Shrink in size very small\n"
    "we've got to save enough room for all\n"
    "Higitus Figitus migitus mum\n"
    "pres-ti-dig-i-ton-i-um!\n"
  };
  const uint32_t l2Base = mem_mapL2Controller();
  if (l2Base == 0) return;
  int fails = 0;
  int i;
  for (i = 0; i < 100; ++i) {
    memcpy(buffer, merlin[i % 2], 1 + strlen(merlin[i % 2]));
    mem_storeDataCacheRange((uint32_t)buffer, (uint32_t)(buffer + sizeof buffer), l2Base);
    mem_invalidateDataCacheRange((uint32_t)buffer, (uint32_t)(buffer + sizeof buffer), l2Base);
    // If the flush failed then the NUL terminator may not have
    // been stored, so be careful.
    if (0 != strncmp(merlin[i % 2], buffer, sizeof buffer)) ++fails;
  }
  mem_unmapL2Controller();
  printf("%d failures out of 100 tries.\n\n", fails);
}
