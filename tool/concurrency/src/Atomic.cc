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
       @class Atomic
       @brief Simple atomic operations on an int counter.

        "Atomic" means that an operation won't return until it has
        been carried out without interference from other threads,
        interrupts or other processors. It might be done by detecting
        interference and retrying or by locking out sources of
        interference.

        Implemented using GNU atomic builtins.
 
        @fn Atomic::Atomic()
        @brief Set the counter to zero.

        @fn Atomic::count() const
        @brief The value of the counter.
        @return The current value of the counter.

        @fn Atomic::add(int addend)
        @brief Add to the counter.
        @param[in] addend The addend.
        @return The new value of the counter.

        @fn Atomic::inc()
        @brief Add +1 to the counter.
        @return The new value of the counter.

        @fn Atomic::dec()
        @brief Add -1 to the counter.
        @return The new value of the counter.

        @var Atomic::m_counter
        @brief The current value of the counter.
        
     */
  }

}
