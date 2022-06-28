// -*-Mode: C;-*-
/**
@file
@brief Implementation of the "mem" MMU table operations for ARM Cortex A9 (ARM v7).

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
2013/10/11

@par Last commit:
\$Date: 2014-04-24 18:06:45 -0700 (Thu, 24 Apr 2014) $ by \$Author: tether $.

@par Revision number:
\$Revision: 3249 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/memory/src/armCA9/memMmu.c $

@par Credits:
SLAC
*/

#include <stdio.h> /* For NULL. */



#include "debug/print.h"
#include "memory/mem.h"

#ifdef __arm__
#include "memory/armCA9/memConstants.h"
#include "memory/armCA9/memRegion.h"
#include "memory/armCA9/memRegionConstants.h"
#include "memory/armCA9/memTest.h"
#endif

/* Assumptions this code makes about the MMU translation table:

   Only short-form entries are used.

   The first level contains one entry per 1 MB section. It starts at
   somw 16 KB-aligned address.  It contains one 4-byte entry per
   section for a total of 16 KB.  Each entry is either a direct
   section entry or a reference to a page table.

   A page table is aligned on a 1 KB boundary and contains 256 entries
   each of which describes a 4 KB page. A page table, once created, is
   never destroyed. 

   A single, entire Region is dedicated to translation table storage.
   First comes the primary table followed by page tables allocated on
   demand.

   No TEX remapping.

   Abbreviated AP fields AP[2:1]. AP[1] is always set to
   1 so that access is possible from any privilege level,
   though perhaps restricted to read-only by AP[2].

   The cache policy for cached memory is always write-back,
   allocate on write for both the inner and outer caches.

   The translation table is normally kept in DRAM marked as
   read-only. To modify it we temporarily alter the Domain
   Access Control Register, setting all domains to Manager
   mode in order to bypass access the access bit settings
   in the translation table.

   The MMU translation tables are kept in memory that is mapped as
   virtual == real.

   We're always in Secure mode.

 */

/* AP and TEXCB fields from Tables B3-6 and B3-10 in the ARMv7-A/R
   Architecture Manual, leaving out reserved or redundant
   combinations.

   (TEX, C, B) (xxx, x, x)
   -----------------------
   Strongly Ordered:  0b00000
   Shareable device:  0b00001
   Non-shareable dev: 0b01000
   Normal, uncached:  0b00100
   Normal, WB WA:     0b00111

   AP[2, 1]
   RW, any privilege level:    0b01
   R,  any privilege level:    0b11
*/

/** Values of contiguous bit-fields controlling properties
    in all section and 4K-page translation table entries.
*/
typedef struct {
  uint32_t xn;
  uint32_t s;
  uint32_t ap2;
  uint32_t ap1;
  uint32_t access; /* AP[0] */
  uint32_t tex;
  uint32_t cb;
  uint32_t stype; /* Type code of TTE if for a section. */
  uint32_t ptype; /* Type code of TTE if for a page. */
} PropParts;

static inline void flagsToPropParts(int flags, PropParts* parts) {
  if (!(flags & (MEM_READ | MEM_WRITE))) {
    /* Inaccessible. */
    parts->xn     = 0;
    parts->s      = 0;
    parts->ap2    = 0;
    parts->ap1    = 0;
    parts->access = 0;
    parts->tex    = 0;
    parts->cb     = 0;
    parts->stype  = STTE_TYPE_FAULT;
    parts->ptype  = PTTE_TYPE_FAULT;
  }
  else {
    parts->stype = STTE_TYPE_SECTION;
    parts->ptype = PTTE_TYPE_SMALL_PAGE;
    parts->access = 1; /* Prevent Access faults. */
    parts->ap2  = (flags & MEM_WRITE)  ? 0 : 1;
    parts->ap1  = 1;
    if (flags & MEM_HANDSHAKE) {
      /* Strongly Ordered. Always shared, S-bit unused. */
      parts->xn   = 1;
      parts->s    = 0;
      parts->tex  = TEX_HANDSHAKE;
      parts->cb   = CB_HANDSHAKE;
    }
    else if (flags & MEM_DEVICE) {
      /* Device, not strongly ordered. S-bit unused, may still be shared. */
      parts->xn    = 1;
      parts->s     = 0;
      parts->tex   = (flags & MEM_SHARED) ? TEX_SHARED_DEVICE : TEX_UNSHARED_DEVICE;
      parts->cb    = (flags & MEM_SHARED) ? CB_SHARED_DEVICE  : CB_UNSHARED_DEVICE;
    }
    else {
      /* Normal. */
      parts->xn   = (flags & MEM_EXEC)   ? 0 : 1;
      parts->s    = (flags & MEM_SHARED) ? 1 : 0;
      parts->tex  = TEX_NORMAL;
      parts->cb   = (flags & MEM_CACHED) ? CB_CACHED : CB_UNCACHED;
    }
  }
}

