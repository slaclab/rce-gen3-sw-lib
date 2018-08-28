// -*-Mode: C++;-*-
/*
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

namespace tool {

  namespace concurrency {

    /**
       @class Semaphore
       @brief A binary semaphore (mutex) limited to the values Red
       (taken) and Green (free).

       This class implements a simple binary semaphore.  A semaphore
       belongs to the Thread that last took it and only that Thread
       may subsequently give it.

       There is no protection against priority inversions.  The order
       in which blocked tasks are freed is not guaranteed. Nested
       locking is not supported.

       @warning It's a profound design error to destroy a Semaphore that's
       still taken because it may leave the owning thread holding
       references to the deallocated storage.  The destructor will
       therefore suspend the thread, if any, that owns the semaphore.

       @enum Semaphore::State
       @brief The possible semaphore states.

       @var Semaphore::Red
       @brief The semaphore is taken.

       @var Semaphore::Green
       @brief The semaphore is free.

 
       @fn Semaphore::Semaphore(State initial)
       @brief Set the initial state of the semaphore. Make the
       calling thread the owner of the semaphore if the state is Red.
       @exception Error if the semaphore can't be created.


       @fn Semaphore::~Semaphore
       @brief Destroy the semaphore if it's Green.

       If the Semaphore is still in use it will not be
       destroyed but the Thread attempting to do so
       will be suspended.

       @fn Semaphore::take(unsigned timeout_usec)
       @brief In one atomic operation wait for a semaphore to
       become Green and then make it Red.
       @param[in] timeout_usec The number of microseconds to wait for a Red
       semaphore to become Green, where zero means wait indefinitely.
       @retval true Success. The semaphore is Red and you own the lock.
       @retval false The deadline expired or there was some other problem.
       @exception Error (maybe) if you already own the semaphore.

       If (and only if) the call returns true then the caller has become the
       owner of the semaphore. It is an error to call take() if you already
       own the semaphore; this will cause some random bad thing to happen.

       @fn Semaphore::give()
       @brief In one atomic operation turn a Red semaphore Green or
       leave a Green one unchanged.
       @exception Error if the the semaphore is Red and caller is not
       the owner of the semaphore.
       
     */
  }

}
