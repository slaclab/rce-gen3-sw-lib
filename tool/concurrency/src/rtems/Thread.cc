// -*-Mode: C++;-*-
/**
@file
@brief The implementation of class Thread under RTEMS.
@verbatim
                               Copyright 2010
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#include <algorithm>
#include <cstring>
#include <exception>

#include <stdio.h>


#include "exception/Exception.hh"
#include "logger/Logger.hh"
#include "logger/StderrLogger.hh"
#include "concurrency/Thread.hh"
#include "shell/redirect.h"

#include <rtems.h>



namespace {
  class ExitException {};
}

namespace tool {

  namespace concurrency {

    int systemTicksPerSec;

    void Thread::m_init() {
      m_current = new Notepad<Thread*>();
      rtems_clock_get(RTEMS_CLOCK_GET_TICKS_PER_SECOND, &systemTicksPerSec);
    }

    size_t Thread::minimumStacksize() {return RTEMS_MINIMUM_STACK_SIZE;}

    void Thread::start() {

      // A Thread may be started only once.
      if (!isUnstarted()) return;
      m_runState = STARTING;

      rtems_status_code status = RTEMS_SUCCESSFUL;

      rtems_name name          = rtems_build_name(m_name[0], m_name[1], m_name[2], m_name[3]);
      rtems_task_priority prio = m_priority;
      size_t stacksize         = std::max(minimumStacksize(), m_stacksize);
      rtems_mode mode          = RTEMS_NO_PREEMPT | RTEMS_NO_TIMESLICE;
      rtems_attribute attrib   = RTEMS_NO_FLOATING_POINT;

      // Create the new RTEMS task.
      status = rtems_task_create(name, prio, stacksize, mode, attrib, &m_taskId);
      if (status != RTEMS_SUCCESSFUL) {
	Logger().error("tool::concurrency::Thread::%s(): rtems_task_%s() failed, status code %d.", "start", "create", status);
	throw std::runtime_error("Thread creation failed.");
      }
      rtems_task_start(m_taskId,
		       reinterpret_cast<rtems_task_entry>(threadStartingPoint),
		       reinterpret_cast<rtems_task_argument>(this));
      // m_runState will be set to RUNNING once the thread begins
      // execution (see the code for ThreadJanitor below).
    }

    void Thread::graduate()
    {
      rtems_status_code   status;
      rtems_task_priority priority;

      if (! isUnstarted()) {
	Logger().error("tool::concurrency::Thread::graduate() can't be done twice for the same Thread.");
        throw std::runtime_error("Thread graduation failed.");
      }
      m_runState = STARTING;

      // Find our properties from task information
      status = rtems_task_ident(RTEMS_SELF, RTEMS_LOCAL, &m_taskId);
      if (status != RTEMS_SUCCESSFUL) {
	Logger().error("tool::concurrency::Thread::%s(): rtems_task_%s() failed, status code %d.", "takeOver", "ident", status);
      }
      status = rtems_task_set_priority(RTEMS_SELF, RTEMS_CURRENT_PRIORITY, &priority);
      if (status != RTEMS_SUCCESSFUL) {
	Logger().error("tool::concurrency::Thread::%s(): rtems_task_%s() failed, status code %d.", "takeOver", "set_priority", status);
      }
      m_priority = priority;
      /* Not sure how to get the stack size from RTEMS */
      rtems_object_get_name(m_taskId, sizeof(m_name), m_name);

      threadStartingPoint(this);

      // m_runState will be set to RUNNING once the thread begins
      // execution (see the code for ThreadJanitor below).
    }

    void Thread::suspend() {rtems_task_suspend(m_taskId);}

    void Thread::resume() {rtems_task_resume(m_taskId);}

    void Thread::yield() {
      rtems_task_wake_after(RTEMS_YIELD_PROCESSOR);
    }

    void Thread::sleep(unsigned millisecs) {
      const long int ticks = (long(systemTicksPerSec) *  millisecs + 999L) / 1000L;
      rtems_task_wake_after(int(ticks));
    }

    void Thread::exit() {throw ExitException();}

    Thread::Thread(bool inherit)
      : m_priority  (inherit ? currentThread().priority()     : APP_PRIO2),
	m_stacksize (inherit ? currentThread().stacksize()    : minimumStacksize()),
	m_loggerImpl(inherit ? currentThread().m_loggerImpl   : LoggerImplPtr(new StderrLogger())),
	m_threshold (inherit ? currentThread().logThreshold() : Logger::Info),
        m_runState  (UNSTARTED)
    {
      m_once.run(m_init);
      std::strncpy(m_name, inherit ? currentThread().name() : "TASK", MAX_NAMESIZE);
      m_name[MAX_NAMESIZE] = '\0';

      m_parentStdinFd = shell_getTtyFd();
    }

    Thread::~Thread() {
      if (isStarting() || isRunning()) {
        Logger().fatal
          ("The Thread with ID=0x%08x and name '%s' is being destroyed while still running.",
           (unsigned)m_taskId, name()
           );
        Logger().fatal("The Thread will be suspended.");
        suspend();
      }
    }




    struct ThreadJanitor {
      Thread *m_self;
      ThreadJanitor(Thread *self)
	: m_self(self)
      {
	if (m_self->m_parentStdinFd >= 0) {
          shell_redirectStdio(m_self->m_parentStdinFd);
	}

	// Use our notepad slot to allow non-member code to find out
	// that this Thread instance is associated with this RTEMS task.
	Thread::m_current->set(self);

	m_self->m_runState = Thread::RUNNING;
      }

      ~ThreadJanitor() {
	m_self->m_runState = Thread::FINISHED;
      }
    };

    void ThreadPlatform::threadStartingPoint(Thread *self) {
      ThreadJanitor tj(self);
      try {
	self->body();
      }
      catch(ExitException &) {
	// Not an error - the thread called exit().
      }
      catch(tool::exception::Exception &e) {
	Logger().fatal(e.what());
	Logger().fatal("Thread %s at %p didn't catch an tool::exception::Exception.",
		       self->name(),
		       self);
	rtems_task_suspend(RTEMS_SELF);
      }
      catch(std::exception &e) {
	Logger().fatal(e.what());
	Logger().fatal("Thread %s at %p didn't catch a std::exception.",
		       self->name(),
		       self);
	rtems_task_suspend(RTEMS_SELF);
      }
      catch(...) {
	Logger().fatal("Thread %s at %p didn't catch a non-standard exception.",
		       self->name(),
		       self);
	rtems_task_suspend(RTEMS_SELF);
      }
    }

  } // concurrency
} // tool




