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

inline void* PoolEntry::operator new(size_t size, char* p)
  {
  return (void*)p;
  }

/*
** ++
**
**
** --
*/

inline PoolEntry::PoolEntry(Pool* pool):
  _pool(pool),
  _tag(0xffffffff)
  {
  }

/*
** ++
**
**
** --
*/

inline PoolEntry* PoolEntry::entry(void* buffer)
  {
  return (PoolEntry*)buffer - 1;
  }

  }; // memory
}; // tool
