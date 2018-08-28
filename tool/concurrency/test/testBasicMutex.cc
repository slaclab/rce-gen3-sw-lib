// -*-Mode: C++;-*-
/**
@file
@brief Implement testBasicMutex().
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/


#include "concurrency/BasicMutex.h"
#include "concurrency/Thread.hh"

#include "Philosopher.hh"


namespace {

  /** @brief Implement a chopstick using a BasicMutex. */
  class BasicChop: public Philosopher::Chopstick {
    /** @brief The lock variable. */
    BasicMutex m_lock;

  public:
    /** @brief Set the chopstick state to "unused" (unlocked). */
    BasicChop() {BasicMutex_init(&m_lock);}

    /** @brief Set the chopstick state to "in use" (locked) as
        soon as it becomes available. */
    void pickUp()  {while (!BasicMutex_trylock(&m_lock)) tool::concurrency::Thread::threadYield();}

    /** @brief Set the chopstick state to "unused" (unlocked). */
    void putDown() {BasicMutex_unlock(&m_lock);}

    /** @brief The required virtual destructor. */
    virtual ~BasicChop() {}

    /** @brief Return a short C-string describing the implementation. */
    static const char* implementation() {return "BasicMutex";};
  };
}


      
namespace tool {

  namespace concurrency {

    /** @brief Run the Dining Philosophers test using BasicMutex objects. */
    void testBasicMutex() {testDiningSkeleton<BasicChop, 5>();}

  }
}



