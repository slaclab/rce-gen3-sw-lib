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

#ifndef TOOL_MEMORY_GENERICPOOL_HH
#define TOOL_MEMORY_GENERICPOOL_HH


#include "memory/Pool.hh"
#include "container/Queue.hh"


namespace tool {
  namespace memory {

class GenericPool : public Queue<PoolEntry>, public Pool
  {
  public:
    GenericPool(size_t sizeofObject, int numberofObjects);
    ~GenericPool();
  private:
    virtual void* deque();
    virtual void  enque(PoolEntry*);
    virtual void* allocate(size_t size);
  private:
    int   _bounds;
    char* _buffer;
    int   _current;
  };

  }; // memory
}; // tool

#include "memory/impl/GenericPool-inl.hh"

#endif // TOOL_MEMORY_GENERICPOOL_HH
