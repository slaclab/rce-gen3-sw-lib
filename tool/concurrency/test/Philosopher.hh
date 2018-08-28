// -*-Mode: C++;-*-
/**
@file
@brief Declare the test class Philosopher.

This file is intended for test code only and is not for general
use.

@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#if !defined(TOOL_CONCURRENCY_PHILOSOPHER_HH)
#define      TOOL_CONCURRENCY_PHILOSOPHER_HH

#include <stdio.h>

#include <vector>
#include <tr1/memory>

#include "concurrency/Thread.hh"


using tool::concurrency::Thread;

class Philosopher;

typedef std::tr1::shared_ptr<Philosopher> PhilPtr;

class Philosopher: public Thread {

public:
    
  class Chopstick {
  public:
    virtual void pickUp()  = 0;
    virtual void putDown() = 0;
    virtual ~Chopstick() {}
  };

  class State {
    const char*  m_string;

    static const char* const m_thinking;
    static const char* const m_hungry;
    static const char* const m_eating;

  public:
    State();
    State& thinking();
    State& hungry();
    State& eating();
    bool isThinking() const;
    bool isHungry() const;
    bool isEating() const;
    const char* string() const;
  };

private:
  Chopstick* m_left;     ///< @brief The chopstick on the left.
  Chopstick* m_right;    ///< @brief The chopstick on the right.
  int        m_id;       ///< @brief ID number.
  int        m_thinkTime;///< @brief How long to spend thinking, in seconds.
  State      m_state;    ///< @brief Current state.
  bool       m_haveEaten;  ///< @brief Have we ever eaten?


  static std::vector<PhilPtr>*       m_diners;        ///< @brief Pointer to vector of diners.
  static int                         m_numDiners;     ///< @brief No. of diners.
  static volatile bool               m_dinnerTime;    ///< @brief Terminate when this is false.
  static volatile int                m_dinersLock;    ///< @brief Synchronizes state changes.

public:

  static void setDiners(std::vector<PhilPtr>& diners);

  static void printStates();

  static bool checkAdjacents(int id);

  static bool checkStarvation();

  static void dinnerIsOver();

public:

  Philosopher(Chopstick& left, Chopstick& right, int id, int thinkTime);

  State state() const;

  bool hasEaten() const;

  void setState(State newState);

  virtual void body();

  virtual ~Philosopher() {}
};



/** @brief Skeleton code for the Dining Philosophers simulation driver.
    @tparam Sync The type of object used to syncronize access to chopsticks.
    Must be a class derived from Philosopher::Chopstick.
    Must implement a static member function implementation()
    which returns a C string naming the kind of implementation.
    @tparam numDiners The number of diners in the simulation, maximum 13.

*/
template <class Sync, int numDiners>
void testDiningSkeleton() {
  // Neither Thread nor Semaphore can be used directly in a
  // vector. Thread is non-copyable.  Semaphore isn't but the copy and
  // the original refer to the same object (in RTEMS), which means
  // that the vector constructor isn't free to create and destroy
  // copies as it pleases.
  typedef std::tr1::shared_ptr<Sync> SyncPtr;
  printf("\n");
  printf("========= Dining Philosophers test with %s ==========\n", Sync::implementation());
  std::vector<SyncPtr>   sticks(numDiners);
  std::vector<PhilPtr>   diners(numDiners);

  std::vector<int> primes; // For thinking times.
  {  int const pr[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41};
     primes.assign(pr, pr + (sizeof pr)/(sizeof pr[0]));
  }
  for (int p = 0; p < numDiners; ++p) {
    sticks.at(p).reset(new Sync());
  }
  for (int p = 0; p < numDiners; ++p) {
    diners.at(p).reset(new Philosopher(*sticks.at(p), *sticks.at((p + 1) % numDiners), p, primes.at(p)));
  }
  Philosopher::setDiners(diners);
  for (int p = 0; p < numDiners; ++p) diners.at(p)->start();
  Thread::threadSleep(60 * 1000);
  printf("Dinner time is over! Wait for everyone to be thinking.\n");
  Philosopher::dinnerIsOver();
}



#endif
