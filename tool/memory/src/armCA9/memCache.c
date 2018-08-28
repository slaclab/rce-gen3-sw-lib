// -*-Mode: C;-*-
/**
@file
@brief Implementation of the "mem" cache operations for ARM Cortex A9 (ARM v7).

@note These operations now assume that there's no address translation,
i.e., that virtual address == physical address.

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
Steve Tether <tether@slac.stanford.edu>, JJ Russell <russell@slac.stanford.edu>

@par Date created:
2013/10/11

@par Last commit:
\$Date: 2016-04-27 14:07:55 -0700 (Wed, 27 Apr 2016) $ by \$Author: smaldona $.

@par Revision number:
\$Revision: 4777 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/memory/src/armCA9/memCache.c $

@par Credits:
SLAC
*/


#include "memory/mem.h"
#include "memory/cpu/memConstants.h"

static inline uint32_t lineFloor(uint32_t virt) {return (uint32_t)virt & CACHE_LINE_MASK;}

static inline uint32_t lineCeil (uint32_t virt) {return ((uint32_t)virt + CACHE_LINE_SIZE - 1U) & CACHE_LINE_MASK;}


void mem_storeDataCacheRange(uint32_t virtBegin, uint32_t virtEnd, register uint32_t l2Base) {
    const uintptr_t beg = lineFloor (virtBegin);
    const uintptr_t end = lineCeil  (virtEnd);

    /* Make all following CP15 cache operations act on the L1 Data cache. */
    asm volatile
    (
        "mcr p15, 2, %0, c0, c0, 0 \n\t"
        "isb                       \n\t"
        : : "r"(0) : "memory"
     );

    /* Act on all specified cache lines. */
    register uintptr_t adr;
    for (adr = beg; adr < end; adr+= CACHE_LINE_SIZE) 
    {
        asm volatile
        (
             /* Clean L1 D-cache line to point of coherency.*/
            "mcr p15, 0, %[adr], c7, c10, 1       \n\t" 

             /* Clean L2 line. Atomic operation for PL310 L2CC.                 */
            "str %[adr], [%[l2Base], %[l2Clean]]  \n\t" 
                                                           
            :
            : [adr]     "r" (adr),
              [l2Base]  "r" (l2Base),
              [l2Clean] "i" (LEVEL2_CLEAN_BY_PA)
            : "memory"
        );
    }

    asm volatile ("dsb"); /* Wait for completion. */
}

void mem_invalidateDataCacheRange(uint32_t virtBegin, uint32_t virtEnd, register uint32_t l2Base) {
    const uintptr_t beg = lineFloor (virtBegin);
    const uintptr_t end = lineCeil  (virtBegin);

    /* Make all following CP15 cache operations act on the L1 Data cache. */
    asm volatile
    (
        "mcr p15, 2, %0, c0, c0, 0 \n\t"
        "isb                       \n\t"
        : : "r"(0) : "memory"
     );

    /* Act on all specified cache lines. */
    register uintptr_t adr;
    for (adr = beg; adr < end; adr+= CACHE_LINE_SIZE) 
    {
        asm volatile
        (
             /* Clean L2 line. Atomic operation for PL310 L2CC.  */
            "str %[adr], [%[l2Base], %[l2Inv]]  \n\t" 


             /* Clean L1 D-cache line to point of coherency.*/
            "mcr p15, 0, %[adr], c7, c6, 1       \n\t" 

                                                           
            :
            : [adr]     "r" (adr),
              [l2Base]  "r" (l2Base),
              [l2Inv]   "i" (LEVEL2_INVALIDATE_BY_PA)
            : "memory"
        );
    }

    asm volatile ("dsb"); /* Wait for completion. */
    return;
}


void mem_invalidateInstructionCacheRange(uint32_t virtBegin, uint32_t virtEnd) {
  const uint32_t begin = lineFloor(virtBegin);
  const uint32_t end   = lineCeil (virtEnd);

  register uint32_t va;
  /* Act on all specified cache lines. */
  for (va = begin; va < end; va += CACHE_LINE_SIZE) {
    asm volatile
      (
       "mcr p15, 0, %[va], c7, c5, 1        \n\t" /* Inval L1 I-cache line to point of unification. */
       "mcr p15, 0, %[va], c7, c5, 1        \n\t" /* Inval branch prediction. */
       "dsb                                 \n\t" /* Wait for completion. */
       :
       : [va]      "r" (va)
       : "memory"
        );
  }
}

void mem_invalidateInstructionCache(void) {
  asm volatile("mcr p15, 0, r0, c7, c5, 0"::: "memory");        /* ICIALLU */
}

static inline void waitForLevel2(volatile uint32_t* level2Controller) {
  uint32_t val;
  do {
    val = level2Controller[LEVEL2_SYNC >> 2] & 1U;
  } while(val != 0);
  asm volatile("dsb":::"memory");
}

static inline unsigned level1WayAndSet(unsigned w, unsigned s) {return (w << 30) | (s << 5);}

static inline void storeAllLevel1(void) {
  unsigned w, s;
  for (w = 0; w < LEVEL1_CACHE_WAYS; ++w) {
    for (s = 0; s < LEVEL1_CACHE_SETS; ++s) {
      const unsigned line = level1WayAndSet(w, s);
      asm volatile("mcr p15, 0, %0,  c7, c10, 2":: "r"(line): "memory"); /* DCCSW */
    }
  }
  asm volatile("dsb":::"memory");
}

static inline void storeAllLevel2(volatile uint32_t* level2Controller) {
  // Must be careful not to alter reserved bits.
  level2Controller[LEVEL2_CLEAN_WAYS >> 2] |= LEVEL2_CACHE_ALL_WAYS;
  waitForLevel2(level2Controller);
}

void mem_storeDataCache(uint32_t l2Base) {
  storeAllLevel1();
  storeAllLevel2((volatile uint32_t*)l2Base);
}

static inline void invalidateAllLevel2(volatile uint32_t* level2Controller) {
  level2Controller[LEVEL2_INVALIDATE_WAYS >> 2] |= LEVEL2_CACHE_ALL_WAYS;
  waitForLevel2(level2Controller);
}

void invalidateAllLevel1(void) {
  unsigned w, s;
  for (w = 0; w < LEVEL1_CACHE_WAYS; ++w) {
    for (s = 0; s < LEVEL1_CACHE_SETS; ++s) {
      const unsigned line = level1WayAndSet(w, s);
      asm volatile("mcr p15, 0, %0, c7, c6, 2":: "r"(line)); /* DCISW */
    }
  }
  asm volatile("dsb":::"memory");
}

void mem_invalidateDataCache(uint32_t l2Base) {
  /* Note the order of L1 vs. L2 compared to that in mem_storeDataCache(). */
  invalidateAllLevel2((volatile uint32_t*)l2Base);
  invalidateAllLevel1();
}
