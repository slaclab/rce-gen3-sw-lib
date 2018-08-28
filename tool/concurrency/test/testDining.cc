// -*-Mode: C++;-*-
/**
@file
@brief Implement testDining().
@verbatim
                               Copyright 2012
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/


#include "concurrency/Semaphore.hh"
#include "concurrency/Thread.hh"

#include "Philosopher.hh"

namespace {

  /** @brief Implement a chopstick using a Semaphore. */
  class SemChop: public Philosopher::Chopstick {

    /** @brief The lock variable. */
    tool::concurrency::Semaphore m_lock;

  public:
    /** @brief Set the chopstick state to "unused" (unlocked). */
    SemChop(): m_lock() {}

    /** @brief Set the chopstick state to "in use" (locked)
        as soon as it become available. */
    void pickUp()  {m_lock.take();}

    /** @brief Set the chopstick state to "unused" (unlocked). */
    void putDown() {m_lock.give();}

    /** @brief The required virtual destructor. */
    virtual ~SemChop() {}

    /** @brief Return a short C-string describing the implementation. */
    static const char* implementation() {return "Semaphore";}
  };
}
  

      
namespace tool {

  namespace concurrency {

    /** @brief Run the Dining Philosophers test using Semaphore objects. */
    void testDining() {testDiningSkeleton<SemChop, 5>();}

  }
}
