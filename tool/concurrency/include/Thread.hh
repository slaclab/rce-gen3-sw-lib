// -*-Mode: C++;-*-
/**
@file
@brief Declares the Thread class.
@verbatim
                              Copyright 2010
                                    by
                       The Board of Trustees of the
                     Leland Stanford Junior University.
                            All rights reserved.
@endverbatim
*/

#if !defined(TOOL_CONCURRENCY_THREAD_HH)
#define      TOOL_CONCURRENCY_THREAD_HH


#include "logger/Logger.hh"
#include "concurrency/Notepad.hh"
#include "concurrency/Once.hh"

#include "concurrency/os/ThreadPlatform.hh"

namespace tool {

  namespace concurrency {

    using namespace service::logger;

    class Thread: private ThreadPlatform {
    public:

      enum Priority {
        SYS_PRIO0 =   1,
        SYS_PRIO1 =  15,
        SYS_PRIO2 =  80,
        SYS_PRIO3 =  90,
        SYS_PRIO4 = 100,
        APP_PRIO0 = 150,
        APP_PRIO1 = 151,
        APP_PRIO2 = 152,
        APP_PRIO3 = 153,
        APP_PRIO4 = 154
      };

      enum {
        MAX_NAMESIZE=20
      };

      static bool const INHERIT      = true;
      static bool const DONT_INHERIT = false;


      explicit Thread(bool inherit=INHERIT);

      virtual ~Thread();

      virtual void start();

      void graduate();

      bool isUnstarted() const;

      bool isStarting() const;

      bool isRunning() const;

      bool isFinished() const;

      void suspend();

      void resume();

      virtual void body() = 0;

      Thread& priority(unsigned);

      Thread& stacksize(unsigned bytes);

      Thread& name(const char*);

      Thread& loggerImpl(const LoggerImplPtr&);

      Thread& logThreshold(Logger::Severity);

      unsigned             priority()     const;
      unsigned             stacksize()    const;
      const char*          name()         const;
      const LoggerImplPtr& loggerImpl()   const;
      Logger::Severity     logThreshold() const;

    public:

      static Thread&  currentThread();

      static size_t minimumStacksize();

      static void   threadYield();

      static void   threadExit();

      static void   threadSleep(unsigned millisecs);

    protected:

      void yield();

      void exit();

      void sleep(unsigned millisecs);

    private:

      Thread(const Thread&);

      Thread& operator=(const Thread&);

    private:
      friend class ThreadJanitor;

      unsigned         m_priority;
      unsigned         m_stacksize;
      char             m_name[1+MAX_NAMESIZE];
      LoggerImplPtr    m_loggerImpl;
      Logger::Severity m_threshold;

      enum RunState {UNSTARTED, STARTING, RUNNING, FINISHED};
      RunState         m_runState;

    private:

      static Once m_once;

      static void m_init();

      static Notepad<Thread*> *m_current;

    };

  } // concurrency



  
  namespace type {
    using namespace tool::concurrency;

    /**@brief A specialization of tool::type::EnumInfo<T>.*/
    template<>
    struct EnumInfo<Thread::Priority> {
      static const Thread::Priority min =  Thread::SYS_PRIO0; ///< @brief Minimum priority value.
      static const Thread::Priority max =  Thread::APP_PRIO4; ///< @brief Maximum priority value.
      static const size_t count =  10;                        ///< @brief Number of priority values.

      /** @brief Converts priority values to strings.
          @param[in] p The priority value.
          @return The corresponding C-string.
      */
      static const char*      str (Thread::Priority p) {
	static const char* name[count] = {
	  "SYS_PRIO0", "SYS_PRIO1", "SYS_PRIO2", "SYS_PRIO3", "SYS_PRIO4",
	  "APP_PRIO0", "APP_PRIO1", "APP_PRIO2", "APP_PRIO3", "APP_PRIO4"
	};
	static const Thread::Priority value[count] = {
	  Thread::SYS_PRIO0, Thread::SYS_PRIO1, Thread::SYS_PRIO2, Thread::SYS_PRIO3, Thread::SYS_PRIO4,
	  Thread::APP_PRIO0, Thread::APP_PRIO1, Thread::APP_PRIO2, Thread::APP_PRIO3, Thread::APP_PRIO4
	};
	for (unsigned i = 0; i < count; ++i) if (value[i] == p) return name[i];
        return "*INVALID*";
      }
    };

  } // type



} // tool

#include "concurrency/impl/Thread-inl.hh"

#endif // TOOL_CONCURRENCY_THREAD_HH
