// -*-Mode: C;-*-
/**
@file
@brief Private "exports" for items needed by "mem" library code outside of memRegion.c.
@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#if !defined(TOOL_MEMORY_MEMREGION_H)
#define      TOOL_MEMORY_MEMREGION_H

/* Specify the MMU properties of a contiguous set of pages. */
typedef struct {
  uint16_t pages; /* No. of pages. Zero means "the rest of the pages in the region". */
  uint16_t flags; /* Combination of values from mem_AttributeFlag (see mem.h). */
} mem_LayoutItem;

/* Region descriptor. */
typedef struct {
  uint32_t        start;     /* Starting address. */
  mem_LayoutItem* layout;    /* Address of layout array. */
  uint32_t        next;      /* Index of next page available for allocation. */
  uint16_t        pages;     /* Number of pages in region. */
  uint16_t        pageSize;  /* log2(page size) */
} mem_Region;

/* Table of Region descriptions, terminated by a sentinel with no layout. */
extern mem_Region mem_RegionTable[];


#endif
