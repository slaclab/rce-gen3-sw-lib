// -*-Mode: C;-*-
/**
@file
@brief The Region table and related code.
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
2013/10/16

@par Last commit:
\$Date: 2014-08-19 12:00:02 -0700 (Tue, 19 Aug 2014) $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/memory/src/armCA9/memRegion.c $

@par Credits:
SLAC
*/
#include <stdio.h> /* For NULL. */



#include "memory/mem.h"
#include "memory/cpu/memConstants.h"
#include "memory/cpu/memRegion.h"
#include "memory/cpu/memRegionConstants.h"


static mem_LayoutItem nullcatcherLayout[] = { {.pages = 0, .flags = MEM_RESERVED} };

static mem_LayoutItem syslogLayout[]      = { {.pages = 0, .flags = MEM_RWDATA} };

static mem_LayoutItem mmuLayout[]         = { {.pages = 0, .flags = MEM_MMU} };

/* The default RTS Region permissions are chosen to be the most
   restrictive set possible that may be changed w/o having to perform
   cache manipulation: uncached and, so that RTEMS can run, executable.
*/
static mem_LayoutItem rtsLayout[]         = { {.pages = 0, .flags = MEM_READ | MEM_WRITE | MEM_EXEC} };

static mem_LayoutItem workspaceLayout[]   = { {.pages = 0, .flags = MEM_RWDATA} };

static mem_LayoutItem uncachedLayout[]    = { {.pages = 0, .flags = MEM_UNCACHED} };

static mem_LayoutItem highregLayout[] = {
  {.pages = HIGHREG_SLCR_PAGES + HIGHREG_PS_PAGES, .flags = MEM_REGISTERS},
  {.pages = HIGHREG_GAP1_PAGES, .flags = MEM_RESERVED},
  {.pages = HIGHREG_CPU_PAGES,  .flags = MEM_REGISTERS},
  {.pages = HIGHREG_GAP2_PAGES, .flags = MEM_RESERVED},
  {.pages = HIGHREG_QSPI_PAGES, .flags = MEM_REGISTERS},
  {.pages = HIGHREG_GAP3_PAGES, .flags = MEM_RESERVED}
};

static mem_LayoutItem ocmLayout[] = {
  {.pages = 0, .flags = MEM_READ | MEM_WRITE | MEM_CACHED | MEM_SHARED}
};

/* Layout for a Region that is entirely mapped to R/W device registers. */
static mem_LayoutItem deviceLayout[] = {{0, MEM_REGISTERS}};

/* Internal indexes for the Regions in the following table. Unlike for
   the public names there must be no gaps and the sequence must be in
   order by start address.
*/
enum {
  REGION_NULLCATCHER,
  REGION_SYSLOG,
  REGION_MMU,
  REGION_RTS,
  REGION_WORKSPACE,
  REGION_UNCACHED,
  REGION_SOCKET,
  REGION_AXI0_TEST,
  REGION_FW_VER_CTL,
  REGION_BSI,
  REGION_AXI1_TEST,
  REGION_USERDEV,
  REGION_USERPPI,
  REGION_IOP,
  REGION_STATIC,
  REGION_HIGHREG,
  REGION_OCM,
  NUM_REGIONS
};

