// -*-Mode: C++;-*-
/**
@file
@brief Implements the Semaphore class for Linux.
@verbatim
                               Copyright 2010
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/


#include "exception/Exception.hh"

#include "concurrency/Semaphore.hh"
#include "concurrency/Thread.hh"

#include <errno.h>
#include <string.h>
#include <time.h>

using tool::exception::Error;

/** @brief Call a Pth function, test the return code and if needed throw Error. */
#define PTH_CALL(VAR, FUNC, ARGS...)                                     \
  VAR = FUNC(ARGS);                                                      \
  if (!VAR) {                                                            \
    throw Error("Call to %s failed in %s: %s", #FUNC, __PRETTY_FUNCTION__, strerror(errno)); \
  }

namespace tool {

  namespace concurrency {

    Semaphore::Semaphore(State initial)
    { pth_init(); // Will fail gracefully if already called.
      int status;
      PTH_CALL(status, pth_mutex_init, &m_mutex);
      // A mutex is created unlocked so we must take it if the initial
      // state is to be Red.
      if (initial == Red) {
	PTH_CALL(status, pth_mutex_acquire, &m_mutex, FALSE, 0);
      }
    }

    Semaphore::~Semaphore() {
      // N.B.: Pth has no "delete mutex" operation. A thread keeps a
      // ring of mutexes that it owns so deallocating an owned mutex
      // will leave the task with a pointer to the deallocated storage.
      if (m_mutex.mx_count > 0) {
        Logger().fatal("Attempt to destroy the Semaphore at %p while it is still taken.", this);
        Logger().fatal("The offending Thread will be suspended.");
        Thread::currentThread().suspend();
      }
    }

    bool Semaphore::take(unsigned timeout_usec)
    {
      int status;
      if (timeout_usec == 0) {
        errno = ENOSYS;
        status = pth_mutex_acquire(&m_mutex, FALSE, 0);
      }
      else {
        pth_time_t  deadline(pth_timeout(timeout_usec/1000000, timeout_usec%1000000));
        pth_event_t deadlineEvent(pth_event(PTH_EVENT_TIME, deadline));
        errno = ENOSYS;
        status = pth_mutex_acquire(&m_mutex, FALSE, deadlineEvent);
        int status2;
        PTH_CALL(status2, pth_event_free, deadlineEvent, PTH_FREE_THIS);
      }
      if (status == TRUE) {
        if (m_mutex.mx_count == 1) return true;
        throw Error("A Thread tried to take a Semaphore it already owns.");
      }
      if (errno == EINTR) return false; // Timeout.
      throw Error("Failed to take the semaphore (%s).", strerror(errno));
    }

    void Semaphore::give()
    {
      int status;
      PTH_CALL(status, pth_mutex_release, &m_mutex);
    }

  } // concurrency

} // tool
