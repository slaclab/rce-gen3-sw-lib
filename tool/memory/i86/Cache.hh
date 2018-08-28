/*!
* @file
* @brief   Implementation for various cache management functions.
* Copyright:                    Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
*/
#ifndef TOOL_MEMORY_CACHE_IMPL_HH
#define TOOL_MEMORY_CACHE_IMPL_HH

inline void tool::memory::cache::allocate(const void* where) {}
inline void tool::memory::cache::touch(const void* where) {}
inline void tool::memory::cache::flush(const void* where) {}
inline void tool::memory::cache::flush(const void* start, const void* const end) {}
inline void tool::memory::cache::invalidate(const void* where) {}
inline void tool::memory::cache::invalidate(const void* start, const void* const end) {}
inline void tool::memory::cache::sync() {}

#endif // TOOL_MEMORY_CACHE_IMPL_HH
