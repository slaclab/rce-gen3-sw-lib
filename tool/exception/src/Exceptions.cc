// -*-Mode: C++;-*-
/*!@file     Exceptions.hh
*
* @brief     Define a collection of standard exceptions
*
* @author    R. Claus -- REG/DRD - (claus/slac/stanford/edu)
*
* @date      March 16, 2014 -- Created from DAT/RRI
*
* $Revision: 3125 $
*
* @verbatim                    Copyright 2014
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#include "exception/Exceptions.hh"

using namespace tool::exception;

HeapError::HeapError(const char* nm,
                     const char* fn) :
  Error("%s: %s allocation from heap failed", fn, nm)
{
}

HeapError::HeapError(size_t      sz,
                     const char* nm,
                     const char* fn) :
  Error("%s: %s allocation of %u bytes from heap failed", fn, nm, sz)
{
}

HeapError::~HeapError()
{
}


RangeError::RangeError(const char* nm,
                       const char* fn) :
  Error("%s: Parameter %s is out of range", fn, nm)
{
}

RangeError::RangeError(uint32_t    idx,
                       const char* nm,
                       const char* fn) :
  Error("%s: Value %s (%08x) is out of range", nm, fn, idx)
{
}

RangeError::RangeError(uint32_t    idx,
                       uint32_t    maxIdx,
                       const char* nm,
                       const char* fn) :
  Error("%s: Value %s (%08x) is out of range [0, %08x)", fn, nm, idx, maxIdx)
{
}

RangeError::RangeError(uint32_t    idx,
                       uint32_t    minIdx,
                       uint32_t    maxIdx,
                       const char* nm,
                       const char* fn) :
  Error("%s: Value %s (%08x) is out of range [%08x, %08x)",
        fn, nm, idx, minIdx, maxIdx)
{
}

RangeError::RangeError(void*       ptr,
                       const char* nm,
                       const char* fn) :
  Error("%s: Pointer %s (%p) is out of range",
        fn, nm)
{
}

RangeError::RangeError(void*       ptr,
                       void*       minPtr,
                       void*       maxPtr,
                       const char* nm,
                       const char* fn) :
  Error("%s: Pointer %s (%p) is out of range [%p, %p)",
        fn, nm, ptr, minPtr, maxPtr)
{
}

RangeError::~RangeError()
{
}
