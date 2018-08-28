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
#ifndef TOOL_MEMORY_ARENA_HH
#define TOOL_MEMORY_ARENA_HH

#include <stddef.h>

#include <new>




namespace tool
{
  namespace memory
  {
    class Arena
    {
    public:
      static Arena* create(void* const base, size_t size, bool loadPattern = false);
      static Arena* instance(uint32_t id);

    protected:
      Arena(void* const base, size_t size,     bool loadPattern = false);
      Arena(void* const base, void* const end, bool loadPattern = false);
    public:
      Arena();
      ~Arena();

      void* operator new (size_t size, uint32_t id);
      void* operator new (size_t size, Arena& arena);
      void* operator new (size_t size, Arena* arena);
      void  operator delete (void * ptr, Arena& arena);
      void  operator delete (void * ptr, Arena* arena);
      void* operator new[] (size_t, void* ptr);

      ptrdiff_t   remaining() const;
      void* const base()      const;
      void* const end()       const;
      void        dump()      const;

    private:
      void* _allocate(size_t size);
      void  _deallocate(void* ptr);

      void  _loadPattern();

    private:
      void* const _base;
      void*       _ptr;
      void* const _end;
    };

  }; // memory
}; // tool

#include "memory/impl/Arena-inl.hh"

#endif // TOOL_MEMORY_ARENA_HH
