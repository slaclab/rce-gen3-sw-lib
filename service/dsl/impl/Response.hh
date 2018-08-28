
/*
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
**	    000 - April 06, 2011
**
**  Revision History:
**	    None.
**
** --
*/
 
#ifndef SERVICE_DSL_RESPONSE
#define SERVICE_DSL_RESPONSE



#include "dsl/impl/Message.hh"
#include "dsl/Location.hh"

namespace service {
namespace dsl  {

class Response : public Message { 
public:
  Response(uint64_t magic_cookie, uint32_t tid, const Location& value) : Message(magic_cookie, tid), result(value) {} 
public:
 ~Response() {}
public:
  void* operator new(size_t size, uint8_t* buffer) {return (void*)buffer;} 
public:
  Location result;
};

}}

#endif

