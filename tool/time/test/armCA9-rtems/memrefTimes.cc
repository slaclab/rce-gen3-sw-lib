// -*-Mode: C++;-*-
/**
@file
@brief Run timing tests of nops, fetches and stores.
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
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/time/test/armCA9-rtems/memrefTimes.cc $

@par Credits:
SLAC

*/



#include "interrupt/platform/InterruptGuard.hh"

#include "time/TimerGuard.hh"

#include "logger/Logger.hh"


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

unsigned time100Nops();
unsigned time100Fetches(volatile unsigned*);
unsigned time100Stores(volatile unsigned*, volatile unsigned*);

void memrefTimes() {
  Logger().info("");
  Logger().info("Nop/fetch/store timing test.");
  Logger().info("100 cached nops take %u ticks.", time100Nops());
  // The Cortex-A9 tecnical reference states that the processor core
  // can dispatch two single-word loads or stores at once, presumably if they
  // don't depend on one another.
  unsigned u[32]; // Spans multiple cache lines.
  Logger().info("100 cached word fetches, alternating registers, from the same address take %u ticks.", time100Fetches(u));
  Logger().info("100 cached word stores, alternating memory addresses, take %u ticks.", time100Stores(u, u+31));
}



unsigned time100Nops() {
  // Make two repetitions so that the second time around the
  // instructions are all in the I-cache.
  MyAccum accum;
  for (int i = 0; i < 2; ++i) {
    InterruptGuard ig;
    TimerGuard<MyAccum> tg(accum, 1, 1, "");
    asm volatile
      (
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       "nop \n\t"
       :
       :
       :
       );
  }
  static const unsigned overhead = 25U;
  return accum.ticks - overhead;
}



unsigned time100Fetches(volatile unsigned *u) {
  MyAccum accum;
  for (int i = 0; i < 2; ++i) {
    register unsigned tmp1(0), tmp2(0);
    register volatile unsigned *uptr = u;
    InterruptGuard ig;
    TimerGuard<MyAccum> tg(accum, 1, 1, "");
    asm volatile
      (
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       "ldr %[tmp1],[%[uptr]] \n\t"
       "ldr %[tmp2],[%[uptr]] \n\t"
       : [tmp1]"=&r"(tmp1), [tmp2]"=&r"(tmp2)
       : [uptr]"r"(uptr)
       :
       );
  }
  static const unsigned overhead = 25U;
  return accum.ticks - overhead;
}



unsigned time100Stores(volatile unsigned *u1, volatile unsigned *u2) {
  // Make two repetitions so that the second time around the
  // instructions are all in the I-cache.
  MyAccum accum;
  for (int i = 0; i < 2; ++i) {
    register unsigned tmp(0);
    register volatile unsigned *uptr1(u1), *uptr2(u2);
    InterruptGuard ig;
    TimerGuard<MyAccum> tg(accum, 1, 1, "");
    asm volatile
      (
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       "str %[tmp],[%[uptr1]] \n\t"
       "str %[tmp],[%[uptr2]] \n\t"
       :
       : [tmp]"r"(tmp), [uptr1]"r"(uptr1), [uptr2]"r"(uptr2)
       :
       );
  }
  static const unsigned overhead = 25U;
  return accum.ticks - overhead;
}
