// -*-Mode: C++;-*-
/*!@file     Exceptions.hh
*
* @brief     Define a collection of standard exceptions
*
* @author    R. Claus -- REG/DRD - (claus/slac/stanford/edu)
*
* @date      March 16, 2014 -- Created from DAT/RRI
*
* $Revision: 3096 $
*
* @verbatim                    Copyright 2014
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef TOOL_EXCEPTION_EXCEPTIONS_HH
#define TOOL_EXCEPTION_EXCEPTIONS_HH

#include <stdlib.h>
#include <stdint.h>

#include "exception/Exception.hh"

namespace tool
{
  namespace exception
  {
    class HeapError : public Error
    {
    public:
      HeapError(             const char* nm = "", const char* fn = "__func__");
      HeapError(size_t size, const char* nm = "", const char* fn = "__func__");
     ~HeapError();
    };

    class RangeError : public Error
    {
    public:
      RangeError(const char* nm = "",
                 const char* fn = "__func__");
      RangeError(uint32_t    idx,
                 const char* nm = "",
                 const char* fn = "__func__");
      RangeError(uint32_t    idx,
                 uint32_t    maxIdx,
                 const char* nm = "",
                 const char* fn = "__func__");
      RangeError(uint32_t    idx,
                 uint32_t    minIdx,
                 uint32_t    maxIdx,
                 const char* nm = "",
                 const char* fn = "__func__");
      RangeError(void*       ptr,
                 const char* nm = "",
                 const char* fn = "__func__");
      RangeError(void*       ptr,
                 void*       minPtr,
                 void*       maxPtr,
                 const char* nm = "",
                 const char* fn = "__func__");
     ~RangeError();
    };
  }
}
#endif