static inline unsigned sectionNum(uintptr_t virtAddr) {return virtAddr >> SECTION_SHIFT;}

static inline uint32_t sectionFloor(uint32_t vaddr) {return vaddr & ~(SECTION_SIZE - 1U);}

static inline uint32_t sectionCeil(uint32_t vaddr) {return sectionFloor(vaddr + SECTION_SIZE - 1U);}

static inline int      sectionAligned(uint32_t vaddr) {return (vaddr == sectionFloor(vaddr)) ? 1 : 0;}

static inline uint32_t* getPageTable(uint32_t stte) {
  unsigned const type = (stte >> STTE_TYPE_SHIFT) & STTE_TYPE_MASK;
  return (type == STTE_TYPE_PAGE_TABLE) ? (uint32_t*)(stte & STTE_TABLE_MASK) : NULL;
}

static inline uint32_t sectionPropertyBits(const PropParts* parts) {
  return
      (parts->xn     << STTE_XN_SHIFT)
    | (parts->s      << STTE_S_SHIFT)
    | (parts->ap2    << STTE_AP2_SHIFT)
    | (parts->ap1    << STTE_AP1_SHIFT)
    | (parts->access << STTE_ACCESS_SHIFT)
    | (parts->tex    << STTE_TEX_SHIFT)
    | (parts->cb     << STTE_CB_SHIFT)
    | (parts->stype  << STTE_TYPE_SHIFT);
}

static inline void decodeSectionProperties(PropParts* parts, uint32_t stte) {
    parts->xn     = (stte >> STTE_XN_SHIFT)   & TTE_XN_MASK;
    parts->s      = (stte >> STTE_S_SHIFT)    & TTE_S_MASK;
    parts->ap2    = (stte >> STTE_AP2_SHIFT)  & TTE_AP2_MASK;
    parts->ap1    = (stte >> STTE_AP1_SHIFT)  & TTE_AP1_MASK;
    parts->access = 1; /* Must be set in order to prevent Access faults.*/
    parts->tex    = (stte >> STTE_TEX_SHIFT)  & TTE_TEX_MASK;
    parts->cb     = (stte >> STTE_CB_SHIFT)   & TTE_CB_MASK;
    parts->stype  = (stte >> STTE_TYPE_SHIFT) & STTE_TYPE_MASK;
    parts->ptype  = parts->stype == STTE_TYPE_FAULT ? PTTE_TYPE_FAULT : PTTE_TYPE_SMALL_PAGE;
}

static inline uint32_t sectionTte(uint32_t sectVaddr, uint32_t sectProps) {
  return sectVaddr | (MEMORY_DOMAIN << STTE_DOMAIN_SHIFT) | sectProps;
}

static inline uint32_t pageTableRefTte(uint32_t* pageTable) {
  return sectionTte((uint32_t)pageTable, STTE_TYPE_PAGE_TABLE << STTE_TYPE_SHIFT);
}

static inline unsigned pageNum(uint32_t virtAddr) {return (virtAddr >> SMALL_PAGE_SHIFT) & SMALL_PAGE_MASK;}

static inline uint32_t pageFloor(uint32_t vaddr) {return vaddr & ~(SMALL_PAGE_SIZE - 1U);}

