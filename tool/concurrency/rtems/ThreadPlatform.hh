// -*-Mode: C++;-*-
/**
@file
@brief RTEMS-specific base class for class tool::concurrency::Thread.
@verbatim
                               Copyright 2011
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#if !defined(TOOL_CONCURRENCY_THREADPLATFORM_HH)
#define      TOOL_CONCURRENCY_THREADPLATFORM_HH

#include <rtems.h>
#include <cstdio>

namespace tool {

  namespace concurrency {

    class Thread;


    struct ThreadPlatform {
      static void threadStartingPoint(Thread* arg);
      rtems_id m_taskId;
      int m_parentStdinFd;
      ThreadPlatform() : m_taskId(0), m_parentStdinFd(-1) {}
    };

  } // concurrency
} // tool


#endif // TOOL_CONCURRENCY_THREADPLATFORM_HH

