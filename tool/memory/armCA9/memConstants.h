// -*-Mode: C;-*-
/**
@file
@brief Define private constants needed by the "mem" library implementation.
@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#if !defined(TOOL_MEMORY_MEMCONSTANTS_H)
#define      TOOL_MEMORY_MEMCONSTANTS_H

/* Type bits appearing in the low-order bits of translation table entries. */
/* In section TTEs. */
#define STTE_TYPE_PAGE_TABLE 1U
#define STTE_TYPE_SECTION    2U
#define STTE_TYPE_FAULT      0U
/* In page TTEs. */
#define PTTE_TYPE_SMALL_PAGE 1U
#define PTTE_TYPE_FAULT      0U

#define STTE_TYPE_MASK  3U
#define STTE_TYPE_SHIFT 0
#define STTE_TABLE_MASK 0xfffffc00

#define PTTE_TYPE_MASK  1U
#define PTTE_TYPE_SHIFT 1

/* Our default domain for all memory access. The shift used to place it in a TTE. */
#define MEMORY_DOMAIN     15U
#define TTE_DOMAIN_MASK   15U
#define STTE_DOMAIN_SHIFT  5

/* The size of the first-level MMU table: 4 bytes per 1 MB section, 4096 sections
   in a 32-bit address space.
*/
#define MMU_FIRST_LEVEL_TABLE_SIZE 16384

/* Other TTE fields. */ 
#define TTE_XN_MASK     1U
#define TTE_S_MASK      1U
#define TTE_AP2_MASK    1U
#define TTE_AP1_MASK    1U
#define TTE_ACCESS_MASK 1U /* AKA AP[0] */
#define TTE_TEX_MASK    7U
#define TTE_CB_MASK     3U

#define STTE_XN_SHIFT      4
#define STTE_S_SHIFT      16
#define STTE_AP2_SHIFT    15
#define STTE_AP1_SHIFT    11
#define STTE_ACCESS_SHIFT 10
#define STTE_TEX_SHIFT    12
#define STTE_CB_SHIFT      2

#define PTTE_XN_SHIFT      0
#define PTTE_S_SHIFT      10
#define PTTE_AP2_SHIFT     9
#define PTTE_AP1_SHIFT     5
#define PTTE_ACCESS_SHIFT  4
#define PTTE_TEX_SHIFT     6
#define PTTE_CB_SHIFT      2

/* TEX+CB for cached encode: normal, write-back, allocate on write. */
#define TEX_HANDSHAKE       0U
#define TEX_SHARED_DEVICE   0U
#define TEX_UNSHARED_DEVICE 2U
#define TEX_NORMAL          1U

#define CB_HANDSHAKE       0U
#define CB_SHARED_DEVICE   1U
#define CB_UNSHARED_DEVICE 0U
#define CB_CACHED          3U
#define CB_UNCACHED        0U

/* Virtual address fields. */
#define KILOBYTE (1U<<10)
#define MEGABYTE (1U<<20)

#define SECTION_SIZE  MEGABYTE
#define SECTION_SHIFT 20
#define SECTION_MASK  0xfffU

#define SMALL_PAGE_SIZE    (KILOBYTE << 2)
#define SMALL_PAGE_SHIFT   12
#define SMALL_PAGE_MASK    0xffU

#define PAGES_PER_SECTION  (1U << (SECTION_SHIFT - SMALL_PAGE_SHIFT))

/* CP15 System Control Register bits. */
#define ENABLE_ACCESS_FLAGS      (1U << 29)
#define ENABLE_ALIGNMENT_FAULTS  (1U << 1)
#define ENABLE_ALL_CACHES        ((1U << 12) | (1U << 2))
#define ENABLE_BRANCH_PREDICTION (1U << 11)
#define ENABLE_MMU               1U

/* Domain Access Control Register domain mode values. */
#define DOMAIN_CLIENT    1U
#define DOMAIN_MANAGER   3U
#define DOMAIN_NO_ACCESS 0U

/* Cache manipulation. */
/* For both L1 and L2 caches. */
#define CACHE_LINE_SIZE  32U
#define CACHE_LINE_MASK  (~(CACHE_LINE_SIZE - 1U))
#define LEVEL1_CACHE_SETS       256U
#define LEVEL1_CACHE_WAYS         4U
#define LEVEL2_CACHE_ALL_WAYS  0xffU  /* 8-way cache. */

/* Byte offsets to operation registers in the L2
   controller. Needs to be a macro so that we can use it with the "i"
   constraint in inline assembler code.
*/
#define LEVEL2_CLEAN_BY_PA      0x7b0U
#define LEVEL2_INVALIDATE_BY_PA 0x770U
#define LEVEL2_SYNC             0x730U
#define LEVEL2_CLEAN_WAYS       0x7bcU
#define LEVEL2_INVALIDATE_WAYS  0x77cU

#endif