static inline uint32_t pageCeil(uint32_t vaddr) {return pageFloor(vaddr + SMALL_PAGE_SIZE - 1U);}

static inline int      pageAligned(uint32_t vaddr) {return (vaddr == pageFloor(vaddr)) ? 1 : 0;}

static inline uint32_t pagePropertyBits(const PropParts* parts) {
  return
      (parts->xn     << PTTE_XN_SHIFT)
    | (parts->s      << PTTE_S_SHIFT)
    | (parts->ap2    << PTTE_AP2_SHIFT)
    | (parts->ap1    << PTTE_AP1_SHIFT)
    | (parts->access << PTTE_ACCESS_SHIFT)
    | (parts->tex    << PTTE_TEX_SHIFT)
    | (parts->cb     << PTTE_CB_SHIFT)
    | (parts->ptype  << PTTE_TYPE_SHIFT);
}

/* Set entries in a page table. */
static inline void setPageTable
(uint32_t* nextTte, uint32_t propBits, uint32_t nextVaddr, uint32_t endVaddr) {
  if (!pageAligned(nextVaddr) || !pageAligned(endVaddr)) {
    dbg_printv("setPageTable(): Unaligned address(es). nextVaddr = %p, endVaddr = %p.\n",
               (void*)nextVaddr, (void*)endVaddr);
    return;
  }
  while (nextVaddr != endVaddr) {
    *nextTte++ = nextVaddr | propBits;
    nextVaddr += SMALL_PAGE_SIZE;
  }
}

/* Set the contents of TTBR0. */
static inline void setTtbr0(register uint32_t val) {
  asm volatile
    (
     "mcr p15, 0, %[val], c2, c0, 0"
     :
     : [val] "r"(val)
     :
     );
}


static inline void setTtbr0WithFlags(uint32_t* firstLevelTable, int flags) {
  uint32_t ttbr0 = (uint32_t)firstLevelTable;
  /*
  According to the V7A/R Architecture manual the lowest order seven
  bits in TTBR0 have to match some of the attributes of the memory
  holding the translation table. However in practice I find that
  anything other than zero in these bits causes crashes. So for now at
  least we have to ignore the flags argument.
  */
  setTtbr0(ttbr0);
}

/* Use the Region information in the Region table and layouts to
   create an MMU translation table.

   The first-level table will occupy the first MEM_MMU_TABLE_SIZE bytes in
   the MMU region.  The second-level table will be
   allocated dynamically from the same Region.
*/
typedef struct {
  uint32_t* nextSectionTte;  /* Where the next section TTE will go. */
  uint32_t* nextPageTte;     /* Where the next page TTE will go. */
  uint32_t  nextVaddr;       /* The next virtual address to be covered by a new TTE. */
  uint32_t  pageTablesStart; /* Address of the start of the page tables (second-level tables). */
  uint32_t  pageTablesEnd;   /* Address of next byte after the page tables. */
} MmuInfo;

static void  allocatePageTables(MmuInfo*, const mem_Region*, char* (*alloc)(int, uint32_t));
static void  makeTableEntries  (MmuInfo*, const mem_Region*);
static void  makeReserved      (MmuInfo*, const mem_Region*);
static void  makePageTables    (MmuInfo*, const mem_Region*);
static void  makeSections      (MmuInfo*, const mem_Region*);

int mem_setupMmuTable() {
  return mem_setupMmuTable_core((uint32_t*)MMU_START, mem_RegionTable, mem_Region_alloc);
}

int mem_setupMmuTable_core
(uint32_t* firstLevelTable, const mem_Region* regionTable, char* (*alloc)(int, uint32_t)) {
  MmuInfo info = {.nextVaddr = 0, .nextSectionTte = firstLevelTable};
  /* Make a pass over the Region table to see how many
     page tables we'll have to allocate, then allocate them.
  */
  allocatePageTables(&info, regionTable, alloc);
  if (!info.nextPageTte) return 0;

  /* Make a second pass over the Region table to create the
     translation table entries.
  */
  makeTableEntries(&info, regionTable);

  /* Now use what we've made. */
  mem_startMmuAndCachesAfterReset(firstLevelTable);
  return 1;
}

