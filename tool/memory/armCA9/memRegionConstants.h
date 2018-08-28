// -*-Mode: C;-*-
/**
@file
@brief Parameters used to define the Regions of a Zynq system and their layouts.

NOTE: This header file is included both in linker scripts and in C code. In
C code you may also need to include <inttypes.h>.

@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#if !defined(TOOL_MEMORY_MEMREGIONCONSTANTS_H)
#define      TOOL_MEMORY_MEMREGIONCONSTANTS_H

/* Page sizes are either section (1 MB) or small page (4 KB). */
#define LOG2_SECTION         20
#define LOG2_SMALL_PAGE      12

/* Starting addresses, page sizes and number of pages for the various
   Regions. Further details for some Regions follow this information.

   The NULLCATCHER, SYSLOG, MMU, RTS, HIGHREG and OCM Regions are all
   assumed to have small pages. Since no 1 MB section can have a
   mixture of page sizes, each contiguous area of memory with small
   pages must start and end on a 1 MB boundary. At the moment there
   are two such areas:
      (1) NULLCATCHER + SYSLOG + MMU + RTS.
      (2) HIGHREG + OCM.

   The MMU Region will have the 16K first-level table, which must start on a 16K
   boundary, followed by the page tables. Each small page requires four bytes
   for its page table entry and each page table has 256 entries, enough to cover
   a megabyte.
*/
#define NULLCATCHER_START         0x00000000
#define NULLCATCHER_PAGES         4

#define SYSLOG_START              (NULLCATCHER_START + (NULLCATCHER_PAGES << LOG2_SMALL_PAGE))
#define SYSLOG_PAGES              256

#define MMU_START                 (SYSLOG_START + (SYSLOG_PAGES << LOG2_SMALL_PAGE))
#define MMU_PAGES                 100

#define RTS_START                 (MMU_START + (MMU_PAGES << LOG2_SMALL_PAGE))
#define RTS_PAGES                 ((WORKSPACE_START >> LOG2_SMALL_PAGE) - NULLCATCHER_PAGES - SYSLOG_PAGES - MMU_PAGES)

#define WORKSPACE_START           0x0aa00000
#define WORKSPACE_PAGES           ((UNCACHED_START  - WORKSPACE_START) >> WORKSPACE_LOG2_PAGE_SIZE)
#define WORKSPACE_LOG2_PAGE_SIZE  LOG2_SECTION
#define WORKSPACE_PAGE_SIZE       (1 << WORKSPACE_LOG2_PAGE_SIZE)
#define WORKSPACE_HOLDBACK_PAGES  16    /* The amount of the workspace region held back from the OS. */

#define UNCACHED_START            0x25500000
#define UNCACHED_PAGES            ((DDR_MEMORY_END  - UNCACHED_START)  >> UNCACHED_LOG2_PAGE_SIZE)
#define UNCACHED_LOG2_PAGE_SIZE   LOG2_SECTION
#define UNCACHED_PAGE_SIZE        (1 << UNCACHED_LOG2_PAGE_SIZE)

#define DDR_MEMORY_END            0x40000000

// DEPRECATED, to be removed ASAP.
#define AXI0_START                0x40000000
#define AXI1_START                0x80000000
// End of DEPRECATED.

#define SOCKET_START              0x40000000
#define SOCKET_LOG2_PAGE_SIZE     LOG2_SECTION
#define SOCKET_PAGES              0x101

#define AXI0_TEST_START           (SOCKET_START + (SOCKET_PAGES << SOCKET_LOG2_PAGE_SIZE))
#define AXI0_TEST_LOG2_PAGE_SIZE  LOG2_SECTION
#define AXI0_TEST_PAGES           1

#define FW_VER_CTL_START          0x80000000
#define FW_VER_CTL_LOG2_PAGE_SIZE LOG2_SECTION
#define FW_VER_CTL_PAGES          1

#define BSI_START                 0x84000000
#define BSI_LOG2_PAGE_SIZE        LOG2_SECTION
#define BSI_PAGES                 1

#define AXI1_TEST_START           (BSI_START + (BSI_PAGES << BSI_LOG2_PAGE_SIZE))
#define AXI1_TEST_LOG2_PAGE_SIZE  LOG2_SECTION
#define AXI1_TEST_PAGES           1

#define USERDEV_START             0xa0000000
#define USERDEV_LOG2_PAGE_SIZE    LOG2_SECTION
#define USERDEV_PAGES             256

#define USERPPI_START             0xb0000000
#define USERPPI_LOG2_PAGE_SIZE    LOG2_SECTION
#define USERPPI_PAGES             256

#define IOP_START                 0xe0000000
#define IOP_LOG2_PAGE_SIZE        LOG2_SECTION
#define IOP_PAGES                 3

#define STATIC_START              0xe1000000
#define STATIC_LOG2_PAGE_SIZE     LOG2_SECTION
#define STATIC_PAGES              80

#define HIGHREG_START             0xf8000000
#define HIGHREG_PAGES             32704

/* Layout of the HIGHREG region, which is rather complicated. The gaps
   will be mapped to cause translation faults.
*/
#define HIGHREG_SLCR_PAGES ((uint16_t)((0xf8001000U - 0xf8000000U) >> LOG2_SMALL_PAGE))
#define HIGHREG_PS_PAGES   ((uint16_t)((0xf8810000U - 0xf8001000U) >> LOG2_SMALL_PAGE))
#define HIGHREG_GAP1_PAGES ((uint16_t)((0xf8900000U - 0xf8810000U) >> LOG2_SMALL_PAGE))
#define HIGHREG_CPU_PAGES  ((uint16_t)((0xf8f03000U - 0xf8900000U) >> LOG2_SMALL_PAGE))
#define HIGHREG_GAP2_PAGES ((uint16_t)((0xfc000000U - 0xf8f03000U) >> LOG2_SMALL_PAGE))
#define HIGHREG_QSPI_PAGES ((uint16_t)((0xfe000000U - 0xfc000000U) >> LOG2_SMALL_PAGE))
#define HIGHREG_GAP3_PAGES ((uint16_t)((0xfffc0000U - 0xfe000000U) >> LOG2_SMALL_PAGE))

#define OCM_START                 0xfffc0000
#define OCM_PAGES                 64

#endif

