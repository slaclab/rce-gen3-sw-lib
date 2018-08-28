// -*-Mode: C;-*-
/**
@file
@brief Linux implementation of the EXIDX-tracking table.


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

#include <stdlib.h>

#include "debug/print.h"

#include "elf/impl/ExidxTable.h"

#include "concurrency/Thread.hh"
using tool::concurrency::Thread;

#define TABLE_SIZE 200


extern "C" void ExidxTable_init(ExidxTable* table) {
  uint32_t const size = TABLE_SIZE * sizeof(Segment);
  if (!(table->start = (Segment*)malloc(size))) {
    dbg_bugcheck("%s %s: Unable to allocate memory for EXIDX tracking.\n", __FILE__, __func__);
  }
  table->next = table->start;
  table->end = table->start + TABLE_SIZE;
}

extern "C" void ExidxTable_lock(ExidxTable* table) {
  while (!BasicMutex_trylock(&table->mutex)) Thread::threadYield();
}

extern "C" void ExidxTable_unlock(ExidxTable* table) {
  BasicMutex_unlock(&table->mutex);
}
