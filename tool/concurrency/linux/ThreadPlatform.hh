// -*-Mode: C++;-*-
/**
@file
@brief Linux-specific base class for tool::concurrency::Thread.
@verbatim
                               Copyright 2012
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#if !defined(TOOL_CONCURRENCY_THREADPLATFORM_HH)
#define      TOOL_CONCURRENCY_THREADPLATFORM_HH

#include <cstdio>

#include "pth.h"

// See src/linux/Thread.cc for the implementation and documentation.
namespace tool {

  namespace concurrency {
      
      struct ThreadPlatform {
        static void* threadStartingPoint(void*);
        pth_t m_pth;  // The thread identifier.
        std::FILE *m_stdin, *m_stdout, *m_stderr;
        ThreadPlatform() : m_pth(), m_stdin(0), m_stdout(0), m_stderr(0) {}
      };

  } // concurrency
} // tool


#endif // TOOL_CONCURRENCY_THREADPLATFORM_HH

