// -*-Mode: C++;-*-
/**
@file
@brief Declares the Semaphore class.
@verbatim
                              Copyright 2010
                                    by
                       The Board of Trustees of the
                     Leland Stanford Junior University.
                            All rights reserved.
@endverbatim
*/
#if !defined(TOOL_CONCURRENCY_SEMAPHORE_HH)
#define      TOOL_CONCURRENCY_SEMAPHORE_HH


#include "concurrency/os/SemaphoreData.hh"

namespace tool {

  namespace concurrency {


    class Semaphore: private SemaphoreData {
    public:
      enum State {Red, Green};

      explicit Semaphore(State initial=Green);

      ~Semaphore();

      bool take(unsigned timeout_usec = 0);

      void give();

    };


  } // concurrency

} // tool

#endif // TOOL_CONCURRENCY_SEMAPHORE_HH
