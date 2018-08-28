// -*-Mode: C++;-*-
/**
@file
@brief "Hello, world" with multiple Threads.
@verbatim
                               Copyright 2012
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/


#include "logger/Logger.hh"
#include "concurrency/Semaphore.hh"
#include "concurrency/SemaphoreGuard.hh"
#include "concurrency/Thread.hh"

namespace {
  using service::logger::Logger;

  /** @brief The test thread that prints "hello" message. */
  class Hello: public tool::concurrency::Thread {
  public:

    /** @brief Taken when the thread runs, released when the thread finishes.*/
    tool::concurrency::Semaphore done;

    /** @brief Identifies the thread writing the message. */
    int id;

    /** @brief Init. "done" flag and ID. */
    Hello(int id)
      : Thread(),
        done(),
        id(id)
    {
      name("child");
    }

    /** @brief Default destructor. */
    virtual ~Hello() {}

    /** @brief The test thread's code. */
    virtual void body() {
      tool::concurrency::SemaphoreGuard wait(done);
      printf("Hello from thread '%s %d'.\n", name(), id);
    }
  };

}

namespace tool {

  namespace concurrency {

    /** @brief Run a simple thread creation test. */
    void testHello() {
      printf("\n");
      printf("========== Test simple thread creation ==========\n");
      printf("Hello from the main thread.\n");
      Hello t1(1);
      Hello t2(2);
      t1.start();
      t2.start();
      // Let the child threads run if they haven't already.
      Thread::threadYield();
      // Make sure that the child threads have stopped.
      SemaphoreGuard(t1.done);
      SemaphoreGuard(t2.done);
    }
  }

}
