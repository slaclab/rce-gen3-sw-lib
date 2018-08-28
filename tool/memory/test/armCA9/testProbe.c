// -*-Mode: C;-*-
/**
@file
@brief Test virtual address translation and discovery of virtual address attributes.
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
2013/06/04

@par Last commit:
\$Date: 2014-04-07 21:30:01 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3125 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/memory/test/armCA9/testProbe.c $

@par Credits:
SLAC
*/
#include <stdint.h>
#include <stdio.h>


#include "memory/mem.h"

#include "utility.h"

static void probe(uint32_t vaddr);

void mem_testProbe(void) {
  printf("\nProbes of the MMU mapping. The test will attempt to\n");
  printf("get the physical address and memory properties of the\n");
  printf("virtual address of the mem_testProbe() function itself.\n");
  printf("Then it will do the same for the virtual addresses\n");
  printf("from 0 to 3 3/4 GB -1 in steps of 1/4 GB. For each address\n");
  printf("each of the six probe functions will be called and the\n");
  printf("results printed. All the results should be consistent\n");
  printf("with each other subject to the documented limitations.\n");
  printf("r=readable  w=writable  c=cached  d=device  h=handshake  s=shared\n");
  printf("\n");
  printf("V-address  function          P-address  flags\n");
  printf("---------- ----------------- ---------- ------\n");
  probe((uint32_t)mem_testProbe);
  const uint32_t gigFourth = 0x10000000;
  uint32_t i;
  for (i = 0; i < 16; ++i) {
    printf("\n");
    probe(i * gigFourth);
  }
}

static void sayProbe(uint32_t paddr, int hasPaddr, int flags, const char* name);

static void probe(uint32_t vaddr) {
  uint32_t paddr;
  int flags;
  paddr = mem_getPhysRead(vaddr);
  flags = 0;
  printf("0x%08lx ", (unsigned long)vaddr);
  sayProbe(paddr, 1, flags, "getPhysRead");
  printf("    PAR value = 0x%08x\n", mem_getPar(vaddr));

  paddr = mem_getPhysWrite(vaddr);
  flags = 0;
  printf("           ");
  sayProbe(paddr, 1, flags, "getPhysWrite");
  printf("\n");

  flags = mem_getFlagsRead(vaddr);
  printf("           ");
  sayProbe(0, 0, flags, "getFlagsRead");
  printf("\n");

  flags = mem_getFlagsWrite(vaddr);
  printf("           ");
  sayProbe(0, 0, flags, "getFlagsWrite");
  printf("\n");

  flags = 0;
  paddr = mem_getPhysFlagsRead(vaddr, &flags);
  printf("           ");
  sayProbe(paddr, 1, flags, "getPhysFlagsRead");
  printf("\n");

  flags = 0;
  paddr = mem_getPhysFlagsWrite(vaddr, &flags);
  printf("           ");
  sayProbe(paddr, 1, flags, "getPhysFlagsWrite");
  printf("\n");
}

static void sayProbe(uint32_t paddr, int hasPaddr, int flags, const char* name) {
  printf("%-18s", name);
  if (hasPaddr) printf("0x%08lx ", (unsigned long)paddr);
  else          printf("           ");
  printf("%s", (flags & MEM_READ)      ? "r" : " ");
  printf("%s", (flags & MEM_WRITE)     ? "w" : " ");
  printf("%s", (flags & MEM_CACHED)    ? "c" : " ");
  printf("%s", (flags & MEM_DEVICE)    ? "d" : " ");
  printf("%s", (flags & MEM_HANDSHAKE) ? "h" : " ");
  printf("%s", (flags & MEM_SHARED)    ? "s" : " ");
}
