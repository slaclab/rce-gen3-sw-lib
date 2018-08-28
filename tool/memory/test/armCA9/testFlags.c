// -*-Mode: C;-*-
/**
@file
@brief Test mem_setFlags() and mem_explode().
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
2013/08/27

@par Last commit:
\$Date: 2014-04-07 21:30:01 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3125 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/memory/test/armCA9/testFlags.c $

@par Credits:
SLAC
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#include "memory/mem.h"
#include "memory/cpu/memConstants.h"
#include "memory/cpu/memRegion.h"
#include "memory/cpu/memTest.h"
#include "utility.h"

/* Allocator used for page tables made during the creation of the off-line MMU table. */
static char* tableStore;
static char* tableNext;
static char* tableLast;
static char* tableAllocator(int regionName, uint32_t numBytes) {
  char* next = tableNext;
  numBytes = (numBytes + (SMALL_PAGE_SIZE - 1U)) & ~(SMALL_PAGE_SIZE - 1U);
  if (tableNext + numBytes <= tableLast) {
    tableNext += numBytes;
    return next;
  }
  else {
    printf("Out of page table space.\n");
    return 0;
  }
}

static void print(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
}

void mem_testFlags(void) {
  int ok = 1;
  int status;
  unsigned const tableSize = 320*KILOBYTE;
  print("\nThis test routine builds an off-line set of MMU translation tables\n"
         "and then modifies it in various ways, making a dump of the tables\n"
         "after every change.\n\n");

  /* Allocate the table store on a 16K boundary; the first 16K will be used
     for the first-level table and the rest for the page tables. Since the
     table is off-line we don't actually need the alignment but we may want to check
     that the table building doesn't itself produce bad alignment.
  */
  if ( (status = posix_memalign((void**)(&tableStore), 16*KILOBYTE, tableSize)) ) {
    ok = 0;
    print("posix_memalign() failed: %s\n", strerror(status));
  }
  tableLast = tableStore + tableSize;
  tableNext = tableStore + 16*KILOBYTE; /* Reserve space for the first-level table. */
  print("Valid table store range [%p, %p)\n", tableStore, tableLast);
  /* Now create the off-line tables according to the standard memory layout. */
  if (ok) {
    ok = mem_setupMmuTable_core((uint32_t*)tableStore, mem_RegionTable, tableAllocator);
    if (!ok) print("MMU table creation failed!\n");
  }

  if (ok) {
    print("Current MMU table:\n");
    mem_printMmuTable((uint32_t*)tableStore, print);
  }

  if (ok) {
    print("Make a few pages of low memory to MEM_RODATA. [5K, 15K).\n");
    mem_setFlags_core(5*KILOBYTE, 15*KILOBYTE, MEM_RODATA, 0, (uint32_t*)tableStore);
    print("New MMU table:\n");
    mem_printMmuTable((uint32_t*)tableStore, print);
  }

  if (ok) {
    print("Make most of the first section MEM_RODATA. [4K, 1M).\n");
    mem_setFlags_core(4*KILOBYTE, 1*MEGABYTE, MEM_RODATA, 0, (uint32_t*)tableStore);
    print("New MMU table:\n");
    mem_printMmuTable((uint32_t*)tableStore, print);
  }

  if (ok) {
    print("Make [1000K, 2M+16K) MEM_UNCACHED.\n");
    mem_setFlags_core(1000*KILOBYTE, 2*MEGABYTE + 16*KILOBYTE, MEM_UNCACHED, 0, (uint32_t*)tableStore);
    print("New MMU table:\n");
    mem_printMmuTable((uint32_t*)tableStore, print);
  }

  if (ok) {
    print("Make 4K pages in [0x0b000000, 0x0b100000)\n");
    ok = mem_explode_core(0x0b000000U, 0x0b100000U, (uint32_t*)tableStore, 0, tableAllocator);
  }

  if (ok) {
    print("New MMU table:\n");
    mem_printMmuTable((uint32_t*)tableStore, print);
  }

  free(tableStore);
}
