// -*-Mode: C++;-*-
/**
@file
@brief Documentation and non-inline implementation of class Notepad for Linux.
@cond development
@verbatim
                               Copyright 2012
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

#include "pth.h"

#include "concurrency/Notepad.hh"
#include "concurrency/Semaphore.hh"

namespace tool {

  namespace concurrency {


    /**

    @class Notepad
    @details Unlike in RTEMS we don't have 16 predefined thread-local
    storage slots available in the equivalent of the Task Control
    Block. Therefore when we allocate a Notepad slot we have to tell
    the threading library Pth to create a new thread-local variable
    whose identity key we must save for use with each get() and
    put(). That variable's storage is potentially usable by any thread
    which knows the key; in this case that means any thread which can
    use the Notepad instance. We have to destroy the thread-local
    variable when the Notepad instance is destroyed.

    @def PTH_CALL
    @brief Call a Pth function, test the return code and throw Error if needed.

    */

    /** @brief Private static resources for class Notepad under Linux. */
    namespace notepad_private {
      
      /** @brief Which notepad slots are in use? */
      bool isAllocated[NUM_NOTEPADS];
      
      /** @brief Thread-local storage keys for each allocated slot. */
      pth_key_t key[NUM_NOTEPADS];
      
      /** @brief Used to synchronize changes to isAllocated[] and key[]. */
      Semaphore* lock;

      /** @brief Used to ensure that init() is called only once. */
      Once initOnce;

      /** @brief Create the Semaphore, set all slots to the unallocated state. */
      void init() {
        lock = new Semaphore(Semaphore::Green);
        for (int i = 0; i < NUM_NOTEPADS; ++i) isAllocated[i] = false;
      }

    } // notepad_private

  } // concurrency
} // tool

/** @endcond */
