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



#include <string.h>

#include "kvt/Kvt.h"
#include "dsl/impl/Request.hh"
#include "dsl/impl/Response.hh"
#include "dsl/impl/Message.hh"
#include "dsl/Client.hh"

#define SEED ((uint32_t)1771875)

using namespace service::dsl;

const char* CLIENT_MAGIC_COOKIE = "$$MAGIC_COOKIE$$";

/*
** ++
**
**
** --
*/

Client::Client(const char* space) :
 _magic_cookie(hash(CLIENT_MAGIC_COOKIE)),
 _space(space),
 _interface(),  
 _link(_interface.bcAddr(), SERVER_PORT),
 _proxy(&_link)
 {
 }

/*
** ++
**
**
** --
*/

Client::Client(const char* space, const char *ifname=0) :
 _magic_cookie(hash(CLIENT_MAGIC_COOKIE)),
 _space(space),
 _interface(ifname),
 _link(_interface.bcAddr(), SERVER_PORT),
 _proxy(&_link)
 {
 }

/*
** ++
**
**
** --
*/

Client::Client(const char* space, uint32_t ifaddr) :
 _magic_cookie(hash(CLIENT_MAGIC_COOKIE)),
 _space(space),
 _link(ifaddr, SERVER_PORT),
 _proxy(&_link)
 {
 }

Location* Client::lookup(const Tag* tag)
  {
 
  Request* request = new(_proxy.request()) Request(_magic_cookie, _space, tag);
  
  _proxy.post(request);
 
  Response* response = _proxy.wait();
 
  return response ? new(_result) Location(response->result, _link.fixup()) : (Location*)0; 
  }

/*
** ++
**
**
** --
*/

uint64_t Client::hash(const char* string) const      
  {
  return KvtHash(string);
  }
