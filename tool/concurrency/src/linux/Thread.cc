// -*-Mode: C++;-*-
/**
@file
@brief Implement the Thread class for Linux using GNU Pth.
@verbatim
                               Copyright 2012
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#include <errno.h>
#include <stdio.h>
#include <string.h>


#include "exception/Exception.hh"
#include "logger/Logger.hh"
#include "logger/StderrLogger.hh"
#include "concurrency/Thread.hh"

#include "pth.h"



#define PTH_CALL(VAR, FUNC, ARGS...)                                     \
  VAR = FUNC(ARGS);                                                      \
  if (!VAR) {                                                            \
    throw Error("Call to %s failed in %s: %s", #FUNC, __PRETTY_FUNCTION__, strerror(errno)); \
  }


namespace {
  class ExitException {};
}



namespace tool {

  namespace concurrency {

    using tool::exception::Error;

    void Thread::m_init() {
      m_current = new Notepad<Thread*>();
    }

    size_t Thread::minimumStacksize() {return 64 * 1024;}

    void Thread::start() {
      if (! isUnstarted()) return; // Start only once.
      m_runState = STARTING;

      // We have hacked the Pth source code so that the value
      // indicating highest priority is -1 and the one indicating the
      // lowest is -255.  That way we can just negate the Thread
      // priority value and get the same scheduling under Pth as under
      // RTEMS.
      pth_attr_t attr;
      int status;
      PTH_CALL(attr, pth_attr_new);
      PTH_CALL(status, pth_attr_set, attr, PTH_ATTR_PRIO,         -m_priority);
      PTH_CALL(status, pth_attr_set, attr, PTH_ATTR_NAME,         m_name);
      PTH_CALL(status, pth_attr_set, attr, PTH_ATTR_DISPATCHES,   0);
      PTH_CALL(status, pth_attr_set, attr, PTH_ATTR_JOINABLE,     FALSE);
      PTH_CALL(status, pth_attr_set, attr, PTH_ATTR_CANCEL_STATE,
               PTH_CANCEL_ENABLE | PTH_CANCEL_ASYNCHRONOUS);
      PTH_CALL(status, pth_attr_set, attr, PTH_ATTR_STACK_SIZE,   m_stacksize);
      
      PTH_CALL(m_pth, pth_spawn, attr, threadStartingPoint, this);
      PTH_CALL(status, pth_attr_destroy, attr);
      threadYield();
    }

    void Thread::graduate() {

      if (! isUnstarted()) {
	Logger().error("tool::concurrency::Thread::graduate() can't be done twice for the same Thread.");
        throw std::runtime_error("Thread graduation failed.");
      }
      m_runState = STARTING;

      // Find our properties from task information
      int prio;
      unsigned stacksize;
      char* name;
      pth_attr_t attr;
      int status;

      PTH_CALL(m_pth, pth_self);
      PTH_CALL(attr, pth_attr_of, m_pth);

      PTH_CALL(status, pth_attr_get, attr, PTH_ATTR_PRIO,       &prio);
      PTH_CALL(status, pth_attr_get, attr, PTH_ATTR_NAME,       &name);
      m_priority = - prio;
      PTH_CALL(status, pth_attr_get, attr, PTH_ATTR_STACK_SIZE, &stacksize);
      if (stacksize == 0) {
        // The current thread is the main-thread created by Pth.
        m_stacksize = minimumStacksize();
      }
      else {
        m_stacksize = stacksize;
      }
      strncpy(m_name, name, MAX_NAMESIZE);
      m_name[MAX_NAMESIZE] = '\0';

      // Adjust other properties to the same as used by start().
      PTH_CALL(status, pth_attr_set, attr, PTH_ATTR_DISPATCHES,   0);
      PTH_CALL(status, pth_attr_set, attr, PTH_ATTR_JOINABLE,     FALSE);
      PTH_CALL(status, pth_attr_set, attr, PTH_ATTR_CANCEL_STATE,
               PTH_CANCEL_ENABLE | PTH_CANCEL_ASYNCHRONOUS);

      threadStartingPoint(this);

      // m_runState will be set to RUNNING once the thread begins
      // execution (see the code for ThreadJanitor below).
    }


    namespace {
      class Suspender: public Thread {
        Thread* m_victim;
      public:
        Suspender(Thread* victim): Thread(), m_victim(victim) {priority(PTH_PRIO_MAX); name("Suspender");}
        virtual ~Suspender() {}
        virtual void body() {m_victim->suspend();}
      };
    }


    void Thread::suspend() {
      // A Pth thread can't suspend itself.
      if (m_pth != pth_self()) {
        // pth_suspend() returns an error if the thread is already suspended or terminated.
        // Let both conditions pass.
        pth_suspend(m_pth);
      }
      else {
        // Simulate self-suspension by creating a high-priority thread
        // to do the job.
        Suspender sus(this);
        sus.start();
        yield();
      }
    }

    void Thread::resume() {pth_resume(m_pth);}

    void Thread::yield() {int status; PTH_CALL(status, pth_yield, 0);}

    void Thread::sleep(unsigned millisecs) {
      pth_usleep(1000 * millisecs);  // Doesn't obey the usual Pth return code rules.
    }

    void Thread::exit() {throw ExitException();}

    Thread::Thread(bool inherit)
      : m_priority  (inherit ? currentThread().priority()     : APP_PRIO2),
	m_stacksize (inherit ? currentThread().stacksize()    : minimumStacksize()),
	m_loggerImpl(inherit ? currentThread().m_loggerImpl   : LoggerImplPtr(new StderrLogger())),
	m_threshold (inherit ? currentThread().logThreshold() : Logger::Info),
        m_runState  (UNSTARTED)
    {
      pth_init(); // Will fail gracefully if already called.

      // Now use Once to run the class init.
      m_once.run(m_init);

      std::strncpy(m_name, inherit ? currentThread().name() : "TASK", MAX_NAMESIZE);
      m_name[MAX_NAMESIZE] = '\0';

      // Take the stdio streams of the creating task.
      m_stdin  = stdin;
      m_stdout = stdout;
      m_stderr = stderr;
    }

    Thread::~Thread() {
      if (isStarting() || isRunning()) {
        Logger().fatal("The Thread with ID=%p and name '%s' is being destroyed while still running.", m_pth, name());
        Logger().fatal("The Thread will be suspended.");
        suspend();
      }
    }



    struct ThreadJanitor {
      Thread *m_self;
      ThreadJanitor(Thread *self)
	: m_self(self)
      {
	if (m_self->m_stdin) {
	  stdin  = m_self->m_stdin;
	  stdout = m_self->m_stdout;
	  stderr = m_self->m_stderr;
	}

	// Use our notepad slot to allow non-member code to find out
	// that this Thread instance is associated with this Pth thread.
	Thread::m_current->set(self);

	m_self->m_runState = Thread::RUNNING;
      }

      ~ThreadJanitor() {
	m_self->m_runState = Thread::FINISHED;
      }
    };


    void* ThreadPlatform::threadStartingPoint(void *arg) {
      Thread *const self(static_cast<Thread*>(arg));
      ThreadJanitor tj(self);
      try {
	self->body();
      }
      catch(ExitException &) {
	// Not an error - the thread called exit().
      }
      catch(tool::exception::Exception &e) {
	Logger().fatal("Thread %s at %p didn't catch a tool::exception::Exception. Its message follows.",
		       self->name(),
		       self);
	Logger().fatal(e.what());
	self->suspend();
      }
      catch(std::exception &e) {
	Logger().fatal("Thread %s at %p didn't catch a std::exception. Its message follows.",
		       self->name(),
		       self);
	Logger().fatal(e.what());
	self->suspend();
      }
      catch(...) {
	Logger().fatal("Thread %s at %p didn't catch a non-standard exception.",
		       self->name(),
		       self);
	self->suspend();
      }
      return 0;
    }
    

  } // concurrency

} // tool




// ========== Documentation ==========


/**
   @def PTH_CALL(VAR, FUNC, ARGS...)
   @brief Call a Pth function, test the return code and throw Error if needed.
*/


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
       
       The top-most function of a Pth thread creates an instance of this
       class as an automatic variable. Construction initializes the data
       associated with the Thread and destruction finalizes it.

       @fn ThreadJanitor::ThreadJanitor(Thread* self)
       @brief Sets self->m_stdin, self->m_stdout, self->m_stderr, and self->m_current.
       Sets self->m_runState to Thread::RUNNING.
       @param[in] self A pointer to the Thread instance corresponding to the Pth thread.

       @fn ThreadJanitor::~ThreadJanitor()
       @brief Sets m_runState to Thread::FINISHED in the Thread instance
       that was passed to the constructor.

       @var ThreadJanitor::m_self
       @brief The Thread* value that was passed to the constructor.
     */



    /**
       @class ThreadPlatform
       @brief Linux-specific base class for class Thread.

       Class Thread privately inherits from this class in order to get
       the declarations of platform-specific member data and member functions.

       @fn ThreadPlatform::threadStartingPoint(void* arg)
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

       @var ThreadPlatform::m_pth
       @brief The identifier of the Pth thread.

       @var ThreadPlatform::m_stdin
       @brief The stdin stream for this thread.

       @var ThreadPlatform::m_stdout
       @brief The stdout stream for this thread.

       @var ThreadPlatform::m_stderr
       @brief The stderr stream for this thread.

     */


    namespace {

      /**
         @class Suspender
         @brief A Thread used to suspend other Threads.

         Pth doesn't allow a thread to suspend itself, so Thread::suspend() starts
         a Suspender thread whose sole job is to suspend the parent thread.

         @fn Suspender::Suspender(Thread* victim)
         @brief Store the pointer to the Thread to be suspended.
         @param[in] victim Apointer to the Thread to be suspended.

         @var Suspender::m_victim
         @brief The pointer to the thread to be suspended.
       */
    }


  }

}
