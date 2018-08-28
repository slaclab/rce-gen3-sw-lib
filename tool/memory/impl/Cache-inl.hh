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

inline void tool::memory::cache::store(const void* start, unsigned bytes)
{
  const char* last = reinterpret_cast<const char*>(start)+bytes;
  store(start, last);
}

inline void tool::memory::cache::flush(const void* start, unsigned bytes)
{
  const char* last = reinterpret_cast<const char*>(start)+bytes;
  flush(start, last);
}

inline void tool::memory::cache::invalidate(const void* start, unsigned bytes)
{
  const char* last = reinterpret_cast<const char*>(start)+bytes;
  invalidate(start, last);
}

inline void tool::memory::cache::zero(const void* start, unsigned bytes)
{
  const char* last = reinterpret_cast<const char*>(start)+bytes;
  zero(start, last);
}

inline void tool::memory::cache::iinvalidate(const void* start, unsigned bytes)
{
  const char* last = reinterpret_cast<const char*>(start)+bytes;
  iinvalidate(start, last);
}

//! @brief Flush the processor's instruction cache for a region of memory
inline void tool::memory::cache::clear(void* start, void* end)
{
  __builtin___clear_cache(start, end);
}

//! @brief Minimize cache-miss latency by moving data into a cache before it is accessed
inline void tool::memory::cache::prefetch(const void* addr, unsigned rw, unsigned locality)
{
  __builtin_prefetch(addr, rw, locality);
}

