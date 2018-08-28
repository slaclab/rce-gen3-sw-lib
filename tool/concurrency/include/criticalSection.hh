// -*-Mode: C++;-*-
/*!@file
* @brief Generic critical section class
* @verbatim                    Copyright 2014
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef TOOL_CONCURRENCY_CRITICAL_SECTION_HH
#define TOOL_CONCURRENCY_CRITICAL_SECTION_HH

#include "concurrency/SemaphoreGuard.hh"

namespace tool
{
  namespace concurrency
  {
    class CriticalSectionLock : public Semaphore
    {
    public:
      CriticalSectionLock();
     ~CriticalSectionLock();
    };

    class CriticalSection : public SemaphoreGuard
    {
    public:
      CriticalSection(CriticalSectionLock& lock);
     ~CriticalSection();
    };
  }
}

#include "concurrency/impl/criticalSectionImpl.hh"

#endif
