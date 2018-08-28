/*
** ++
**  Package:
**	
**
**  Abstract:
**      
**
**  Author:
**      Michael Huffer, SLAC (mehsys@slac.stanford.edu)
**
**  Creation Date:
**	000 - January 09, 2007
**
**  Revision History:
**	None.
**
** --
*/

#include <stdlib.h>

#include "memory/FreeList.hh"

using namespace tool::memory;

/*
** ++
**
**
** --
*/

FreeList::FreeList(unsigned size, unsigned number) :
 _buffer(),
 _base((char*)malloc(size*number)),  
 _number(number),
 _size(size),
 _reserved(0)
 {

 unsigned remaining = _number;
 unsigned quanta    = _size;
 char*    next      = _base;

 if(quanta < sizeof(Dummy)) return;

 while(remaining--)
    {
    _buffer.insert((Dummy*)next);
    next += quanta;
    }
 }

/*
** ++
**
**
** --
*/
 
void* FreeList::allocate()
 {

 Dummy* entry = _buffer.remove();

 return entry != _buffer.empty() ? (void*)entry : (void*)0;
 }

/*
** ++
**
**
** --
*/

void FreeList::deallocate(void* entry)
 {

 _buffer.insert((Dummy*)entry);

 return;
 }

/*
** ++
**
**
** --
*/

FreeList::~FreeList()
 {
 free((void*)_base);
 }
