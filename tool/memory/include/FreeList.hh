/*
** ++
**  Package:
**  
**
**  Abstract:
**
**  Author:
**      Michael Huffer, SLAC, (415) 926-4269
**
**  Creation Date:
**	000 - June 20 1,1997
**
**  Revision History:
**	None
**
** --
*/

#ifndef TOOL_MEMORY_FREELIST
#define TOOL_MEMORY_FREELIST

#include "container/LinkedList.hh"

#include <inttypes.h>

namespace tool {
namespace memory {

/*
** ++
**
**
** --
*/

class Dummy : public container::linkedList::Entry<Dummy> {
public:
  Dummy() {} 
public:
 ~Dummy() {}
};

/*
** ++
**
**
** --
*/

class FreeList {
public:
  FreeList(unsigned size, unsigned number); 
public:
 ~FreeList();
public:
  void* allocate();  
  void  deallocate(void*);
public:
  bool     probe()        {return _buffer.head() != _buffer.empty();}
  unsigned size()   const {return _size;}     
  unsigned number() const {return _number;}
private:
  container::linkedList::List<Dummy> _buffer;
  char*               _base;  
  unsigned            _number;
  unsigned            _size;
  unsigned            _reserved;
};

}}

#endif
