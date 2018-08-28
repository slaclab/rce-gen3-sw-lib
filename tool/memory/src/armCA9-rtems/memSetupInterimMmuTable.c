// -*-Mode: C;-*-
/**
@file
@brief Set up an MMU table similar to the one made by DAT code + BSP.
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
2013/11/11

@par Last commit:
\$Date: 2014-04-07 21:30:01 -0700 (Mon, 07 Apr 2014) $

@par Credits:
SLAC
*/
#include <stdint.h>
#include <stdio.h>



#include "debug/print.h"

#include "memory/mem.h"
#include "memory/cpu/memConstants.h"
#include "memory/cpu/memRegion.h"
#include "memory/cpu/memRegionConstants.h"
#include "memory/cpu/memTest.h"

extern  void mem_startMmuAndCachesAfterReset(uint32_t* tableFirst);

/* Linker symbols. */
extern char bsp_section_start_begin[];
extern char bsp_section_text_begin[];
extern char bsp_section_rodata_begin[];
extern char bsp_section_vector_begin[];
extern char bsp_section_data_begin[];
extern char bsp_section_work_begin[];
extern char bsp_translation_table_base[];

#define SECTIONS(a)    ((unsigned)(a) >> SECTION_SHIFT)
#define SMALL_PAGES(a) ((unsigned)(a) >> SMALL_PAGE_SHIFT)

/* Interim region names.*/
enum {
  REGION_LOW_DDR,
  REGION_HIGH_DDR,
  REGION_AXI0,
  REGION_AXI1,
  REGION_IOP,
  REGION_STATIC,
  REGION_HIGH_DEVICE,
  REGION_END_OF_LIST
};

/* Layout of the low part of the address space. */
static mem_LayoutItem lowDdrLayout[] = {
  {.pages = 0, .flags = MEM_RESERVED},    /* Catch derefs of NULL + offset */
  {.pages = 0, .flags = MEM_MMU},         /* MMU tables */
  {.pages = 0, .flags = MEM_TEXT},        /* start and text sections */
  {.pages = 0, .flags = MEM_RODATA},      /* rodata section */
  {.pages = 0, .flags = MEM_RWDATA}       /* vector and data sections */
};

/* Layout of the rest of dynamic RAM. */
static mem_LayoutItem highDdrLayout[] = {
  {.pages = 0,  .flags = MEM_NORMAL}       /* work section, syslog buffer */
};

static mem_LayoutItem deviceLayout[] = {
  {.pages = 0, .flags = MEM_REGISTERS}
};

static mem_LayoutItem highDeviceLayout[] = {
  {.pages = HIGH_SLCR_PAGES + HIGH_PS_PAGES, .flags = MEM_REGISTERS},
  {.pages = HIGH_GAP1_PAGES, .flags = MEM_RESERVED},
  {.pages = HIGH_CPU_PAGES,  .flags = MEM_REGISTERS},
  {.pages = HIGH_GAP2_PAGES, .flags = MEM_RESERVED},
  {.pages = HIGH_QSPI_PAGES, .flags = MEM_REGISTERS},
  {.pages = HIGH_GAP3_PAGES, .flags = MEM_RESERVED},
  {.pages = 0,               .flags = MEM_NORMAL } /* OCM */
};

static mem_Region interim_regions[] = {
  [REGION_LOW_DDR] =
  {.start    = 0x00000000,
   .pageSize = SMALL_PAGE_SHIFT,
   .pages    = 0,
   .layout   = lowDdrLayout,
  },

  [REGION_HIGH_DDR] =
  {.start    = (uint32_t)bsp_section_work_begin,
   .pageSize = SECTION_SHIFT,
   .pages    = 0,
   .layout   = highDdrLayout
  },

  [REGION_AXI0] = {
    .start    = AXI0_START,
    .pageSize = AXI0_LOG2_PAGE_SIZE,
    .pages    = AXI0_PAGES,
    .next     = 0,
    .layout   = deviceLayout
  },

  [REGION_AXI1] = {
    .start    = AXI1_START,
    .pageSize = AXI1_LOG2_PAGE_SIZE,
    .pages    = AXI1_PAGES,
    .next     = 0,
    .layout   = deviceLayout
  },

  [REGION_IOP]  = {
    .start    = IOP_START,
    .pageSize = IOP_LOG2_PAGE_SIZE,
    .pages    = IOP_PAGES,
    .next     = 0,
    .layout   = deviceLayout
  },

  [REGION_STATIC] = {
    .start    = STATIC_START,
    .pageSize = STATIC_LOG2_PAGE_SIZE,
    .pages    = STATIC_PAGES,
    .next     = 0,
    .layout   = deviceLayout,
  },

  [REGION_HIGH_DEVICE] = {
    .start    = HIGH_START,
    .pageSize = LOG2_SMALL_PAGE,
    .pages    = HIGH_PAGES,
    .next     = 0,
    .layout   = highDeviceLayout
  },

  [REGION_END_OF_LIST] =
  {.layout   = NULL
  }
};


static char* tableNext;
static char* tableFirst;
static char* tableLast;

static char* tableAllocator(int regionName, uint32_t numBytes) {
  char* next = tableNext;
  numBytes = (numBytes + (SMALL_PAGE_SIZE - 1U)) & ~(SMALL_PAGE_SIZE - 1U);
  if (tableNext + numBytes <= tableLast) {
    tableNext += numBytes;
    return next;
  }
  else {
    dbg_prints("mem_setupInterimMmuTable(): Out of page table space.\n");
    return 0;
  }
}

void mem_setupInterimMmuTable(void) {
  int ok;
  /* Set a location for the first-level table. */
  tableFirst = bsp_translation_table_base;
  /* Set a location for the first page table: right behind the first-level table. */
  tableNext = tableFirst +   16*1024;
  tableLast = tableFirst + 1024*1024;

  /* Set the non-constant parts of the Region descriptions. */
  lowDdrLayout[0].pages = SMALL_PAGES(bsp_translation_table_base);
  lowDdrLayout[1].pages = SMALL_PAGES(bsp_section_start_begin - bsp_translation_table_base);
  lowDdrLayout[2].pages = SMALL_PAGES(bsp_section_rodata_begin - bsp_section_start_begin);
  lowDdrLayout[3].pages = SMALL_PAGES(bsp_section_vector_begin - bsp_section_rodata_begin);
  interim_regions[REGION_LOW_DDR].pages = SMALL_PAGES(bsp_section_work_begin);
  interim_regions[REGION_HIGH_DDR].pages = SECTIONS(DDR_MEMORY_END - (unsigned)bsp_section_work_begin);

  ok = mem_setupMmuTable_core((uint32_t*)tableFirst, interim_regions, tableAllocator);
  if (ok) {
    mem_startMmuAndCachesAfterReset((uint32_t*)tableFirst);
    dbg_prints("\nmem_setupInterimMmuTable(): MMU table just installed:\n");
    mem_printMmuTable((uint32_t*)tableFirst, dbg_printv);
  }
}
