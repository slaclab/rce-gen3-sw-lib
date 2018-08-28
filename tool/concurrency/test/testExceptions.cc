// -*-Mode: C++;-*-
/**
@file
@brief Implements the exception throwing test for threads.
@verbatim
                               Copyright 2012
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/


#include "exception/Exception.hh"

#include "concurrency/Thread.hh"

namespace tool {

  namespace concurrency {

    using tool::exception::Error;

    namespace {

      /** @brief A test thread that throws exceptions.
          @details The first exception should be caught by the thread,
          the second should not - instead the second will be caught
          by Thread support code.
      */
      class Thrower: public Thread {
      public:

        /** @brief The test thread's code. */
        virtual void body() {
          printf("\n");
          printf("========== Throw/catch test ==========\n");
          printf("About to throw Error('First exception.') which ought to be caught in the thread.\n");
          try {
            throw Error("First exception.");
          }
          catch(Error& exc) {
            printf("Caught Error in thread. Message: %s\n", exc.what());
          }
          printf("\n");
          printf("Next we'll throw Error('Second exception.') which the thread doesn't catch.\n");
          printf("The Thread support code should catch it, print a couple of\n"
                 "FATAL messages and suspend the thread.\n");
          throw Error("Second exception.");
        }
      };
    }

    /** @brief Create and start the exception-throwing test thread.*/
    void testExceptions() {
      Thrower thr;
      thr.name("Thrower");
      thr.start();
      Thread::threadYield();
      while(!thr.isFinished()) {
        thr.resume();
        Thread::threadYield();
      }
    }

  }
}