/* Read the CP15 SCTRL register. */
static inline unsigned getControlRegister(void) {
  unsigned val;
  asm volatile("mrc p15, 0, %0,  c1,  c0, 0": "=r"(val) :: "memory");
  return val;
}


/* Set a new value in the CP15 SCTRL, returning the old value. */
static inline unsigned setControlRegister(unsigned newVal) {
  unsigned oldVal;
  asm volatile("mrc p15, 0, %0,  c1,  c0, 0 \n\t"
               "mcr p15, 0, %1,  c1,  c0, 0 \n\t"
               "dsb    \n\t"
               "isb"
               : "=&r"(oldVal) : "r"(newVal): "memory");
  return oldVal;
}

/* Assume that the data caches are off and the MMU is off but that the MMU table has been constructed. */
void mem_startMmuAndCachesAfterReset(uint32_t* tableBase) {
  unsigned sctrl = getControlRegister();
  sctrl &= ~ENABLE_ALIGNMENT_FAULTS;
  sctrl |= ENABLE_ACCESS_FLAGS | ENABLE_BRANCH_PREDICTION;
  setControlRegister(sctrl);

  /* Invalidate all data cache lines so that any garbage in them doesn't get written
     to memory at some random time later.
  */
  mem_invalidateDataCache(mem_mapL2Controller());
  mem_unmapL2Controller();

  /* Invalidate all TLB entries. After a reset the TLB may contain garbage. */
  asm volatile("mcr p15, 0, %0,  c8,  c7, 0":: "r"(0): "memory");

  /* Set the DACR to enable Client mode for the memory domain we use,
     i.e., make the CPU core respect the permission bits in the MMU
     translation table once the MMU is turned on.
  */
  asm volatile
    (
     "mcr p15, 0, %[new],  c3, c0, 0 \n\t" /* Write to the DACR. */
     : 
     : [new] "r"(DOMAIN_CLIENT << (MEMORY_DOMAIN<<1))
     : "memory"
     );

  /* No need to change the reset value of TTBCR, which disables TTBR1. */
  /* Set TTBR0. */
  setTtbr0WithFlags(tableBase, MEM_MMU);

  /* DSB to wait for memory writes to finish. ISB to let machine state
     changes take effect.
  */
  asm volatile("dsb":::"memory");
  asm volatile("isb":::"memory");

  /* Set the enable bits we want. */
  sctrl |= ENABLE_MMU | ENABLE_ALL_CACHES;
  setControlRegister(sctrl);

  /* ISB again. */
  asm volatile("isb":::"memory");
}


/* Allocate space for all secondary translation tables required by the Region configuration. */
static void allocatePageTables(MmuInfo* info, const mem_Region* regionTable, char* (*alloc)(int, uint32_t)) {
  unsigned pages = 0;
  const mem_Region* reg;
  uint32_t bytes;
  for (reg = regionTable; reg->layout; ++reg) {
    if (reg->pageSize == SMALL_PAGE_SHIFT) pages += reg->pages;
  }
  bytes = pages * sizeof(uint32_t);
  info->nextPageTte     = (uint32_t*)alloc(MEM_REGION_MMU, bytes);
  info->pageTablesStart = (uint32_t)info->nextPageTte;
  info->pageTablesEnd   = info->pageTablesStart + bytes;
}

/* Write all the translation table entries required by the Region configuration. */
static void makeTableEntries(MmuInfo* info, const mem_Region* regionTable) {
  const mem_Region* reg;
  for (reg = regionTable; reg->layout; ++reg) {
    
    if (reg->start > info->nextVaddr) {
      /* A gap between regions. Make access cause a translation fault. */
      makeReserved(info, reg);
    }
    if (reg->pageSize == SMALL_PAGE_SHIFT) {
      makePageTables(info, reg);
    }
    else {
      makeSections(info, reg);
    }
    
  }

  /* If the Region table covers the entire 4 GB address space then the
     next-virtual-address counter will have wrapped back to zero. If
     not then we have one more gap to fill in.
  */
  if (info->nextVaddr) {
    const mem_Region above = {.start = 0U};
    makeReserved(info, &above);
  }
}

