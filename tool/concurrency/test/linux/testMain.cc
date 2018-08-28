// -*-Mode: C++;-*-
/**
@file
@brief Linux main program to call the test driver.
@verbatim
                               Copyright 2012
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/


#include "concurrency/Thread.hh"

namespace tool {

  namespace concurrency {

    void testConcurrency();
  }

}

using tool::concurrency::Thread;


/** @brief A Thread used to run the test code. */
class Main: public Thread {
public:

  /** @brief Init. the base class. */
  Main() : Thread(false) {}

  /** @brief Call the test's driver function. */
  virtual void body() {
    tool::concurrency::testConcurrency();
  }

  /** @brief default destructor. */
  virtual ~Main() {}
};


/** @brief Main program for Linux. */
int main() {
  Main tmain;
  tmain.graduate();
  return 0;
}