/* Table of Regions. Address space not covered by any Region will be given 1 MB pages
   and the flag set MEM_RESERVED. Regions must be ordered by increasing start address.
*/
mem_Region mem_RegionTable[] = {
  /* Regions partitioning dynamic RAM.*/
  [REGION_NULLCATCHER] = {
    .start     = NULLCATCHER_START,
    .pageSize  = LOG2_SMALL_PAGE,
    .pages     = NULLCATCHER_PAGES,
    .next      = 0,
    .layout    = nullcatcherLayout
  },
  [REGION_SYSLOG] = {
    .start     = SYSLOG_START,
    .pageSize  = LOG2_SMALL_PAGE,
    .pages     = SYSLOG_PAGES,
    .next      = 0,
    .layout    = syslogLayout,
  },
  [REGION_MMU] = {
    .start     = MMU_START,
    .pageSize  = LOG2_SMALL_PAGE,
    .pages     = MMU_PAGES,
    /* Preallocate space for the 1st-level translation table. */
    .next      = (MMU_FIRST_LEVEL_TABLE_SIZE >> LOG2_SMALL_PAGE),
    .layout    = mmuLayout
  },
  [REGION_RTS] = {
    .start     = RTS_START,
    .pageSize  = LOG2_SMALL_PAGE,
    .pages     = RTS_PAGES,
    .next      = 0,
    .layout    = rtsLayout
  },
  [REGION_WORKSPACE] = {
    .start     = WORKSPACE_START,
    .pageSize  = WORKSPACE_LOG2_PAGE_SIZE,
    .pages     = WORKSPACE_PAGES,
    .next      = 0,
    .layout    = workspaceLayout
  },
  [REGION_UNCACHED] = {
    .start     = UNCACHED_START,
    .pageSize  = UNCACHED_LOG2_PAGE_SIZE,
    .pages     = UNCACHED_PAGES,
    .next      = 0,
    .layout    = uncachedLayout
  },

  /* Regions determined by the hardware or firmware.
  */

  [REGION_SOCKET] = {
    .start    = SOCKET_START,
    .pageSize = SOCKET_LOG2_PAGE_SIZE,
    .pages    = SOCKET_PAGES,
    .next     = 0,
    .layout   = deviceLayout
  },

  [REGION_AXI0_TEST] = {
    .start    = AXI0_TEST_START,
    .pageSize = AXI0_TEST_LOG2_PAGE_SIZE,
    .pages    = AXI0_TEST_PAGES,
    .next     = 0,
    .layout   = deviceLayout
  },

  [REGION_FW_VER_CTL] = {
    .start    = FW_VER_CTL_START,
    .pageSize = FW_VER_CTL_LOG2_PAGE_SIZE,
    .pages    = FW_VER_CTL_PAGES,
    .next     = 0,
    .layout   = deviceLayout
  },

  [REGION_BSI] = {
    .start    = BSI_START,
    .pageSize = BSI_LOG2_PAGE_SIZE,
    .pages    = BSI_PAGES,
    .next     = 0,
    .layout   = deviceLayout
  },

  [REGION_AXI1_TEST] = {
    .start    = AXI1_TEST_START,
    .pageSize = AXI1_TEST_LOG2_PAGE_SIZE,
    .pages    = AXI1_TEST_PAGES,
    .next     = 0,
    .layout   = deviceLayout
  },

  [REGION_USERDEV] = {
    .start    = USERDEV_START,
    .pageSize = USERDEV_LOG2_PAGE_SIZE,
    .pages    = USERDEV_PAGES,
    .layout   = deviceLayout
  },

  [REGION_USERPPI] = {
    .start    = USERPPI_START,
    .pageSize = USERPPI_LOG2_PAGE_SIZE,
    .pages    = USERPPI_PAGES,
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

  [REGION_HIGHREG] = {
    .start    = HIGHREG_START,
    .pageSize = LOG2_SMALL_PAGE,
    .pages    = HIGHREG_PAGES,
    .next     = 0,
    .layout   = highregLayout
  },

  [REGION_OCM] = {
    .start    = OCM_START,
    .pageSize = LOG2_SMALL_PAGE,
    .pages    = OCM_PAGES,
    .layout   = ocmLayout
  },

  [NUM_REGIONS] = {.layout = NULL}
};


/* Translation from public to internal names. */
static int nameMap[] = {
  [MEM_REGION_SYSLOG]    = REGION_SYSLOG,
  [MEM_REGION_MMU]       = REGION_MMU,
  [MEM_REGION_RTS]       = REGION_RTS,
  [MEM_REGION_WORKSPACE] = REGION_WORKSPACE,
  [MEM_REGION_UNCACHED]  = REGION_UNCACHED,
  [MEM_REGION_SOCKET]    = REGION_SOCKET,
  [MEM_REGION_AXI0_TEST] = REGION_AXI0_TEST,
  [MEM_REGION_FW_VER_CTL]= REGION_FW_VER_CTL,
  [MEM_REGION_BSI]       = REGION_BSI,
  [MEM_REGION_AXI1_TEST] = REGION_AXI1_TEST,
  [MEM_REGION_USERDEV]   = REGION_USERDEV,
  [MEM_REGION_USERPPI]   = REGION_USERPPI,
  [MEM_REGION_IOP]       = REGION_IOP,
  [MEM_REGION_STATIC]    = REGION_STATIC,
  [MEM_REGION_HIGHREG]   = REGION_HIGHREG,
  [MEM_REGION_OCM]       = REGION_OCM 
};

char* mem_Region_alloc(int name, uint32_t numBytes) {
  mem_Region* const reg       = mem_RegionTable + nameMap[name];
  uint32_t    const pageSize  = reg->pageSize;
  uint32_t    const remainder = (1 << pageSize) - 1;
  uint32_t    const request   = ((numBytes + remainder) & ~remainder) >> pageSize; // Units = pages.
  uint32_t    const current   = reg->next;
  uint32_t    const next      = current + request;
  if (next > reg->pages) {
    return NULL;
  }
  else {
    reg->next = next;
    return (char*)reg->start + (current << pageSize);
  }
}

void mem_Region_undoAlloc(int name, char* allocPtr) {
  mem_Region* const reg       = mem_RegionTable + nameMap[name];
  uint32_t    const pageSize  = reg->pageSize;
  uint32_t    const request   = ((char*)(reg->start + (reg->next<<pageSize)) - allocPtr) >> pageSize;
  reg->next -= request;
}

char* mem_Region_startAddress(int name) {
  mem_Region* const reg       = mem_RegionTable + nameMap[name];
  return (char*)reg->start;
}

uint32_t mem_Region_remaining(int name) {
  mem_Region* const reg = mem_RegionTable + nameMap[name];
  uint32_t    const pageSize  = reg->pageSize;
  uint32_t    const current   = reg->next;
  uint32_t    const total     = reg->pages;
  return (total - current) << pageSize;;
}

uint32_t mem_Region_size(int name) {
  mem_Region* const reg = mem_RegionTable + nameMap[name];
  return reg->pages << reg->pageSize;
}

uint32_t mem_Region_pageSize(int name) {
  mem_Region* const reg = mem_RegionTable + nameMap[name];
  return (1U << reg->pageSize);
}

uint32_t mem_Region_workspaceHoldback(void) {
  return WORKSPACE_HOLDBACK_PAGES * WORKSPACE_PAGE_SIZE;
}

static const char* regionString[] = {
  [REGION_NULLCATCHER] = "NULL-catcher",
  [REGION_SYSLOG]      = "SYSLOG",
  [REGION_MMU]         = "MMU",
  [REGION_RTS]         = "RTS",
  [REGION_WORKSPACE]   = "WORKSPACE",
  [REGION_UNCACHED]    = "UNCACHED",
  [REGION_SOCKET]      = "SOCKET",
  [REGION_AXI0_TEST]   = "AXI0_TEST",
  [REGION_FW_VER_CTL]  = "FW_VER_CTL",
  [REGION_BSI]         = "BSI",
  [REGION_AXI1_TEST]   = "AXI1_TEST",
  [REGION_USERDEV]     = "USERDEV",
  [REGION_USERPPI]     = "USERPPI",
  [REGION_IOP]         = "IOP",
  [REGION_STATIC]      = "STATIC",
  [REGION_HIGHREG]     = "HIGHREG",
  [REGION_OCM]         = "OCM"
};

static const char* flagString[] = {
  "READ", "WRITE", "EXEC", "CACHED", "DEVICE", "HANDSHAKE", "SHARED"
};

void mem_Region_printConfig(void (*print)(const char* fmt, ...)) {
  int ir;
  const unsigned kilo = 10;
  const unsigned mega = 20;
  const unsigned giga = 30;
  print("\nMemory region configuration\n"
           "---------------------------\n");
  for (ir = 0; ir < NUM_REGIONS; ++ir) {
    const mem_Region* const reg = mem_RegionTable + ir;
    const char* units;
    int shift = reg->pageSize;
    int  pagesLeft = reg->pages;
    const mem_LayoutItem* lay = reg->layout;
    if (shift < kilo) {
      units = "";
    }
    else if (shift < mega) {
      shift -= kilo;
      units = "K";
    }
    else if (shift < giga) {
      shift -= mega;
      units = "M";
    }
    else {
      shift -= giga;
      units = "G";
    }
    print("Region %-15s: start 0x%08x   page size %u%s   pages %5u   next free page %5u\n",
          regionString[ir], (unsigned)reg->start, 1U<<shift, units, (unsigned)reg->pages, (unsigned)reg->next);
    while (pagesLeft > 0) {
      int pages = lay->pages ? lay->pages : pagesLeft;
      int flags = lay->flags;
      pagesLeft -= pages;
      print("%9u pages of", pages);
      if (flags == MEM_RESERVED) {
        print(" RESERVED");
      }
      else {
        unsigned pos;
        for (pos = 0; pos < (sizeof flagString)/(sizeof flagString[0]); ++pos) {
          if ( (flags & (1U << pos)) != 0) print(" %s", flagString[pos]);
        }
      }
      print("\n");
      ++lay;
    }
  }
}
