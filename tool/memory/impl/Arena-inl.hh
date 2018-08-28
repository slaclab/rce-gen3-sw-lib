/*!@class  Arena
*
* @brief   Class Arena manages a block of memory from which smaller blocks
*          may be allocated
*
*    Class Arena, from which memory is allocated for DMA transfers.  The
*    memory thus allocated is parcelled out as needed by the Allocator classes.
*    Although it is used this way, this class is meant to be generic, allowing
*    one to place an arena anywhere in the allowable address space.  Note that
*    once allocated, an arena is not deallocatable under the assumption that
*    the owning process will be restarted when this action is needed.
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    April 14, 2009 -- Created
*
* @verbatim                    Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

#include "debug/print.h"

inline
tool::memory::Arena* tool::memory::Arena::instance(uint32_t id)
{
  extern tool::memory::Arena* _arenas[];
  tool::memory::Arena* arena = _arenas[id];
  if (arena == 0)  dbg_bugcheck("No Arenas found");
  return arena;
}

inline
tool::memory::Arena::Arena() :
  _base(0),
  _ptr(0),
  _end(0)
{
}

inline
tool::memory::Arena::~Arena()
{
}

inline
void* tool::memory::Arena::operator new (size_t   size,
                                         uint32_t id)
{
  return tool::memory::Arena::instance(id)->_allocate(size);
}

inline
void* tool::memory::Arena::operator new (size_t               size,
                                         tool::memory::Arena& arena)
{
  return arena._allocate(size);
}

inline
void* tool::memory::Arena::operator new (size_t               size,
                                         tool::memory::Arena* arena)
{
  return arena->_allocate(size);
}

inline
void tool::memory::Arena::operator delete (void*                ptr,
                                           tool::memory::Arena& arena)
{
  arena._deallocate(ptr);
}

inline
void tool::memory::Arena::operator delete (void*                ptr,
                                           tool::memory::Arena* arena)
{
  arena->_deallocate(ptr);
}

inline
void* tool::memory::Arena::operator new[] (size_t, void* ptr)
{
  return ptr;
}

inline
ptrdiff_t  tool::memory::Arena::remaining() const
{
  return static_cast<char*>(_end) - static_cast<char*>(_ptr);
}

inline
void* const tool::memory::Arena::base() const
{
  return _base;
}

inline
void* const tool::memory::Arena::end() const
{
  return _end;
}

inline
void* tool::memory::Arena::_allocate(size_t size)
{
  char* ptr = reinterpret_cast<char*>(_ptr);
  char* end = ptr + size;
  if (end <= _end)  { _ptr = end; }
  else              {  ptr = 0;   }
  return ptr;
}
