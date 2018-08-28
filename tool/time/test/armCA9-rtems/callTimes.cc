// -*-Mode: C++;-*-
/**
@file
@brief Time various forms of subroutine calls for the ARM Cortex-A9.
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
2013/10/02

@par Last commit:
\$Date: 2014-06-23 16:45:19 -0700 (Mon, 23 Jun 2014) $ by \$Author: tether $.

@par Revision number:
\$Revision: 3411 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/time/test/armCA9-rtems/callTimes.cc $

@par Credits:
SLAC
*/


#include "logger/Logger.hh"
using service::logger::Logger;

#include "time/TimerGuard.hh"
using tool::time::TimerGuard;

#include "interrupt/platform/InterruptGuard.hh"
using tool::interrupt::InterruptGuard;

namespace {

  typedef void SimpleFunc();
  
  // Call+return for a local non-member function with no arguments.
  // The noinline attribute keeps the compiler's optimizer from
  // removing calls to this do-thing function.
  __attribute__((noinline))             void        func1()          {}
  __attribute__((noinline))             SimpleFunc* func1Addr()      {return &func1;}
  __attribute__((always_inline)) inline void        localNonMember() {func1();}
  
  // Call+return for a non-local non-member function with no arguments.
  __attribute__((always_inline)) inline void nonlocalNonMember(SimpleFunc* f) {f();}

  // Call+return for a non-virtual member function of a local object
  // whose only argument is the implicit "this" pointer.
  struct C1 {
    __attribute__((noinline))         void func1() const;
    __attribute__((noinline)) virtual void func2() const;
  };
  void C1::func1() const {}
  void C1::func2() const {}
  C1 obj1;
  __attribute__((always_inline)) inline void localMember() {obj1.func1();}

  // Call+return for a non-virtual member function of a non-local object
  // whose only argument is the implicit "this" pointer.
   __attribute__((always_inline)) inline void nonlocalMember(const C1*const obj) {obj->func1();}

  // Call+return for a virtual member function of a local object
  // whose only argument is the implicit "this" pointer.
  __attribute__((always_inline)) inline void localVmember() {obj1.func2();}

  // Call+return for a virtual member function of a non-local object
  // whose only argument is the implicit "this" pointer.
  __attribute__((always_inline)) inline void nonlocalVmember(const C1*const obj) {obj->func2();}




  enum {LNM, NLNM, LM, NLM, LVM, NLVM, NUM_TESTS}; // Test IDs.

  class Accumulator {
  public:
    void takeTime(unsigned long long ticks, unsigned repeats, unsigned id, const char *comment) {
      test[id].ticks   = ticks;
      test[id].repeats = repeats;
      test[id].comment = comment;
    }

    void report() const {
      for (int i = 0; i < 2*NUM_TESTS; ++i) {
	Logger().info("%ll6u ticks for %4d %s", test[i].ticks, test[i].repeats, test[i].comment);
      }
    }

    Accumulator() {
      for (int i = 0; i < 2*NUM_TESTS; ++i) {
	test[i].ticks   =  0;
        test[i].repeats =  1;
        test[i].comment = "UNUSED TEST SLOT";
      }
    }

    struct {unsigned long long ticks; unsigned repeats; const char* comment;} test[2*NUM_TESTS];
  };

} // nameless namespace


void callTimes() {
  enum {NUM_CALLS = 100};

  Logger().info("");
  Logger().info("Function-call timing tests. Uncached then cached times are printed for each test.");
  Accumulator accum;
  for (int i = 0; i < 2; ++i) {
    InterruptGuard ig;
    TimerGuard<Accumulator> tg(accum, NUM_CALLS, 2*LNM+i, "calls to local non-member func");
    asm volatile("nop":::"memory");
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    localNonMember();
    asm volatile("nop":::"memory");
  }
  for (int i = 0; i < 2; ++i) {
    SimpleFunc* const f(func1Addr());
    InterruptGuard ig;
    TimerGuard<Accumulator> tg(accum, NUM_CALLS, 2*NLNM+i, "calls to non-local non-member func");
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
    nonlocalNonMember(f);
  }
  for (int i = 0; i < 2; ++i) {
    InterruptGuard ig;
    TimerGuard<Accumulator> tg(accum, NUM_CALLS, 2*LM+i, "calls to member func. of local object");
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
    localMember();
  }
  const C1* const obj(new C1);
  for (int i = 0; i < 2; ++i) {
    InterruptGuard ig;
    TimerGuard<Accumulator> tg(accum, NUM_CALLS, 2*NLM+i, "calls to member func. of non-local object");
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
    nonlocalMember(obj);
  }
  for (int i = 0; i < 2; ++i) {
    InterruptGuard ig;
    TimerGuard<Accumulator> tg(accum, NUM_CALLS, 2*LVM+i, "calls to virt. member func. of local object");
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
    localVmember();
  }
  for (int i = 0; i < 2; ++i) {
    InterruptGuard ig;
    TimerGuard<Accumulator> tg(accum, NUM_CALLS, 2*NLVM+i, "calls to virt. member func. of non-local object");
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
    nonlocalVmember(obj);
  }

  accum.report();
}
