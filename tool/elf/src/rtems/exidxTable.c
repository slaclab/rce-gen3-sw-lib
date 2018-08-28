// -*-Mode: C;-*-
/**
@file
@brief RTEMS-dependent part of the EXIDX-tracking table implementation.

This code makes uses of the Region memory allocators which are defined
only for RTEMS. Also, we can't call class Thread's yield operation
since the dynamic linker resides in rtems.so in which C++ code
is not permitted.

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
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2014/05/19

@par Last commit:
\$Date: 2014-06-10 15:41:51 -0700 (Tue, 10 Jun 2014) $ by \$Author: tether $.

@par Credits:
SLAC
*/

#include <inttypes.h>

#include <rtems.h>

#include "debug/print.h"

#include "memory/mem.h"

#include "elf/impl/ExidxTable.h"

#define TABLE_PAGES 1
#define TABLE_REGION MEM_REGION_WORKSPACE

void ExidxTable_init(ExidxTable* table) {
  uint32_t const size = TABLE_PAGES * mem_Region_pageSize(TABLE_REGION);
  if (!(table->start = (Segment*)mem_Region_alloc(TABLE_REGION, size))) {
    dbg_bugcheck("%s %s: Unable to allocate memory for EXIDX tracking.\n", __FILE__, __func__);
  }
  table->next = table->start;
  table->end = table->start + size / sizeof(Segment);
}

void ExidxTable_lock(ExidxTable* table) {
  while (!BasicMutex_trylock(&table->mutex)) rtems_task_wake_after(RTEMS_YIELD_PROCESSOR);
}

void ExidxTable_unlock(ExidxTable* table) {
  BasicMutex_unlock(&table->mutex);
}
