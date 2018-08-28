// -*-Mode: C++;-*-
/**
@file
@brief Run timing tests of cache flushing and memory attribute-probing functions.
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
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2013/10/04

@par Last commit:
\$Date: 2014-04-07 21:30:01 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3125 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/time/test/armCA9-rtems/memopTimes.cc $

@par Credits:
SLAC

*/



#include "interrupt/platform/InterruptGuard.hh"

#include "time/TimerGuard.hh"

#include "logger/Logger.hh"

#include "memory/mem.h"

namespace {

  struct MyAccum {
    unsigned long long ticks;
    void takeTime(unsigned long long t, unsigned, unsigned, const char *) {
      ticks = t;
    }
  };
}


using tool::interrupt::InterruptGuard;
using tool::time::TimerGuard;
using service::logger::Logger;

unsigned timeCacheFlushes();
unsigned timePhysWrite();
unsigned timeFlagsWrite();
unsigned timePhysFlagsWrite();

void memopTimes() {
  Logger().info("");
  Logger().info("Timing of 'mem' library routines from tool/memory.");
  Logger().info("100 calls to mem_StoreDataCacheRange(), storing 32,000 bytes each, take %u ticks.", timeCacheFlushes());
  Logger().info("100 calls to mem_getPhysWrite()      take %u ticks.", timePhysWrite());
  Logger().info("100 calls to mem_getFlagsWrite()     take %u ticks.", timeFlagsWrite());
  Logger().info("100 calls to mem_getPhysFlagsWrite() take %u ticks.", timePhysFlagsWrite());
}


static char buffer[1000*32]; // About 1000 cache lines.
const uint32_t buffStart(reinterpret_cast<uint32_t>(buffer));
const uint32_t buffEnd  (buffStart + sizeof buffer);

unsigned timeCacheFlushes() {
  // Make two repetitions so that the second time around the
  // instructions are all in the I-cache.
  MyAccum accum;
  uint32_t l2base(mem_mapL2Controller());
  for (int i = 0; i < 2; ++i) {
    InterruptGuard ig;
    TimerGuard<MyAccum> tg(accum, 1, 1, "");
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
    mem_storeDataCacheRange(buffStart, buffEnd, l2base);
  }
  mem_unmapL2Controller();
  static const unsigned overhead = 25U;
  return accum.ticks - overhead;
}



unsigned timePhysWrite() {
  MyAccum accum;
  for (int i = 0; i < 2; ++i) {
    InterruptGuard ig;
    TimerGuard<MyAccum> tg(accum, 1, 1, "");
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
    mem_getPhysWrite(buffStart);
  }
  static const unsigned overhead = 25U;
  return accum.ticks - overhead;
}



unsigned timeFlagsWrite() {
  MyAccum accum;
  for (int i = 0; i < 2; ++i) {
    InterruptGuard ig;
    TimerGuard<MyAccum> tg(accum, 1, 1, "");
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
    mem_getFlagsWrite(buffStart);
  }
  static const unsigned overhead = 25U;
  return accum.ticks - overhead;
}



unsigned timePhysFlagsWrite() {
  MyAccum accum;
  for (int i = 0; i < 2; ++i) {
    int flags;
    InterruptGuard ig;
    TimerGuard<MyAccum> tg(accum, 1, 1, "");
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
    mem_getPhysFlagsWrite(buffStart, &flags);
  }
  static const unsigned overhead = 25U;
  return accum.ticks - overhead;
}