/* Write fault-producing first-level translation table entries for a gap between regions. */
static void makeReserved(MmuInfo* info, const mem_Region* reg) {
  if (!sectionAligned(info->nextVaddr) || !sectionAligned(reg->start)) {
    dbg_printv("makeReserved(): Address(es) not section-aligned. nextVaddr = %p, region start = %p.\n",
               (void*)info->nextVaddr, (void*)reg->start);
    return;
  }
  while(info->nextVaddr != reg->start) {
    *info->nextSectionTte++ = sectionTte(info->nextVaddr, STTE_TYPE_FAULT << STTE_TYPE_SHIFT);
    info->nextVaddr += SECTION_SIZE;
  }
}

/* Write the first-level translation table entries for a Region without small pages. */
void makeSections(MmuInfo* info, const mem_Region* reg) {
  // For all layouts lay.
  //     Make Props p from lay->flags;
  //     Make section property bits from p.
  //     For all sections in lay.
  //         Make and store tte from vaddr and property bits.
  unsigned sectionsLeft = reg->pages;
  mem_LayoutItem* lay = reg->layout;
  while (sectionsLeft) {
    const unsigned sections = lay->pages ? lay->pages : sectionsLeft;
    PropParts parts;
    flagsToPropParts(lay->flags, &parts);
    const uint32_t propbits = sectionPropertyBits(&parts);
    int s;
    for (s = 0; s < sections; ++s) {
      *info->nextSectionTte++ = sectionTte(info->nextVaddr, propbits);
      info->nextVaddr += SECTION_SIZE;
    }
    ++lay;
    sectionsLeft -= sections;
  }
}

/* For a Region with small pages Write the second-level translation
   table entries together with the first-level entries that refer to
   them.

   IN:
   info->nextPageTte is where the next page TTE will go.
   info->nextVaddr is the first vaddr to be covered by the page tables.

   OUT:
   info->nextPageTte is the first location after the newly created page tables.
   info->nextVaddr is 1 + the last vaddr covered by the new tables.
*/
static void makePageTables(MmuInfo* info, const mem_Region* reg) {
  uint32_t* pageTableAddr = info->nextPageTte; // Save for the second loop.
  /* First make the entries for all the pages in the Region. */
  {
    unsigned pagesLeft = reg->pages;
    mem_LayoutItem* lay = reg->layout;
    while (pagesLeft) {
      const unsigned pages    = lay->pages ? lay->pages : pagesLeft;
      const uint32_t endVaddr = info->nextVaddr + (pages << SMALL_PAGE_SHIFT);
      PropParts parts;
      flagsToPropParts(lay->flags, &parts);
      setPageTable(info->nextPageTte, pagePropertyBits(&parts), info->nextVaddr, endVaddr);
      info->nextPageTte += pages;
      info->nextVaddr   =  endVaddr;
      ++lay;
      pagesLeft         -= pages;
    }
  }

  /* Now make the first-level TTEs that refer to the page tables. We store
     one such entry per section boundary crossed but we have to
     remember that the Region may not have started on a section boundary.
  */
  {
    uint32_t nextVaddr;
    uint32_t const endVaddr = reg->start + (reg->pages << SMALL_PAGE_SHIFT);
    for (nextVaddr = reg->start; nextVaddr != endVaddr; nextVaddr += SMALL_PAGE_SIZE) {
      if (sectionAligned(nextVaddr)) {
        *info->nextSectionTte++ = pageTableRefTte(pageTableAddr);
      }
      ++pageTableAddr;
    }
  }
}





/* Wipe cached TLBs, set Manager access for all access domains.*/
static inline uint32_t openMmu() {
  register uint32_t dacr;
  asm volatile
    (
     "mrc p15, 0, %[old],  c3, c0, 0 \n\t" /* Read the DACR. */
     "mcr p15, 0, %[new],  c3, c0, 0 \n\t" /* Write to the DACR. */
     "isb                            \n\t" /* Wait for the write to take effect. */
     "mcr p15, 0, %[zero], c8, c7, 0 \n\t" /* Invalidate all unlocked TLBs. */
     "dsb"                                 /* Wait for completion. */
     : [old] "=&r"(dacr)
     : [new] "r" (-1), [zero] "r"(0)
     : "memory"
     );
  return dacr;
}

