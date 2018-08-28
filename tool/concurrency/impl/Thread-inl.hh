// -*-Mode: C++;-*-
/**
@file
@brief Inline definitions for the Thread class in tool::concurrency.

@verbatim
                              Copyright 2010
                                    by
                       The Board of Trustees of the
                     Leland Stanford Junior University.
                            All rights reserved.
@endverbatim
*/
#include <cstring>

namespace tool {

  namespace concurrency {

    inline Thread& Thread::priority(unsigned p)                {m_priority   = p; return *this;}
    inline Thread& Thread::stacksize(unsigned s)               {m_stacksize  = s; return *this;}
    inline Thread& Thread::loggerImpl(const LoggerImplPtr &l)  {m_loggerImpl = l; return *this;}
    inline Thread& Thread::logThreshold(Logger::Severity t)    {m_threshold  = t; return *this;}

    inline Thread& Thread::name(const char* n) {
      std::strncpy(m_name, n, MAX_NAMESIZE);
      m_name[MAX_NAMESIZE] = '\0';
      return *this;
    }


    inline bool                 Thread::isRunning()    const {return m_runState == RUNNING;}
    inline bool                 Thread::isStarting()   const {return m_runState == STARTING;}
    inline bool                 Thread::isUnstarted()  const {return m_runState == UNSTARTED;}
    inline bool                 Thread::isFinished()   const {return m_runState == FINISHED;}
    inline unsigned             Thread::priority()     const {return m_priority;}
    inline unsigned             Thread::stacksize()    const {return m_stacksize;}
    inline const char*          Thread::name()         const {return m_name;}
    inline const LoggerImplPtr& Thread::loggerImpl()   const {return m_loggerImpl;}
    inline Logger::Severity     Thread::logThreshold() const {return m_threshold;}

    inline void    Thread::threadYield()                   {currentThread().yield();}
    inline void    Thread::threadExit()                    {currentThread().exit();}
    inline void    Thread::threadSleep(unsigned millisecs) {currentThread().sleep(millisecs);}


  } // concurrency

} // tool

