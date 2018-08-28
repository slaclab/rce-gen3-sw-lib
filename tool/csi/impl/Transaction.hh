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

#ifndef TOOL_CSI_TRANSACTION
#define TOOL_CSI_TRANSACTION

#include <inttypes.h>
#include <stddef.h>

#include "container/LinkedList.hh"

namespace tool {
namespace csi  {

  class Transaction : public tool::container::linkedList::Entry<Transaction> {
private:
  enum {MAX_REQUEST_SIZE = 8192};
public:
  static void*    buffer(void* request)  {return (void*)((uint8_t*)request + MAX_REQUEST_SIZE);} 
  static uint32_t sizeof_request()       {return sizeof(Transaction)       + MAX_REQUEST_SIZE;} 
public:
  Transaction(uint32_t length, uint32_t timout, uint32_t retries) : _timout(timout), _remaining(retries), _length(length) {}
public:
 ~Transaction() {}
public:
  uint32_t retry() {uint32_t remaining = _remaining; if(remaining) {_timout <<= 1; _remaining = remaining - 1;} return remaining;}
public: 
  void*    request() const {return (void*)((uint8_t*)this - MAX_REQUEST_SIZE);}
  uint32_t length()  const {return _length;}
  uint32_t timout()  const {return _timout;}
public:
  void* operator new(size_t size, void* buffer) {return buffer;}
private:
  uint32_t  _timout;
  uint32_t  _remaining;
  uint32_t  _length;
};

}}

#endif
