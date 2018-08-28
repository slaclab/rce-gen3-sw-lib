// -*-Mode: C++;-*-
/*!@file
* @brief Inline implementation of class CriticalSectionLock.
*
* @verbatim                    Copyright 2014
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#include "concurrency/Semaphore.hh"
#include "concurrency/SemaphoreGuard.hh"


inline
tool::concurrency::CriticalSectionLock::CriticalSectionLock() :
  tool::concurrency::Semaphore()
{
}

inline
tool::concurrency::CriticalSectionLock::~CriticalSectionLock()
{
}


inline
tool::concurrency::CriticalSection::CriticalSection(CriticalSectionLock& lock) :
  tool::concurrency::SemaphoreGuard(lock)
{
}

inline
tool::concurrency::CriticalSection::~CriticalSection()
{
}
