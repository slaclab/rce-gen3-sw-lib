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
       @class Once
       @brief Thread-safe calling of a once-only function.

       You often see something like this in packages that initialize
       themselves the first time you call them:

       @code
       static bool needInit = true;

       if (needInit) {
        initialize();
        needInit = false;
       }
       @endcode

       This class implements a thread-safe version of this logic,
       eliminating a race condition in the testing and setting of the
       flag. The package should a static instance of this class.
       Threads will call the run() member function passing as argument
       a pointer to initialization function, which will be called just
       once.

       @fn Once::run( void (*func)()  )
       @brief If run hasn't already been called, run a function.
       @param[in] func Pointer to the function to run.

       If the run() member function of this instance of Once hasn't
       already been called then the given function will be called.
       Note that there is no check to ensure that all threads pass
       the same function as argument; the do-this-once flag is
       associated with the Once instance, not with the function it
       calls.

       @var Once::m_data
       @brief Platform-dependent data for the instance.

     */
  }

}
