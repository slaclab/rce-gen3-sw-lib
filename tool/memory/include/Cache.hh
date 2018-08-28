/*!
* @file
* @brief   Various cache management functions.
*
* @author  Ric Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    March 30, 2011 -- Created
*
* Copyright:                    Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
*/
#ifndef TOOL_MEMORY_CACHE_HH
#define TOOL_MEMORY_CACHE_HH



namespace tool {
  namespace memory {
    namespace cache {
      void allocate(const void* where);
      void touch(const void* where);
      void store(const void* where);
      void store(const void* start, const void* const end);
      void store(const void* start, unsigned bytes);
      void flush(const void* where);
      void flush(const void* start, const void* const end);
      void flush(const void* start, unsigned bytes);
      void invalidate(const void* where);
      void invalidate(const void* start, const void* const end);
      void invalidate(const void* start, unsigned bytes);
      void zero(const void* where);
      void zero(const void* start, const void* const end);
      void zero(const void* start, unsigned bytes);
      void sync();
      void itouch(const void* where);
      void iinvalidate(const void* where);
      void iinvalidate(const void* start, const void* const end);
      void iinvalidate(const void* start, unsigned bytes);
      void isync();
      void clear( void* start, void* end);
      void prefetch(const void* addr, unsigned rw = 0, unsigned locality = 3);
    }; // cache
  }; // memory
}; // tool

#include "memory/impl/Cache-inl.hh"
#include "memory/cpu/Cache.hh"

#endif // SERVICE_CACHE_HH
