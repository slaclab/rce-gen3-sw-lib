// -*-Mode: C++;-*-
/**
@file
@brief Implements the Semaphore class for RTEMS. From the original by A. Perrazo and J. Panetta.
@verbatim
                               Copyright 2010
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/



#include "exception/Exception.hh"
using tool::exception::Error;

#include "concurrency/Semaphore.hh"
#include "concurrency/Thread.hh"

namespace tool {

  namespace concurrency {

    Semaphore::Semaphore(State initial)
    {
      rtems_name name = rtems_build_name('S', 'E', 'M', 'N');
      rtems_status_code status =
	rtems_semaphore_create(name,
			       int(initial),
			       RTEMS_SIMPLE_BINARY_SEMAPHORE,
			       0,
			       &m_semId);
      // Note that RTEMS automatically considers you the holder of the
      // semaphore if you create it empty (Red).
      if (status) {
	throw Error("Failed to create semaphore (status code 0x%x)", status);
      }
    }

    Semaphore::~Semaphore()
    {
      const rtems_status_code status = rtems_semaphore_obtain(m_semId, RTEMS_NO_WAIT, 0);
      // A SIMPLE binary semaphore in RTEMS can be deleted without
      // getting a status of RTEMS_RESOURCE_IN_USE.  However, it can't
      // be taken by two tasks ow twice by the same task.
      if (status != RTEMS_SUCCESSFUL) {
        Logger().fatal("Attempt to destroy the Seamphore at %p while it is still taken.", this);
        Logger().fatal("The offending Thread will be suspended.");
        Thread::currentThread().suspend();
      }
      rtems_semaphore_delete(m_semId);
    }

    bool Semaphore::take(unsigned timeout_usec)
    {
      const unsigned ticks =
	(timeout_usec == 0) ? RTEMS_NO_TIMEOUT : RTEMS_MICROSECONDS_TO_TICKS(timeout_usec);
      const rtems_status_code status = rtems_semaphore_obtain(m_semId, RTEMS_WAIT, ticks);
      if (status == RTEMS_SUCCESSFUL) return true;
      if (status == RTEMS_TIMEOUT) return false;
      throw Error("Failed to take semaphore (status code 0x%x)", status);
    }

    void Semaphore::give()
    {
      const rtems_status_code status = rtems_semaphore_release(m_semId);
      if (status) {
	throw Error("Failed to give semaphore (status code 0x%x)", status);
      }
    }

  } // concurrency

} // tool
