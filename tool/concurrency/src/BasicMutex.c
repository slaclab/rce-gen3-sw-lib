// -*-Mode: C;-*-
/*
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

/**
   @typedef BasicMutex
   @brief Avoid having to say "struct BasicMutex" in C code.

   @class BasicMutex
   @brief Define a close-to-the-metal mutex.

   The aim is to provide the following features:
   - No dependence on operating system services.
   - Works on all supported platforms.
   - Works even outside of a task/thread context.
   - May be statically initialized.
   - Resistant to false sharing.
   - Works for SMP.

   Note that a "try-lock" member function is implemented but not an
   unconditional "lock". What should be done when a lock attempt fails
   depends on the application. In some cases one just spins, i.e.,
   retries the operation at once. In other cases one may need to yield
   the processor, or sleep, etc.

   @see BASIC_MUTEX_INITIALIZER

   @cond developer
   Not all operating systems give you a mutex or semaphore that can
   be allocated statically and statically initialized to a proper
   state.  In such a case the mutex must be created dynamically,
   which process may itself be subject to a data race.  The OS mutex
   may also fail to work properly if used outside the context of any
   task or thread, e.g., during system initialization.

   So-called false sharing may occur on systems that actually lock
   more memory, the exclusion granularity, than you use for your
   mutex. A typical case is that an entire cache line is locked but
   some systems lock much more memory. In such cases even ordinary
   writes to variables near the mutex may break the lock. A properly
   programmed lock will still work but use more lock retries than
   would otherwise have been needed. In order to make sure
   that no two BasicMutexes occupy the same exclusion granule we
   align each instance to a granularity boundary and make sure the
   instance takes up an entire granule. Of course this may waste
   some space.

   A BasicMutex uses GCC synchonization built-ins which use locking
   instruction sequences that will work on SMP systems that use a
   memory-coherency mechanism.
   @endcond

   @fn BasicMutex_trylock(BasicMutex* mutex)
   @memberof BasicMutex
   @brief Attempt to lock a BasicMutex.
   @param[in,out] mutex A pointer to the mutex.
   @retval 0 Success, you have the lock.
   @retval nonzero Failure.

   Locking doesn't disable interrupts.

   @fn BasicMutex_unlock(BasicMutex* mutex)
   @memberof BasicMutex
   @brief Unlock a BasicMutex. The caller must ensure that it is the
   holder of a lock gotten using BasicMutex_trylock().
   @param[in,out] mutex A pointer to the mutex.

   @fn BasicMutex_init(BasicMutex* mutex)
   @memberof BasicMutex
   @brief Dynamically initializes a BasicMutex to the unlocked state.
   @param[out] mutex A pointer to the mutex.

   @var BasicMutex::lock
   @brief The actual lock variable.

   @var BasicMutex::filler
   @brief Make the struct occupy an entire exclusion granule.

   @def BASIC_MUTEX_INITIALIZER
   @brief  Initializes a BasicMutex to the unlocked state.
   @code
   BasicMutex foo = BASIC_MUTEX_INITIALIZER;
   @endcode
*/
