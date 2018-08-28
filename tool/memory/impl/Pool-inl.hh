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

inline void* Pool::alloc(size_t size)
  {
  _numberofAllocs++;
  return (size > _sizeofObject) ? (void*)0 : deque();
  }

/*
** ++
**
**
** --
*/

inline void Pool::free(void* buffer)
  {
  Pool* pool = (PoolEntry::entry(buffer))->_pool;
  pool->enque(PoolEntry::entry(buffer));
  pool->_numberofFrees++;
  }

/*
** ++
**
**
** --
*/

inline Pool::~Pool()
  {
  }

/*
** ++
**
**
** --
*/

inline size_t Pool::sizeofObject() const
  {
  return _sizeofObject;
  }

/*
** ++
**
**
** --
*/

inline size_t Pool::sizeofAllocate() const
  {
  return _quanta;
  }

/*
** ++
**
**
** --
*/

inline int Pool::numberofObjects() const
  {
  return _numberofObjects;
  }

/*
** ++
**
**
** --
*/

inline int Pool::numberofAllocs() const
  {
  return _numberofAllocs;
  }

/*
** ++
**
**
** --
*/

inline int Pool::numberofFrees() const
  {
  return _numberofFrees;
  }

  }; // memory
}; // tool
