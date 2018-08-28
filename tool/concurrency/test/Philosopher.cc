// -*-Mode: C++;-*-
/**
@file
@brief Implement the test class Philosopher.

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

#include "Philosopher.hh"

const char* const Philosopher::State::m_thinking = "Thinking"; ///< @brief String for thinking state.

const char* const Philosopher::State::m_hungry   = "Hungry";   ///< @brief String for hungry state.

const char* const Philosopher::State::m_eating   = "Eating";   ///< @brief String for eating state.

Philosopher::State::State() : m_string("INVALID") {}

Philosopher::State& Philosopher::State::thinking() {m_string = m_thinking; return *this;}

Philosopher::State& Philosopher::State::hungry()   {m_string = m_hungry; return *this;}

Philosopher::State& Philosopher::State::eating()   {m_string = m_eating; return *this;}

bool Philosopher::State::isThinking() const {return m_string == m_thinking;}

bool Philosopher::State::isHungry()   const {return m_string == m_hungry;}

bool Philosopher::State::isEating()   const {return m_string == m_eating;}

const char* Philosopher::State::string() const {return m_string;}


/** @class Philosopher
    @brief Represent a Philosopher using Thread.

    Each Philosopher thinks for a period  set for it
    number and each eats for 3 seconds at a time. The Philosopher
    with ID i has chopstick i on his left and chopstick (i+1) mod N
    on his right.
*/

/** @typedef PhilPtr 
    @brief Type used to refer to a Philosopher.

    Class Philosopher is non-copyable, being derived from thread,
    so its instances can't be members of a vector. Smart-pointers
    to instances can be.
*/


std::vector<PhilPtr>* Philosopher::m_diners = 0;

int               Philosopher::m_numDiners = 0;

volatile bool     Philosopher::m_dinnerTime = true;

volatile int      Philosopher::m_dinersLock = 0;


/** @brief Initialize static variables prior to starting the simulation.
    @param[in] diners     The array of all the Philosophers who are dining.

    Remember the location of the vector of diners. Set dinnerTime = true.
 */
void Philosopher::setDiners(std::vector<PhilPtr>& diners) {
  m_diners          = &diners;
  m_numDiners       = diners.size();
  m_dinnerTime      = true;
  m_dinersLock      = 0;
}
  


/** @brief Print the states of all diners. */
void Philosopher::printStates() {
  for (int p = 0; p < m_numDiners; ++p) printf("%-10s", m_diners->at(p)->state().string());
  printf("\n");
}

  
/** @brief No two adjacent diners should be eating at the same time.
    @param[in] id The ID of the diner who's just started eating.
    @retval true Neither neighbor is eating at the same time.
    @retval false At least one neighbor is eating.
*/
bool Philosopher::checkAdjacents(int id) {
  // Avoid negative dividend of "%" by adding no. of diners.
  if (   (m_diners->at((id-1+m_numDiners) % m_numDiners)->state().isEating())
      || (m_diners->at((id+1)             % m_numDiners)->state().isEating())) {
    printf("ERROR: Adjacent eaters!\n");
    return false;
  }
  else {
    return true;
  }
}

/** @brief Every philosopher should have had a chance to eat.
    @retval true Everyone has eaten at least once.
    @retval false At least one diner never ate.
*/
bool Philosopher::checkStarvation() {
  bool ok = true;
  for (int i = 0; i < m_numDiners; ++i) {
    if (!m_diners->at(i)->hasEaten()) {
      printf("ERROR: Philosopher %d has starved!\n", i);
      ok = false;
    }
  }
  return ok;
}



