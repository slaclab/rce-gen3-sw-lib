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
**	000 - April 20, 2012
**
**  Revision History:
**	None.
**
** --
*/
 
#ifndef SERVICE_DSL_CLIENT
#define SERVICE_DSL_CLIENT



#include "net/Interface.hh"

#include "dsl/impl/Proxy.hh"

#include "dsl/Tag.hh"
#include "dsl/Location.hh"
#include "udp/Link.hh"

namespace service {
namespace dsl  {

class Client {
public: 
  Client(const char* space);
  Client(const char* space, const char *ifname);
  Client(const char* space, uint32_t ifaddr);  
public:
 ~Client() {}
public:
  Location* lookup(const Tag*);
public:
  uint64_t hash(const char* string) const;
private:
  uint64_t       _magic_cookie;
  const char*    _space;
  net::Interface _interface;  
  udp::Link      _link;
  Proxy          _proxy;
  uint8_t        _result[sizeof(Location)];
};

}}

#endif
