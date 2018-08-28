/*
** ++
**  Package:
**      Tool
**
**  Abstract:
**
**  Author:
**      Michael Huffer, SLAC, (415) 926-4269
**
**  Creation Date:
**      000 - December 20 1,1997
**
**  Revision History:
**      None.
**
** --
*/

#ifndef TOOL_MEMORY_POOL_HH
#define TOOL_MEMORY_POOL_HH


#include "memory/PoolEntry.hh"

namespace tool {
  namespace memory {

class Pool
  {
  public:
    virtual ~Pool();
    Pool(size_t sizeofObject, int numberofOfObjects);
    void*         alloc(size_t size);
    static void   free(void* buffer);
    size_t        sizeofObject()    const;
    int           numberofObjects() const;
    int           numberofAllocs()  const;
    int           numberofFrees()   const;
  protected:
    size_t        sizeofAllocate()  const;
    virtual void* deque()               = 0;
    virtual void  enque(PoolEntry*)     = 0;
    virtual void* allocate(size_t size) = 0;
    void          populate();
  private:
    size_t        _sizeofObject;
    int           _numberofObjects;
    int           _numberofAllocs;
    int           _numberofFrees;
    int           _remaining;
    size_t        _quanta;
  };

  }; // memory
}; // tool


#include "memory/impl/Pool-inl.hh"

#endif // TOOL_MEMORY_POOL_HH
