// -*-Mode: C++;-*-
/**
@file
@brief Documentation of the generic critical section classes.
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
       @class CriticalSectionLock
       @brief An generic mutual exclusion object for critical sections.
       @cond development
       @details A derived class of Semaphore which uses its default constructor.
       @endcond

       @fn CriticalSectionLock::CriticalSectionLock()
       @brief Initializes the lock to the unlocked state.

       @fn CriticalSectionLock::CriticalSectionLock()
       @brief Destroys the generic lock.



       @class CriticalSection
       @brief An generic critical section which uses CriticalSectionLock for exclusion.
       @details Once constructed the critical section persists until
       the destructor is called. That will happen
       when the scope in which the object is declared is exited, be it via
       throwing an exception, returning from a function or reaching the end of a code block.
       @cond development
       @details A derived class of SemaphoreGuard; essentially a synonym for it.
       @endcond

       @fn CriticalSection::CriticalSection(CriticalSectionLock& lock)
       @brief Takes the lock.
       @param[in,out] lock The generic lock for this critical section.

       @fn CriticalSection::~CriticalSection()
       @brief Release the lock that was given to the constructor.
       
     */
  }

}
