// -*-Mode: C;-*-
/**
@file
@brief A generic function to generate C source code defining the
lookup tables used by mem_getFlagsRead() et al. for ARM.
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
Steve Tether <tether@slac.stanford.edu>

@par Date created:
2013/05/13

@par Last commit:
\$Date: 2013-09-13 13:13:44 -0700 (Fri, 13 Sep 2013) $ by \$Author: tether $.

@par Revision number:
\$Revision: 2104 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/memory/src/armCA9/genArmTables.c $

@par Credits:
SLAC
*/
#include <stdio.h>

static void decode(int par);

void mem_genArmTables(void) {
  int i;
  static const int numEntries = 64;
  printf("/** One byte of Physical Address Register info with the last two\n"
         "    bits equal to zero results in 64 4-byte table entries.\n"
         "    The index will be the PAR bits shifted left by 2.\n"
         "*/\n\n");

  printf("/* The table to use after an successful ATS1CPR operation. MEM_READ\n"
         "   will always be set. MEM_CACHED will be set if either inner or\n"
         "   outer caching is indicated.\n"
         "*/\n");

  printf("int mem_flagsLookupRead[] = {\n");
  for (i = 0; i < numEntries; ++i) {
    printf("  MEM_READ");
    decode(i);
    printf("%s\n", (i != numEntries-1) ? "," : "");
  }
  printf("};\n\n");


  printf("/* The table to use after an successful ATS1CPW operation. The same\n"
         "   as the first table except that MEM_WRITE is also always set.\n"
         "*/\n");

  printf("int mem_flagsLookupWrite[] = {\n");
  for (i = 0; i < numEntries; ++i) {
    printf("  MEM_READ | MEM_WRITE");
    decode(i);
    printf("%s\n", (i != numEntries-1) ? "," : "");
  }
  printf("};\n");
}

static void decode(int par) {
  int cached = 0;
  if (par & 0x20) printf(" | MEM_SHARED");
  switch((par & 0x1c) >> 2) { /* Test Inner bits. */
  case 0x1:
    printf(" | MEM_DEVICE | MEM_HANDSHAKE");
    break;
  case 0x3:
    printf(" | MEM_DEVICE");
    break;
  case 0x5:
  case 0x6:
  case 0x7:
    cached = 1;
    break;
  default:;
  }
  cached |= (par & 0x3); /* Test Outer bits. */
  if (cached) printf(" | MEM_CACHED");
}
