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
 
#ifndef SERVICE_ATCA_CLIENT
#define SERVICE_ATCA_CLIENT



#include "dsl/Client.hh"
#include "atca/Address.hh"

namespace service {
namespace atca  {

class Client {
public: 
  Client();
  Client(uint32_t ifaddr);
  Client(const char* ifname);  
public:
 ~Client() {}
public:
  dsl::Location* lookup(const Address&);
private:
   dsl::Client _client;
};

}}

#endif
