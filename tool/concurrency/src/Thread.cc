// -*-Mode: C++;-*-
/**
@file
@brief Non-inline platform-independent implementation of class Thread.
Also contains documentation.

@verbatim
                               Copyright 2011
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/




#include "concurrency/Notepad.hh"
#include "concurrency/Once.hh"
#include "concurrency/Thread.hh"

namespace tool {

  namespace concurrency {

    /** @cond development */
    namespace {

      struct DummyThread: public Thread {
        DummyThread(): Thread(false) {}
        virtual void body() {}
        virtual ~DummyThread() {}
      };

      DummyThread dummy;

    }
    /** @endcond */

    Once Thread::m_once;

    Notepad<Thread*>* Thread::m_current = 0;

    Thread& Thread::currentThread() {
      if (m_current && m_current->get()) {
        // A current Thread has been defined.
        return *m_current->get();
      }
      else {
        // No Thread instance is current, use a the dummy.
        return dummy;
      }
    }

  } // concurrency

} // tool




// ========== Documentation ==========
namespace tool {

  namespace concurrency {

 /**
    @class Thread
    @brief Encapsulates a native flow of control, or thread.

    This class provides cooperative or non-preemptive threading.
    You may assume that a thread will keep control of the CPU until one of the
    following things happens:
    - Waiting for I/O to complete.
    - Calling sleep().
    - Calling yield().
    - Calling take() on a Semaphore.
    - Some thread (including itself) calling suspend() on the current thread.
    - Spawning a new thread.
    - Resuming a suspended thread.
    - Calling exit().

    Not all of these are guaranteed to cause a context switch
    every time but may switch depending on the state of the
    application.

    The member functions provided offer a subset of the operations
    allowed under RTEMS, but are implemented under Linux as well
    in order to allow generic code to be tested under Linux. Note
    that no join operation is provided since RTEMS tasks don't
    have it.

    The actual code to be run by the thread is the virtual member
    function body(), which has no implementation in this abstract
    base class.  In order to create and run a thread you must
    first derive a new class from Thread and give it the body you
    want. This approach automatically gives you a simple kind of
    "thread-local" storage; just declare the data as non-static
    members of the derived class. Code that has no access to the
    "this" pointer can use the static member function
    currentThread() to get a reference to the Thread instance
    corresponding to the currently running thread; if needed one
    can convert the result using dynamic_cast<>(). There are a number of
    static convenience member functions that use currentThread():
    threadYield(), threadSleep() and threadExit(), so named to
    avoid conflicts with the non-static member functions.

    @warning It is a profound design error to destroy a Thread
    instance before its body() has exited. The Thread destructor
    will suspend its instance if that instance is still running;
    it will in that case also log a message of Fatal severity.
    These actions may still come too late to prevent problems
    since by the time the error is detected the destructor
    of the Thread-derived class will have already been run,
    invalidating its data members. For example,
    operator delete may have been called.

    Each Thread may inherit its name, priority, stack size,
    default logging implementation and logging severity threshold
    from the thread calling the constructor. You can alter these
    settings in the constructor of your derived thread
    class. %Once the thread is started any changes you make to the
    name, priority or stack size of your thread object will have
    no effect on the platform-dependent implementation thread. In
    the thread code you can change the logger settings for a block
    or a function using the LoggingGuard class.
    @see service::logger::Logger
    @see service::logger::LoggingGuard

    The file descriptor for the parent thread's stdin stream is
    used for the stdin, stdout and stderr streams of the new
    thread.

    The Thread constructor doesn't actually create a platform
    thread.  After you create an instance of a class derived from
    Thread you can call its start() member function to create and
    start a new platform thread, or you can call graduate() to
    make the current platform thread run body() with all of the
    facilities provided by the Thread et al.

    Every thread goes through these phases, each of which has
    a corresponding test function, e.g., isUnstarted().
    - Unstarted. Constructed but start() or graduate() not called.
    - Starting. start() or graduate() called but body() not yet entered.
    - Started. body() is running.
    - Finished. body() has exited.

    There is no special state for suspended threads; for them isRunning()
    will be true.


    @enum Thread::Priority
    @brief Relative priorities. Lower numbers mean higher
    priorities.

    Each value maps to a distinct priority value in the platform
    threading API. The SYS values are for high priority system threads
    while the APP values are for application threads. Although
    priorities range from 1 to 255, using only these standard
    priorities will aid in porting Thread to new platforms.

    Operations for this enum and its values are provided using
    tool::type::EnumInfo.

    APP_PRIO0 should be the same as the default priority given
    to Tasks started using the RTEMS shell's "run" command.

    @var Thread::SYS_PRIO0
    @brief System priority 0 (the highest).

    @var Thread::SYS_PRIO1
    @brief System priority 1.

    @var Thread::SYS_PRIO2
    @brief System priority 2.

    @var Thread::SYS_PRIO3
    @brief System priority 3.

    @var Thread::SYS_PRIO4
    @brief System priority 4.

    @var Thread::APP_PRIO0
    @brief Application priority 0.

    @var Thread::APP_PRIO1
    @brief Application priority 1.

    @var Thread::APP_PRIO2
    @brief Application priority 2.

    @var Thread::APP_PRIO3
    @brief Application priority 3.

    @var Thread::APP_PRIO4
    @brief Application priority 4 (the lowest)


    @var Thread::MAX_NAMESIZE
    @brief The maximum size of the Thread name not counting any
          terminating null.



    @name Flags
    @brief Constants to make the constructor's first argument clearer.
    @{
        @var Thread::INHERIT
        @brief Do inherit properties from the parent Thread.

        @var Thread::DONT_INHERIT
        @brief There is no parent Thread or just don't inherit its properties.
    @}

    
    @fn Thread::Thread(bool inherit)
    @brief Create a new Thread that as yet has no thread created in the underlying API.
    @param[in] inherit If false the new Thread will have
    the following default properties:
    - Priority APP_PRIO2.
    - Minimum allowed stack size.
    - Name "THRD".
    - Logging implementation StderrLogger.
    - Logging threshold Info.

    If inherit is true then the new Thread will inherit its
    properties from the creating Thread. For this to work the
    creator must already be a Thread.

    In either case the new Thread will adopt the C stdio streams
    currently in use.


    @fn Thread::~Thread()
    @brief Suspend the platform thread *this and issue an error message
    if the thread is still running, else normal destruction.


    @fn Thread::start()
    @brief If isUnstarted() is true then create a new flow of
    control starting at body() and mark it as ready to execute.
    On RTEMS, the new thread may or may not get the CPU right
    away.  On Linux, start calls threadYield() so that body()
    may start to execute depending on whether higher priority
    threads are waiting to run.


    @fn Thread::graduate()
    @brief If isUnstarted() is true then set the properties of
    this Thread from those of the current flow of control, set
    this Thread as the current Thread, mark it as already
    starting and execute body().


    @fn Thread::isUnstarted() const
    @brief Has start() or graduate() never been called?


    @fn Thread::isStarting() const
    @brief Has start() or graduate() been called but the Thread has not run yet?


    @fn Thread::isRunning() const
    @brief Has the Thread received run time and not yet terminated?


    @fn Thread::isFinished() const
    @brief Has the thread finished execution?


    @fn Thread::suspend()
    @brief Suspend execution, if not already suspended, but do not terminate.


    @fn Thread::resume()
    @brief If suspended, resume execution.


    @fn Thread::body()
    @brief The body of code to run in the Thread.


    @fn Thread::priority(unsigned p)
    @brief Set the Thread priority.
    @param[in] p The priority value.
    @return *this (for chaining setter functions).


    @fn Thread::stacksize(unsigned s)
    @brief Set the Thread stack size. If need be the size will be
    increased to the system minimum when the platform thread is actually created.
    @param[in] s The stack size value.
    @return *this (for chaining setter functions).


    @fn Thread::name(const char* name)
    @brief Set the Thread name.
    @param[in] name The name string, copied into the Thread object.
    @return *this (for chaining setter functions).


    @fn Thread::loggerImpl(const LoggerImplPtr& impl)
    @brief Set the Thread logging handler.
    @param[in] impl The logger implementation to use.
    @return *this (for chaining setter functions).


    @fn Thread::logThreshold(Logger::Severity sev)
    @brief Set the logging severity threshold.
    @param[in] sev The severity level to set.
    @return *this (for chaining setter functions).


    @fn Thread::priority() const
    @return The thread priority.


    @fn Thread::stacksize() const
    @return The thread stack size in bytes.


    @fn Thread::name() const
    @return The thread name.


    @fn Thread::loggerImpl() const
    @return The thread's logging implementation.


    @fn Thread::logThreshold() const
    @return The thread's logging severity threshold.


    @fn Thread::currentThread()
    @return The Thread associated with the current flow of control.
    If no such Thread exists, return a reference to a dummy thread.


    @fn Thread::minimumStacksize()
    @return The minimum stack size recommended for the platform.


    @fn Thread::threadYield()
    @brief Allows nonmember code to call currentThread().yield().


    @fn Thread::threadExit()
    @brief Allows nonmember code to call currentThread().exit().


    @fn Thread::threadSleep(unsigned millisecs)
    @brief Allows nonmember code to call currentThread().sleep().
    @param[in] millisecs The sleep time in milliseconds.


    @fn Thread::yield()
    @brief Give up the processor.


    @fn Thread::exit()
    @brief Exit the Thread body then terminate the platform thread.


    @fn Thread::sleep(unsigned millisecs)
    @brief Make the Thread unready to run for a time (rounded up
          to an integral number of system clock ticks).
    @param[in] millisecs The sleep time in milliseconds.


    @fn Thread::Thread(const Thread&)
    @brief Not implemented; Threads are not copyable.


    @fn Thread::operator=(const Thread&)
    @brief Not implemented; Threads are not assignable.


    @var Thread::m_priority
    @brief The current priority of the thread.


    @var Thread::m_stacksize;
    @brief The thread stack size in bytes.


    @var Thread::m_name
    @brief The name of this Thread.


    @var Thread::m_loggerImpl
    @brief The logging implementation for this Thread.


    @var Thread::m_threshold
    @brief The logging severity threshold for this Thread.


    @enum Thread::RunState 
    @brief The possible states of Thread execution.


    @var Thread::UNSTARTED
    @brief The Thread has been created but start() has not been called.


    @var Thread::STARTING
    @brief start() has been called for the Thread but body() has not yet begun to execute.


    @var Thread::RUNNING
    @brief The Thread's body() member function is executing.


    @var Thread::FINISHED
    @brief The Thread's body() member function has exited.


    @var Thread::m_runState
    @brief The current run state of this Thread.


    @fn Thread::m_once
    @brief Used to ensure that class initialization code is called only once.


    @fn Thread::m_init();
    @brief The class initialization code.


    @fn Thread::m_current
    @brief Pointer to the Thread instance (if any) that owns the platform thread/task
    that is currently running.

    This is thread-local storage; a context switch restores the correct value for
    the newly selected thread/task.

 */

    
    /** @cond development */
    namespace {

      /**
         @class DummyThread
         @brief A do-nothing derived class of Thread.
         @see dummy

         @var dummy
         An instance of DummyThread. Thread::currentThread() returns a reference
         to this object if no Thread is associated with the
         current flow of control. This will be so for threads that
         weren't created using the Thread wrapper class.

      */
    }
    /** @endcond */


  } // concurrency

} // tool
