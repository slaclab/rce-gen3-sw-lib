/*!
* @file
* @brief   Implementation of various cache management functions.
*
* @author  Anonymous -- DRD (somebody@slac.stanford.edu)
*
* @date    March 30, 2011 -- Created
*
* $Revision: 662 $
*
* Copyright:                    Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
*/
#ifndef TOOL_MEMORY_CACHE_IMPL_HH
#define TOOL_MEMORY_CACHE_IMPL_HH


inline void tool::memory::cache::allocate(const void* where)
{
}

inline void tool::memory::cache::touch(const void* where)
{
}

//! @brief Write back D-cache lines but do not invalidate.
inline void tool::memory::cache::store(const void* where)
{
}

//! @brief Write back D-cache lines but do not invalidate.
inline void tool::memory::cache::store(const void* start, const void* const end)
{
}

//! @brief Write back (if needed) and invalidate D-cache lines.
inline void tool::memory::cache::flush(const void* where)
{
}

//! @brief Write back (if needed) and invalidate D-cache lines.
inline void tool::memory::cache::flush(const void* start, const void* const end)
{
}

inline void tool::memory::cache::invalidate(const void* where)
{
}

inline void tool::memory::cache::invalidate(const void* start, const void* const end)
{
}

inline void tool::memory::cache::zero(const void* where)
{
}

inline void tool::memory::cache::zero(const void* start, const void* const end)
{
}

inline void tool::memory::cache::sync()
{
}

inline void tool::memory::cache::itouch(const void* where)
{
}

inline void tool::memory::cache::iinvalidate(const void* where)
{
}

inline void tool::memory::cache::iinvalidate(const void* start, const void* const end)
{
}

inline void tool::memory::cache::isync()
{
}

#endif // TOOL_MEMORY_CACHE_IMPL_HH
