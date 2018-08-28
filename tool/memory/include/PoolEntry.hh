/*
** ++
**  Package:
**	Tool
**
**  Abstract:
**
**  Author:
**      Michael Huffer, SLAC, (415) 926-4269
**
**  Creation Date:
**	000 - December 20 1,1997
**
**  Revision History:
**	None.
**
** --
*/

#ifndef TOOL_MEMORY_POOLENTRY_HH
#define TOOL_MEMORY_POOLENTRY_HH

#include <stddef.h>                     // for size_t

namespace tool {
  namespace memory {

class Pool;           // Necessary to resolve forward reference...

class PoolEntry
  {
  public:
    PoolEntry(Pool*);
    void* operator new(size_t, char*);
    static PoolEntry* entry(void* buffer);
    void*         _opaque[2];
    Pool*         _pool;
    unsigned long _tag;
  };

  }; // memory
}; // tool


#include "memory/impl/PoolEntry-inl.hh"

#endif  // TOOL_MEMORY_POOLENTRY_HH
