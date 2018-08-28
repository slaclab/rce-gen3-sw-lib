// -*-Mode: C;-*-
/**
@file
@brief Declarations exposed for the sake of unit tests.
@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#if !defined(TOOL_MEMORY_MEMTEST_H)
#define      TOOL_MEMORY_MEMTEST_H


#include "memory/cpu/memRegion.h"

/* @brief Call the MMU table setup function directly.

   @param[in] firstLevelTable This is where the first-level
   translation table should be placed.  In production this would be
   right after the null-catcher area of the RTS Region.

   @param[in] regionTable An array of Region objects terminated with a
   sentinel that has no layout. In production this would be
   mem_RegionTable.

   @param[in] alloc The allocator to use when creating the page
   tables. In production this would be mem_Region_alloc().

   For example, test program to examine the MMU table made from the
   production Region table will need to give a special value for
   firstLevelTable in order not to disturb a working system. It should
   also give a non-standard allocator for the same reason.

*/
int mem_setupMmuTable_core
(uint32_t* firstLevelTable, const mem_Region* regionTable, char* (*alloc)(int, uint32_t));

/** @brief Does the cache management, etc., required before enabling
    the MMU. Sets TTBR0 to the location of the base address of the MMU table.
*/
void mem_startMmuAndCachesAfterReset(uint32_t* tableBase);

/* @brief Does all the work for mem_explode(), which sets sectTable to
   the address of the currently active first-level MMU translation
   table.
*/
int mem_explode_core
(uint32_t        startVaddr,
 uint32_t        endVaddr,
 uint32_t* const sectTable,
 int             mmuActive,
 char*          (*alloc)(int, uint32_t));

/* @brief Does all the work for mem_setFlags(), which sets mmuActive
   to 1 and sectionTable to the address of the currently active
   first-level MMU translation table.
*/
void mem_setFlags_core
(uint32_t        startVaddr,
 uint32_t        endVaddr,
 int             flags,
 int             mmuActive,
 uint32_t* const sectTable);

#endif
