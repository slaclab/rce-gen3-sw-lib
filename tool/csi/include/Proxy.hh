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
 
#ifndef TOOL_CSI_PROXY
#define TOOL_CSI_PROXY

#include <inttypes.h>

#include "container/LinkedList.hh"
#include "memory/FreeList.hh"
          
#include "csi/Link.hh"
#include "csi/impl/Transaction.hh"

namespace tool {
namespace csi  {

class Proxy {
public: 
  Proxy(Link*, uint32_t timout, uint32_t retries);
public:
  virtual ~Proxy() {}
public:  
  void* request() {return _freelist.allocate();}
protected:
  void  _post(void* request, uint32_t length);
  void* _wait();
private:
  virtual void     _tidIs(uint32_t value, void* request)  = 0; 
  virtual uint32_t _tid(void* packet) const               = 0;  
private:
  enum {MAX_PENDING = 8};
private: 
  void* _rundown(void* response); 
  void* _rundown(Transaction*); 
private:
  tool::container::linkedList::List<Transaction> _inflight;
  Link*                     _link;
  uint32_t                  _next_tid;
  uint32_t                  _timout; 
  uint32_t                  _retries; 
  memory::FreeList            _freelist;
  Transaction*              _pending[MAX_PENDING];
};

}}

#endif
