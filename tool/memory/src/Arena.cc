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
* $Revision: 3125 $
*
* @verbatim                    Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#include <cstdlib>
#include <stdio.h>
#include <inttypes.h>



#include "debug/print.h"
#include "memory/Arena.hh"


namespace tool
{
  namespace memory
  {

Arena* Arena::create(void* const base, size_t size, bool loadPattern)
{
  Arena* arena = ::new Arena(base, size, loadPattern);
  if (arena == NULL)
    dbg_bugcheck("Arena allocation failed for base %p, size %d",
                             base, size);
  return arena;
}

Arena::Arena(void* const base, size_t size, bool loadPattern) :
  _base(base),
  _ptr(base),
  _end(static_cast<void*>(static_cast<uint8_t*>(base) + size))
{
  if (loadPattern)  _loadPattern();
}

Arena::Arena(void* const base, void* const end, bool loadPattern) :
  _base(base),
  _ptr(base),
  _end(end)
{
  if (loadPattern)  _loadPattern();
}

void Arena::dump() const
{
  uint32_t inUse = static_cast<uint8_t*>(_ptr) - static_cast<uint8_t*>(_base);
  uint32_t left  = remaining();
  uint32_t total = static_cast<uint8_t*>(_end) - static_cast<uint8_t*>(_base);

  printf("Arena %p: end: %p, size: 0x%08x, current ptr: %p, "
         "in use: 0x%08x = %.2f%%, remaining: 0x%08x = %.2f%%\n",
         _base, _end, unsigned(total), _ptr,
         unsigned(inUse), (100.0 * inUse) / total,
         unsigned(left),  (100.0 * left) / total );
}

void Arena::_deallocate(void* /*ptr*/)
{
  // Can't deallocate, so attempting to constitutes a bug
  dbg_bugcheck("");
}

void Arena::_loadPattern()
{
  // Load a pattern onto the space
  uint32_t* ptr = reinterpret_cast<uint32_t*>(_base);
  uint32_t  cnt = 0;
  do
  {
    *ptr++ = (0xffu << 24) | (++cnt & 0xff) << 16;
  } while (ptr < _end);
}

  }; // memory
}; // tool
