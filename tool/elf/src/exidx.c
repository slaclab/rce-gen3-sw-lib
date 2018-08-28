// -*-Mode: C;-*-
/**
@file
@brief Help the unwind library find the right ARM EXIDX table.

The basic unwind library that comes with the compiler (libgcc)
contains the function get_eit_entry() whose job it is to find the
right EXTAB entry given the return address of the function call which
is throwing. To do this it first needs to find the EXIDX for the code
object that contains the throw. If the function
__gnu_Unwind_Find_exidx() is not present (weak reference == 0)
get_eit_entry() uses the default method of relying on linker-defined
symbols: the EXIDX starts at __exidx_start and ends at the byte just
before __exidx_end. The libgcc.a used with RTEMS lacks
__gnu_Unwind_Find_exidx() and so the default method is used.

The default method won't suffice when you have multiple shared objects.
get_eit_entry() can only use the values of __exidx_start and
__exidx__end that are local to the whatever shared object it's
inside. Yet the handler for an exception may well lie in another
object altogether. A way is needed to search for the handler in all
the EXIDX tables present in the system.

A solution is to provide rtems.so with a custom implementation of
__gnu_Unwind_Find_exidx() in the ELF library. It searches a table
maintained by the dynamic linker in which each entry contains the
range of addresses used by a loadable, executable segment together
with the location and number of entries of the EXIDX for the
corresponding image. If no segment matches then rtems.so's linker
symbols will be used to determine the EXIDX location and number of
entries. Each entry is eight bytes long.

This solution works because the source associated with an exception
changes when the stack frame under consideration changes. For example
suppose foo() calls bar() and then bar() throws. At first the unwinder
considers the frame for bar() with the exception source being the call
to __cxa_throw(). The unwinder therefore searches the EXIDX for the
code object containing that call. If nothing is found then the
unwinder considers the frame for foo(), changing the exception source
to the call to bar(). Now another EXIDX search is performed but in the
table for the code object that contains the call to bar(). If foo()
contains a handler for the exception then this second search will find
it.

Our system may have multiple copies of an image in memory and
therefore may have duplicate entries of its segments in the segment
table searched by __gnu_Unwind_Find_exidx(). This doesn't matter since
the call stack unwinding process ensures that only function calls with
frames on the stack will be considered as exception sources. Looking
at it another way, the code segments of different copies never overlap
in memory so there's no danger of getting them confused.

@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
DAT

@author
Steve Tether<tether@slac.stanford.edu>

@par Date created:
2013-04-15

@par Last commit:
\$Date: 2014-06-10 15:41:51 -0700 (Tue, 10 Jun 2014) $ by \$Author: tether $.

@par Revision number:
\$Revision: 3382 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/elf/src/exidx.c $

@par Credits:
SLAC
*/
#include <stdio.h>

#include "debug/print.h"

#include "elf/baseTypes.h"
#include "elf/Object.h"

#include "elf/impl/ExidxTable.h"

static inline void debugPrint1(const Segment* seg);
static inline void debugPrint2(UnwindPtr pc);
static inline void debugPrint3(const Segment* seg);
static inline void debugPrint4(void);
static inline void debugPrint5(void);

static void addSegment(const Segment* seg);

static const Segment* findSegment(UnwindPtr pc);

// Record all loadable, executable segments if there is a PT_EXIDX
// segment.
void rememberExidx(const elf_Object* elfObj) {
  // Is there an EXIDX segment?
  const char* const exidx      = elfObj->exidx;
  // If there is then record the segments that it may refer to.
  if (exidx) {
    const unsigned     const numEntries = (elfObj->exidxSize) >> 3;
    const elf_Pheader* const headers    = elfObj->phdrtab;
    const elf_Pheader* const end        = headers + elfObj->numPhdr;
    const elf_Pheader*       hdr;
    for (hdr = headers; hdr != end; ++hdr) {
      if (elf_Pheader_isLoadable(hdr) && elf_Pheader_isExecutable(hdr)) {
        Segment seg;
        seg.segStart        = (UnwindPtr)elf_Pheader_segAddr(hdr, elfObj->baseAddress);
        seg.segEnd          = seg.segStart + elf_Pheader_memSize(hdr);
        seg.exidxStart      = (UnwindPtr)exidx;
        seg.numExidxEntries = numEntries;
        addSegment(&seg);
      }
    }
  }
}

// The two linker script symbols that we fall back upon. For this to
// work this code must be in rtems.so with the unwind library.
extern char __exidx_start[0] __attribute__((weak));
extern char __exidx_end[0]   __attribute__((weak));

/** @brief Find the right EXIDX table for the given source of an
    exception.

    @param[in] pc The return address for the function call that is
    currently considered to be the source of the exception.

    @param[out] pcount The number of entries in the EXIDX table found.

    @return The address of the EXIDX table found.
*/
UnwindPtr __gnu_Unwind_Find_exidx (UnwindPtr pc, int* pcount) {
  const Segment* const seg = findSegment(pc);
  // Return the information on the EXIDX table.
  if (seg) {
    // Found a module segment that contains the call.
    *pcount = seg->numExidxEntries;
    return seg->exidxStart;
  }
  else {
    // No matching module segment.
    *pcount = (__exidx_end - __exidx_start) >> 3;
    return (UnwindPtr)(__exidx_start);
  }
}

static ExidxTable exidxTable = {
  .mutex = BASIC_MUTEX_INITIALIZER,
  .start = NULL,
  .end   = NULL,
  .next  = NULL
};

static void addSegment(const Segment* seg) {
  ExidxTable_lock(&exidxTable);
  if (!exidxTable.start) ExidxTable_init(&exidxTable);
  if (exidxTable.next >= exidxTable.end) {
    ExidxTable_unlock(&exidxTable);
    const char* const warning = "WARNING! Out of space for tracking EXIDX tables. Time to reboot!\n";
    dbg_prints(warning);
    printf(warning);
  }
  else {
    *exidxTable.next++ = *seg;
    ExidxTable_unlock(&exidxTable);
    debugPrint1(seg);
  }
}

static const Segment* findSegment(UnwindPtr pc) {
  debugPrint2(pc);
  ExidxTable_lock(&exidxTable);
  if (!exidxTable.start) ExidxTable_init(&exidxTable);
  const Segment* const next = exidxTable.next;
  ExidxTable_unlock(&exidxTable);
  const Segment* seg;
  for (seg = exidxTable.start; seg != next; ++seg) {
    debugPrint3(seg);
    if (seg->segStart <= pc && pc < seg->segEnd) {
      debugPrint4();
      return seg;
    }
    debugPrint5();
  }
  return NULL;
}



static inline void debugPrint1(const Segment* seg) {
  /* dbg_printv("Added segment: start %08x  end %08x  exidx %08x  entries %d\n", */
  /*            seg->segStart, */
  /*            seg->segEnd, */
  /*            seg->exidxStart, */
  /*            seg->numExidxEntries); */
}

static inline void debugPrint2(UnwindPtr pc) {
  /* dbg_printv("Finding segment for exception source at %08x\n", pc); */
}

static inline void debugPrint3(const Segment* seg) {
  /* dbg_printv("Checking segment: start %08x  end %08x  exidx %08x  entries %d\n", */
  /*            seg->segStart, */
  /*            seg->segEnd, */
  /*            seg->exidxStart, */
  /*            seg->numExidxEntries); */
}

static inline void debugPrint4(void) {
  /* dbg_prints("Match.\n"); */
}

static inline void debugPrint5(void) {
  /* dbg_prints("No match.\n"); */
}
