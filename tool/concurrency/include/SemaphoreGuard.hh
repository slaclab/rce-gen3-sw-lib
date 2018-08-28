// -*-Mode: C++;-*-
/**
@file
@verbatim
                               Copyright 2010
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

#if !defined(TOOL_CONCURRENCY_SEMAPHOREGUARD_HH)
#define      TOOL_CONCURRENCY_SEMAPHOREGUARD_HH


#include "concurrency/Semaphore.hh"

namespace tool {

  namespace concurrency {

    class SemaphoreGuard {

      Semaphore &_lock;

    public:

      SemaphoreGuard(Semaphore &lock) : _lock(lock) {_lock.take();}

      ~SemaphoreGuard() {_lock.give();}
    };

  }; // concurrency

}; // tool

#endif // TOOL_CONCURRENCY_SEMAPHOREGUARD_HH