/** @brief Remember my chopsticks and ID. Start thinking. Assign a Thread name
    of PhNN where NN is the ID.
    @param[in] left   The chopstick to my left.
    @param[in] right  The chopstick to my right.
    @param[in] id     My ID number.
    @param[in] thinkTime How long to think each time I think.
*/
Philosopher::Philosopher(Philosopher::Chopstick& left,
                         Philosopher::Chopstick& right,
                         int id,
                         int thinkTime)
  : Thread(),
    m_left(&left),
    m_right(&right),
    m_id(id),
    m_thinkTime(thinkTime),
    m_state(State().thinking()),
    m_haveEaten(false)
{
  char idname[20];
  snprintf(idname, sizeof idname, "Ph%02d", id);
  name(idname);
}


/** @brief The state of the philosopher. */
Philosopher::State Philosopher::state() const {return m_state;}

/** @brief Has the philosopher eaten? */
bool Philosopher::hasEaten() const {return m_haveEaten;}


/** @brief Check every state transition.
    @param[in] newState The new state after the transition.
    @retval true  The state change is legal.
    @retval false The state change is illegal.
    @see checkAdjacents(int id)
    @see printStates()

    After a Philosopher changes state to EATING we check to make sure
    that neither of the adjacent philosophers are are eating.  We also
    print the states of all diners.
*/
void Philosopher::setState(Philosopher::State newState) {
  // Need to synchronize since we're doing possibly locking I/O inside
  // the critical region.
  while (0 != __sync_lock_test_and_set(&m_dinersLock, 1)) sleep(0);
  m_state = newState;
  printStates();
  if (newState.isEating()) {
    m_haveEaten = true;
    checkAdjacents(m_id);
  }
  __sync_lock_release(&m_dinersLock);
}


/** @brief Signal the diners to exit their endless loops, then wait for them to finish. */
void Philosopher::dinnerIsOver() {
  m_dinnerTime = false;

  bool finished = false;
  while (!finished) {
    Thread::threadYield();
    finished = true;
    for (int p = 0; p < m_numDiners; ++p) {
      if (!m_diners->at(p)->isFinished()) finished = false;
    }
  }
  checkStarvation();
}


/** @brief Execute a loop: think, get chopsticks, eat, put down chopsticks, think, ... */
void Philosopher::body() {
  while (m_dinnerTime) { /* At the test we're always thinking. */
    sleep(m_thinkTime * 1000);
    setState(State().hungry());
    m_left->pickUp();
    m_right->pickUp();
    setState(State().eating());
    sleep(3 * 1000); // Eating time is constant.
    // The order of put-downs must be the opposite of that of pick-ups
    // so as to prevent deadlocks.
    m_right->putDown();
    m_left->putDown();
    setState(State().thinking());
  }
}



/** @class Philosopher::Chopstick
    @brief The abstract base class for chopsticks.

    @fn Philosopher::Chopstick::pickUp()
    @brief Pick up (reserve) a chopstick.

    @fn Philosopher::Chopstick::putDown()
    @brief Put down (release) a chopstick.

    @fn Philosopher::Chopstick::~ChopStick()
    @brief The required virtual destructor.



    @class Philosopher::State
    @brief The state of a philosopher.

    @var Philosopher::State::m_string
    @brief One of the valid state strings defined as static class members.

    @fn Philosopher::State::State()
    @brief Set the state to INVALID. Use a state-setter member fn to set a valid state.

    @fn Philosopher::State::thinking()
    @brief Set the state to Thinking.

    @fn Philosopher::State::hungry()
    @brief Set the state to Hungry.

    @fn Philosopher::State::eating()
    @brief Set the state to Eating.

    @fn Philosopher::State::isThinking() const
    @brief In the Thinking state?
    @retval true The state is Thinking.
    @retval false The state is not Thinking.

    @fn Philosopher::State::isHungry() const
    @brief In the Hungry state?
    @retval true The state is Hungry.
    @retval false The state is not Hungry.

    @fn Philosopher::State::isEating() const
    @brief In the Eating state?
    @retval true The state is Eating.
    @retval false The state is not Eating.

    @fn Philosopher::State::string() const
    @brief The C-string name of the state.
    @return The string.

*/
