// -*-Mode: C++;-*-
/**
@file
@brief Implements the test driver function for the package.
@verbatim
                               Copyright 2012
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
2012/11/29

@par Credits:
SLAC
*/

#include "concurrency/Atomic.hh" // Make sure header compiles.

namespace tool {

  namespace concurrency {

    void testHello();
    void testDining();
    void testBasicMutex();
    void testExceptions();
    void testSafety();

    /** @brief The test driver function for the concurrency package. */
    void testConcurrency() {
      Atomic aint; // Make sure Atomic can be instantiated.

      testHello();
      testDining();
      testExceptions();
      testSafety();
      testBasicMutex();
    }
  }

}