/* Restore the DACR contents saved by openMmu(). */
static inline void closeMmu(uint32_t dacr) {
  asm volatile
    (
     "mcr p15, 0, %[zero], c8, c7, 0 \n\t"  /* Invalidate all unlocked TLBs. */
     "mcr p15, 0, %[zero], c7, c5, 6 \n\t"  /* Invalidate all branch predictions. */
     "mcr p15, 0, %[dacr], c3, c0, 0 \n\t"  /* Restore the old DACR value. */
     "dsb                            \n\t"  /* Wait for invalidations to complete. */
     "isb"                                  /* Wait for DACR update to take effect. */
     :
     : [dacr] "r" (dacr), [zero] "r"(0)
     : "memory"
     );

}

/* Get the TTBR0 register contents. */
static inline uint32_t getTtbr0() {
  register uint32_t ttbr0;
  asm volatile
    (
     "mrc p15, 0, %[ttbr0], c2, c0, 0"
     : [ttbr0] "=r"(ttbr0)
     :
     :
     );
  return ttbr0;
}


/* Get the address, assumed real==virtual, of the first-level MMU translation table. */
static inline uint32_t* getSectionTable() {return (uint32_t*)(getTtbr0() & ~0x3fffU);}

/* If needed alter the existing first-level MMU translation tables and
   create new second-level tables so that the given range of virtual
   addresses has small pages.
*/
int mem_explode(uint32_t startVaddr, uint32_t endVaddr) {
  return mem_explode_core(startVaddr, endVaddr, getSectionTable(), 1, mem_Region_alloc);
}

int mem_explode_core
(uint32_t startVaddr, uint32_t endVaddr, uint32_t* const sectTable, int mmuActive, char* (*alloc)(int, uint32_t)) {
  uint32_t            oldDacr      = 0;
  unsigned            pages        = 0;
  uint32_t*           nextPageTte;
  uint32_t*     const startSectionTte = sectTable + sectionNum(startVaddr);
  uint32_t*           nextSectionTte;
  uint32_t            nextVaddr;
  /* Handle an empty address range. */
  if (startVaddr == endVaddr) return 1;

  /* Align to section boundaries. */
  startVaddr = sectionFloor(startVaddr);
  endVaddr   = sectionCeil(endVaddr);

  /* Don't begin changing the MMU tables unless we know we can
     make all the required page table allocations.
  */
  for (nextVaddr = startVaddr, nextSectionTte = startSectionTte;
       nextVaddr != endVaddr;
       nextVaddr += SECTION_SIZE)
  {
    uint32_t const stte = *nextSectionTte++;
    /* Not already a page table TTE? Then we'll need to create a new page table later. */
    if (! getPageTable(stte)) pages += PAGES_PER_SECTION;
  }

  /* Does the entire address range already have 4K pages? */
  if (pages == 0) return 1;

  /* Allocate space for the new page tables we'll need to create. */
  nextPageTte = (uint32_t*)alloc(MEM_REGION_MMU, pages * sizeof(uint32_t));
  if (!nextPageTte) return 0;

  /* Enable access to write-protected MMU tables. */
  if (mmuActive) oldDacr = openMmu();

  /* Create a page table for each section that hasn't already got one. */
  for (nextVaddr = startVaddr, nextSectionTte = startSectionTte;
       nextVaddr != endVaddr;
       nextVaddr += SECTION_SIZE, nextPageTte += PAGES_PER_SECTION)
  {
    /* We're may be doing this on the active MMU tables therefore we
       must make the page tables first so that the first-level TTEs
       that refer to them will never refer to garbage.
    */
    uint32_t const stte = *nextSectionTte;
    if (! getPageTable(stte)) {
      PropParts parts;
      uint32_t const endSectVaddr = nextVaddr + SECTION_SIZE;
      decodeSectionProperties(&parts, stte);
      setPageTable(nextPageTte, pagePropertyBits(&parts), nextVaddr, endSectVaddr);
      *nextSectionTte++ = sectionTte((uint32_t)nextPageTte, STTE_TYPE_PAGE_TABLE << STTE_TYPE_SHIFT);
    }
  }

  /* Put MMU access protections back into effect. */
  if (mmuActive) closeMmu(oldDacr);
  return 1;
}

