// -*-Mode: C;-*-
/**
@file
@brief Declare the type ExidxTable.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#if !defined(TOOL_ELF_EXIDXTABLE_H)
#define      TOOL_ELF_EXIDXTABLE_H

#include "concurrency/BasicMutex.h"


// Unsigned value guaranteed to be the same size as a pointer, unlike
// uintptr_t which may be larger than needed. This matches
// the declaration used by the unwind library.
typedef unsigned UnwindPtr __attribute__((mode(__pointer__)));

// Each loadable, executable segment occupies addresses [segStart,
// segEnd) and has an EXIDX table with numExidxEntries starting at
// address exidxStart.
typedef struct {
  UnwindPtr segStart, segEnd, exidxStart;
  int numExidxEntries;
} Segment;

// Table associating segments with EXIDX tables.
typedef
struct {
  BasicMutex mutex;   // Mutex which protects the table.
  Segment* start;     // Pointer to first table entry.
  Segment* end;       // Pointer just after last allocated table entry.
  Segment* next;      // Pointer to next unused table entry.
} ExidxTable;

#if defined(__cplusplus)
extern "C" {
#endif

// Platform-dependent table initialization.
void ExidxTable_init(ExidxTable*);

// Platform-dependent synchonization.
void ExidxTable_lock   (ExidxTable*);
void ExidxTable_unlock (ExidxTable*);

#if defined(__cplusplus)
}
#endif

#endif
