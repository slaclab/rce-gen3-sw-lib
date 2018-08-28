// -*-Mode: C;-*-
/*
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
*/

/**

   @def ONCE_FLAG_INITIALIZER
   @brief The "not set" value of a OnceFlag.

   @code
   static OnceFlag foo = ONCE_FLAG_INITIALIZER;
   @endcode


   @class OnceFlag
   @brief Thread-safe once-only initialization flag.

   This is a thread-safe implementation of the following pattern
   @code
   static initDone = 0;

   void foo() {
     if (!initDone) {initDone = 1; init();}
   }
   @endcode
   
   An instance can be initialized statically using the macro ONCE_FLAG_INITIALIZER.
   @see ONCE_FLAG_INITIALIZER

   @cond development
   Implemented using the GCC builtin __sync_bool_compare_and_swap().
   @endcond

   @fn OnceFlag_testAndSet(OnceFlag* flag)
   @brief Atomically test the flag and set it if it's zero.
   @param[in] flag A pointer to the OnceFlag instance.
   @retval true The flag was unset but is now set.
   @retval false The flag was already set.
   @memberof OnceFlag

   @fn OnceFlag_init(OnceFlag* flag)
   @brief Run-time initialization of a OnceFlag.
   @param[in] flag A pointer to the OnceFlag.
   @memberof OnceFlag

   @var OnceFlag::notdone
   @brief The actual flag value passed to __sync_bool_compare_and_swap().
   

 */