/* Alter the exisitng MMU translation tables so that the range of
   virtual addresses given has desired memory attributes.
*/

void mem_setFlags(uint32_t startVaddr, uint32_t endVaddr, int flags) {
  mem_setFlags_core(startVaddr, endVaddr, flags, 1, getSectionTable());
}

void mem_setFlags_core
(uint32_t startVaddr, uint32_t endVaddr, int flags, int mmuActive, uint32_t* const sectionTable) {
  uint32_t*           nextSectionTte;
  uint32_t            nextVaddr;
  uint32_t            nextnext;
  uint32_t            sectProps;
  uint32_t            pageProps;
  uint32_t            oldDacr = 0;
  uint32_t*           pageTable;
  PropParts           parts;

  /* If the new permissions don't allow writing then we have to make
     sure that any stores to the affected memory finish before
     we make the change.
  */
  if (!(flags & MEM_WRITE)) asm volatile("dmb st":::"memory");

  /* Check for various special cases in order to avoid having to test
     inside the loop over sections whether the whole section is
     affected. That check is needed only in the first and last
     sections.
  */
  /* Empty address range? */
  if (startVaddr == endVaddr) return;

  /* There's at least one section affected. */
  if (mmuActive) oldDacr        = openMmu();
  flagsToPropParts(flags, &parts);
  pageProps      = pagePropertyBits(&parts);

  /* Is there only one section affected and does it have small pages? */
  pageTable = getPageTable(sectionTable[sectionNum(startVaddr)]);
  if ((sectionNum(startVaddr) == sectionNum(endVaddr-1U)) && pageTable) {
    /* May have to trim pages both at the front and at the back. */
    setPageTable(pageTable+pageNum(startVaddr), pageProps, pageFloor(startVaddr), pageCeil(endVaddr));
    if (mmuActive) closeMmu(oldDacr);
    return;
  }
  sectProps    = sectionPropertyBits(&parts);

  /* Does the first section affected have small pages? */
  if (pageTable) {
    /* May have to trim pages at the front but not at the back. */
    setPageTable(pageTable + pageNum(startVaddr),
                 pageProps,
                 pageFloor(startVaddr),
                 sectionCeil(startVaddr));
    /* Omit the first section from the loop over sections. */
    startVaddr = sectionCeil(startVaddr);
  }

  /* Does the last section affected have small pages? */
  pageTable = getPageTable(sectionTable[sectionNum(endVaddr - 1U)]);
  if (pageTable) {
    /* May have to trim pages from the end. */
    setPageTable(pageTable, pageProps, sectionFloor(endVaddr - 1U), pageCeil(endVaddr));
    /* Omit the last section from the loop over sections. */
    endVaddr = sectionFloor(endVaddr - 1U);
  }

  /* Loop over all remaining affected sections which we know are all
     *completely* affected. Make sure that the start and end addresses
     are rounded properly if they haven't been already.
  */
  startVaddr = sectionFloor(startVaddr);
  endVaddr   = sectionCeil(endVaddr);
  for (nextVaddr = startVaddr, nextSectionTte = sectionTable + sectionNum(startVaddr);
       nextVaddr != endVaddr;
       nextVaddr = nextnext, ++nextSectionTte)
    {
      uint32_t  const stte      = *nextSectionTte;
      uint32_t* const pageTable = getPageTable(stte);
      nextnext = nextVaddr + SECTION_SIZE;
      if (pageTable) {
        setPageTable(pageTable, pageProps, nextVaddr, nextnext);
      }
      else {
        *nextSectionTte = sectionTte(nextVaddr, sectProps);
      }
    }
  if (mmuActive) closeMmu(oldDacr);
}


