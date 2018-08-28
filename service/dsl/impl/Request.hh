
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
 
#ifndef SERVICE_DSL_REQUEST
#define SERVICE_DSL_REQUEST



#include <string.h>

#include "conversion/BSWP.h"
#include "conversion/impl/BSWP.ih"

#include "dsl/impl/Message.hh"
#include "dsl/Tag.hh"
#include "dsl/Space.hh"

namespace service {
namespace dsl  {

class Request : public Message {
public:
  enum {MAX_SPACE_NAME = 64};
public:
  Request(uint64_t magic_cookie, const char *space, const Tag* tag) : Message(magic_cookie), _tag(*tag) {strncpy((char *)_space,space,sizeof(_space));}
public:
 ~Request() {}
public:
  const Tag* tag() {return &_tag;}
public:
  const char* space() const {return (const char *)_space;}    
public:
  void* operator new(size_t size, void* buffer) {return buffer;}
private:
  uint8_t  _space[MAX_SPACE_NAME];
  Tag      _tag;
};

}}

#endif