namespace {

  /** @class ExitException
      @brief This gets thrown in order to exit a Thread with the
      proper stack unwinding.
  */
}



namespace tool {

  namespace concurrency {

    /**
       @class ThreadJanitor
       @brief Used to set up and tear down a Thread.
       
       The top-level function of the RTEMS task created by class Thread
       creates an instance of this class as an automatic
       variable. Construction initializes the data associated with the
       Thread and destruction finalizes it.

       @fn ThreadJanitor::ThreadJanitor(Thread* self)
       @brief Initial setup for the Thread associated with the RTEMS task.
       @param[in] self A pointer to the Thread instance associated with the RTEMS task.

       Purpose:
       - Make sure tha the tasks stdio streams use the same file descriptors as those
         of the parent task.
       - Set self->m_runState to Thread::RUNNING.
       - Set m_self to self.

       @fn ThreadJanitor::~ThreadJanitor()
       @brief Sets m_self->m_runState to Thread::FINISHED in the Thread instance
       that was passed to the constructor.

       @var ThreadJanitor::m_self
       @brief The Thread* value that was passed to the constructor.
     */

    /**
       @class ThreadPlatform
       @brief RTEMS-specific base class for class Thread.

       Class Thread privately inherits from this class in order to get
       the declarations of platform-specific member data and member functions.

       @fn ThreadPlatform::threadStartingPoint(Thread* arg)
       @brief All threads made with class Thread start in this function.
       @param[in] arg A pointer to the Thread instance to be associated with
       the platform-specific thread that will run this function.

       This is the function that's passed to the platform threading API as the
       top-level function for the thread. It performs the following houskeeping
       tasks:
       - Declare an automatic instance of ThreadJanitor which will
         complete the initialization of the Thread instance.
       - Call the body() member function of the associated Thread.
       - Catch C++ exceptions that were not caught by the thread code.
         If such an exception is caught the thread is suspended after a
         diagnostic message is printed. If the caught exception is derived
         from tool::exception::Exception or from std::exception then
         the exception's what() string is printed as part of the
         diagnostic message.

       @var ThreadPlatform::m_taskId
       @brief The ID of the underlying RTEMS task..

       @var ThreadPlatform::m_parentStdinFd
       @brief The file descriptor no. for the stdin stream of the parent task.

       Unlike stdio streams, file descriptors are global resources
       available to all tasks. This file descriptor no. is stored so
       that later on (in the constructor for Thread) we can test to
       see whether it's the fd of the system console. If it isn't then
       new stdio streams will be created for the task that use this
       fd.

     */

    /**
       @var systemTicksPerSec
       @brief The frequency of the RTEMS system clock.
     */

  }

}
