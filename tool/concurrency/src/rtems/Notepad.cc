// -*-Mode: C++;-*-
/**
@file
@brief Documentation and non-inline implementation of class Notepad for RTEMS.
@verbatim
                               Copyright 2011
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

#include "concurrency/Notepad.hh"


namespace tool {

  namespace concurrency {

    /** @class Notepad

        We take advantage of the 16 notepad slots built into each Task Control
        Block. Since the pointer-to-current-TCB must be reset anyway for every
        context switch we get 16 words of task-local storage with no extra
        overhead per switch.

        FYI RTEMS also has a more general implementation of task-local storage
        using global variables outside the TCB but it incurs an overhead per
        context switch proportional to the number of task-local variables
        used.  It uses a linked list of address-value pairs. On a context
        switch the scheduler must walk down the list for the old task and copy
        the values at the given addresses into the list nodes. Then it must
        walk the list for the new thread and copy the values from the nodes to
        the addresses.
    */

    /** @brief Private static resources for class Notepad under RTEMS. */
    namespace notepad_private {
        
      /** @brief Which notepad slots are in use? */
      bool isAllocated[NUM_NOTEPADS];
        
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