typedef enum {RESERVED, SECTION, SMALL_PAGE} PageType;

typedef struct {
  unsigned ap;
  unsigned tex;
  unsigned cb;
  unsigned xn;
  unsigned shared;
  unsigned type;
} Props;

void makeProps(Props* p, unsigned tte, unsigned tableLevel) {
  if ((tableLevel == 1) && ((tte & 2) == 2)) {
    p->type   = SECTION;
    p->ap     = ((tte >> 14)& 2) | ((tte >> 11)&1);
    p->tex    = ( tte >> 12)& 7;
    p->cb     = ( tte >>  2)& 3;
    p->xn     = ( tte >>  4)& 1;
    p->shared = ( tte >> 16)& 1;
  }
  else if ((tableLevel == 2) && ((tte & 2) != 0)) {
    p->type   = SMALL_PAGE;
    p->ap     = ((tte >>  8)& 2) | ((tte >> 5)&1);
    p->tex    = ( tte >>  6)& 7;
    p->cb     = ( tte >>  2)& 3;
    p->xn     = ( tte >>  0)& 1;
    p->shared = ( tte >> 10)& 1;
  }
  else {
    p->type = RESERVED;
    p->ap = p->tex = p->cb = p->xn = p->shared = 0;
  }
}

typedef void (*PrintFunc)(const char* fmt, ...);

static void printProps(const Props* p, PrintFunc print) {
  if (p->type == RESERVED)
    print("Reserved\n");
  else
    print("S %u  AP 0x%1x  TEX 0x%1x  CB 0x%1x  XN %u %s\n",
           p->shared, p->ap, p->tex, p->cb, p->xn, (p->type == SECTION ? "1M": "4K"));
}

static int sameProps(const Props* p, const Props* q) {
  return
    (p->type   == q->type  ) &&
    (p->ap     == q->ap    ) &&
    (p->tex    == q->tex   ) &&
    (p->cb     == q->cb    ) &&
    (p->xn     == q->xn    ) &&
    (p->shared == q->shared);
}

static void printPageTable(unsigned sectionBase, uint32_t sectionTte, Props* prevProps, PrintFunc print) {
  unsigned        const domain = ( sectionTte >>  5)&15;
  const uint32_t* const table  = (const uint32_t*)(sectionTte & 0xfffffc00);
  unsigned i;
  for (i = 0; i < 256U; ++i) {
    unsigned const addrIn  = sectionBase + (i << SMALL_PAGE_SHIFT);
    unsigned const addrOut = table[i] & 0xfffff000;
    Props currentProps;
    if (addrIn != addrOut) {
      print("Virt. != real: 0x%08x->0x%08x", addrIn, addrOut);
      return;
    }
    makeProps(&currentProps, table[i], 2);
    if (!sameProps(&currentProps, prevProps)) {
      print("Addr 0x%08x->0x%08x  Dom %2u ",
            addrIn,
            addrOut,
            domain);
      printProps(&currentProps, print);
      *prevProps = currentProps;
    }
  }
}

void mem_printMmuTable(const uint32_t* const table, PrintFunc print) {
  Props prevProps;
  Props currentProps;
  prevProps.type = 0xffffffff;
  unsigned i;
  for (i = 0; i < 4096U; ++i) {
    if ((table[i] & 3) == 1) {
      printPageTable(i << SECTION_SHIFT, table[i], &prevProps, print);
    }
    else {
      unsigned const addrIn  = i << SECTION_SHIFT;
      unsigned const addrOut = table[i] & 0xfff00000;
      if (addrIn != addrOut) {
        print("Virt. != real: 0x%08x->0x%08x", addrIn, addrOut);
        return;
      }
      makeProps(&currentProps, table[i], 1);
      if (!sameProps(&currentProps, &prevProps)) {
        print("Addr 0x%08x->0x%08x  Dom %2u ",
              addrIn,
              addrOut,
              (unsigned)(table[i] >> 5) & 0xf);
        printProps(&currentProps, print);
        prevProps = currentProps;
      }
    }
  }
}
