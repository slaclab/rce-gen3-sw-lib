// -*-Mode: C++;-*-
/*
                               Copyright 2010
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
*/

namespace tool {

  namespace concurrency {

    /**
       @class SemaphoreGuard
       @brief Take a Semaphore on construction, give it back on destruction. Non-copyable.

       Creates a critical section using the Resource Allocation Is
       Initialization strategy.  A block of code that needs to execute
       while holding a Semaphore constructs a SemaphoreGuard object with
       the Semaphore, which is immediately taken.  The Semaphore is given
       back when the SemaphoreGuard object is destroyed so you don't
       have to remember to give the Semaphore back yourself. This
       technique automatically handles nested critical sections correctly
       since the C++ destructors are called in the order opposite that of
       the corresponding constructors (the Last Shall Be First rule).
       Examples:
       @code
       void Foo::func() {
         ...
         SemaphoreGuard cs(globalSem);
         // The remaining function body is a critical section.
         ...
       }

       void Spam::sausage() {
         ...
         Semaphore lock(Semaphore::Green);
         startChildTask(lock);
         ...
         { SemaphoreGuard cs(lock);
           // This block is a critical section.
           ...
         }
       }
       @endcode

       @fn SemaphoreGuard::SemaphoreGuard(Semaphore &lock)
       @brief Take the Semaphore and store a reference to it.
       @param[in,out] lock The semaphore to take.

       @fn SemaphoreGuard::~SemaphoreGuard()
       @brief Give the semaphore that was passed to the constructor.

       @var SemaphoreGuard::_lock
       @brief A reference to the semaphore to use.
       
     */

  }

}
