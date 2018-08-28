// -*-Mode: C++;-*-
/**
@file
@brief Test the ability of Semaphores and Threads to detect some forms of misuse.
@verbatim
                               Copyright 2012
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

#include <stdio.h>

#include "concurrency/Semaphore.hh"
#include "concurrency/Thread.hh"


namespace tool {

  namespace concurrency {

    namespace {

      /** @brief A thread that deletes a taken Semaphore. */
      class SemTest: public Thread {
      public:

        /** @brief Set thread name to SemTest. */
        SemTest(): Thread() {name("SemTest");}

        /** @brief Default destructor. */
        virtual ~SemTest() {}

        /** @brief The code for the thread. */
        virtual void body() {
          printf("\n");
          printf("Deletion of a taken semaphore.\n");
          {
            Semaphore sem(Semaphore::Green);
            sem.take();
          }
          printf("The misuse was not detected.\n");
        }
      };


      /** @brief A test thread to be destroyed while active. */
      class ThrTest: public Thread {
      public:

        /** @brief While running the thread increments this counter. */
        volatile int* pcounter;

        /** @brief Remember pointer to counter, set thread name to ThrTest. */
        ThrTest(volatile int* pc): Thread(), pcounter(pc) {name("ThrTest");}

        /** @brief default destructor. */
        virtual ~ThrTest() {}

        /** @brief Endless loop: increment counter then yield.*/
        virtual void body() {
          for (;;) {
            ++*pcounter;
            printf("Counter in test thread is now %d.\n", *pcounter);
            yield();
          }
        }
      };

    }

    /** @brief Test whether the deletion of taken Semaphores or active Threads is detected. */
    void testSafety() {
      printf("\n");
      printf("========== Detection of misuses of Semaphore and Thread ==========\n");
      volatile int counter(0);
      {
        printf("Destruction of a Thread that is still running.\n");
        ThrTest thr(&counter);
        thr.start();
        while (counter < 1) {
          Thread::threadYield();
        }
      }
      // The ThrTest thread should have been suspended.
      int c2(counter);
      Thread::threadYield();
      if (counter == c2) {
        printf("The misuse was detected (test thread suspended).\n");
      }
      else {
        printf("The misuse wasn't detected (test thread still running).\n");
      }

      SemTest sem;
      sem.start();
      Thread::threadSleep(1000);

      printf("\n");
      printf("We should now get another Thread destruction warning\n");
      printf("since the SemTest thread never got to terminate.\n");
    }
  }

}
