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

namespace tool {
  namespace memory {

/*
** ++
**
**
** --
*/

inline
GenericPool::GenericPool(size_t sizeofObject, int numberofObjects) :
  Pool(sizeofObject, numberofObjects),
  _bounds(sizeofAllocate()*numberofObjects),
  _buffer(new char[_bounds]),
  _current(0)
{
populate();
}

/*
** ++
**
**
** --
*/

inline GenericPool::~GenericPool()
  {
  delete[] _buffer;
  }

/*
** ++
**
**
** --
*/

inline void* GenericPool::deque()
  {
  PoolEntry* entry = remove();
  return (entry != empty()) ? (void*)&entry[1] : (void*)0;
  }

/*
** ++
**
**
** --
*/

inline void GenericPool::enque(PoolEntry* entry)
  {
  insert(entry);
  }

/*
** ++
**
**
** --
*/

inline void* GenericPool::allocate(size_t size)
  {
  int offset  = _current;
  char* entry = _buffer + offset;

  if ((offset += size) <= _bounds)
    {
    _current = offset;
    return (void*) entry;
    }
  else
    {
    return (void*) 0;
    }
  }

  }; // memory
}; // tool
