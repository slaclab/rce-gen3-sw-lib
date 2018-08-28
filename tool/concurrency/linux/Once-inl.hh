// -*-Mode: C++;-*-
/**
@file
@brief Inline implementation of class Once for Linux.
@verbatim
                               Copyright 2012
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/


#include <cerrno>
#include <cstring>

#include "pth.h"

#include "exception/Exception.hh"

/** @cond development */
#define PTH_CALL(VAR, FUNC, ARGS...)                                     \
  VAR = FUNC(ARGS);                                                      \
  if (!VAR) {                                                            \
    throw Error("Call to %s failed in %s: %s", #FUNC, __PRETTY_FUNCTION__, strerror(errno)); \
  }
/** @endcond */


namespace tool {

  namespace concurrency {

    using std::strerror;
    using tool::exception::Error;

    /** @cond development */
    namespace Once_private {
      struct ArgRec {void (*func)();};
      // The function pth_once() calls must take a void* argument
      // but the Once class specifies a function that takes none.
      inline void onearg(void *arg) {static_cast<ArgRec*>(arg)->func();}
    }
    /** @endcond */

    inline Once::Once(): m_data(PTH_ONCE_INIT) {}

    inline void Once::run(void (*func)()) {
      pth_init(); // Will fail gracefully if already called.
      Once_private::ArgRec arg = {func};
      int status;
      PTH_CALL(status, pth_once, &m_data, &Once_private::onearg, &arg);
    }

  } // concurrency

} // tool
#undef PTH_CALL
