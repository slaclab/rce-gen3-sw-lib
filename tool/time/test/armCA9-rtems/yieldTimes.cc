// -*-Mode: C++;-*-
/**
@file
@brief Time context switches by creating a bunch of threads of the same
priority, that priority being the lowest available with class
Thread. On an idle system all the higher-priority threads will be
mostly sleeping so RTEMS will just round-robin these application
threads.

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
2013/10/07

@par Last commit:
\$Date: 2014-04-07 21:30:01 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3125 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/time/test/armCA9-rtems/yieldTimes.cc $

@par Credits:
SLAC
*/



#include <memory>
using std::auto_ptr;

#include "logger/Logger.hh"
using service::logger::Logger;

#include "concurrency/Thread.hh"
using tool::concurrency::Thread;

#include "time/TimerGuard.hh"
using tool::time::TimerGuard;

#include <rtems.h>

namespace {

  // Each yield of the timer thread should be followed by one round
  // of running all the other threads we make here.  As a double-check
  // we'll have each thread increment a global counter. We start
  // counting the first time the timer thread runs; it sets runinc to
  // one. Once the timer thread has performed the required number of
  // yields it will reset runinc. This will give us an accurate count
  // of the number of context switches that take place while the timer
  // thread is running.
  volatile unsigned nruns(0);

  volatile unsigned runinc(0);

  enum {NUM_THREADS = 10, NUM_CYCLES = 1000};

  const Thread::Priority PRIORITY(Thread::APP_PRIO4);



  class YieldingThread: public Thread {
  public:
    YieldingThread();
    virtual ~YieldingThread();
    virtual void body();
    void stop();

  private:
    bool m_stop;
  };


  YieldingThread::YieldingThread(): Thread(), m_stop(false) {
    priority(PRIORITY);
    name("tyld");
    stacksize(minimumStacksize());
  }

  YieldingThread::~YieldingThread() {}

  void YieldingThread::body() {
    while (!m_stop) {nruns += runinc; yield();}
  }

  void YieldingThread::stop() {m_stop = true;}



  class Accumulator {
  public:

    Accumulator(): ticks(0), repeats(0) {}

    void takeTime(unsigned long long ticks, unsigned repeats, unsigned, const char *) {
      this->ticks   += ticks;
      this->repeats += repeats;
    }

    void report() const {
      Logger().info("%llu ticks were required for %u context switches.", this->ticks, this->repeats);
    }

    unsigned long long ticks;
    unsigned repeats;
  };





  class TimingThread: public Thread {
  public:
    TimingThread();
    virtual ~TimingThread();
    virtual void body();
    void report();

  private:
    Accumulator m_accum;
  };


  TimingThread::TimingThread(): Thread() {
    priority(PRIORITY);
    name("ttmr");
    stacksize(minimumStacksize());
  }

  TimingThread::~TimingThread() {}

  void TimingThread::body() {

    nruns = 0;
    runinc = 1; // Make the yield-only threads start counting.
    for (int i = 0; i < NUM_CYCLES; ++i) {
      TimerGuard<Accumulator> tg(m_accum, NUM_THREADS, 0, 0);
      nruns += runinc;
      yield();
    }
    runinc = 0; // Make the yield-only threads stop counting.
  }

  void TimingThread::report() {
    m_accum.report();
  }

} // nameless namespace

static void runTest();

void yieldTimes() {

  Logger().info("");
  Logger().info("Context-switch timing test.");
  runTest();
}

static void runTest() {
  Logger().info("Starting the yield-only threads.");
  auto_ptr<YieldingThread> yielder[NUM_THREADS-1];
  for (int i = 0; i < NUM_THREADS-1; ++i) {
    yielder[i].reset(new YieldingThread());
    yielder[i]->start();
  }

  Logger().info("Starting the timer thread.");
  auto_ptr<TimingThread> timer(new TimingThread());
  timer->start();

  do {Thread::threadSleep(100);} while(timer->isRunning());
  timer->report();

  Logger().info("Shutting down the yield-only threads.");
  for (int i = 0; i < NUM_THREADS-1; ++i) {yielder[i]->stop();}
  bool someRunning;
  do {
    Thread::threadSleep(100);
    someRunning = false;
    for (int i = 0; i < NUM_THREADS-1; ++i) someRunning |= yielder[i]->isRunning();
  } while(someRunning);
  Logger().info("Double-check on number of context switches: %u", nruns);
}
