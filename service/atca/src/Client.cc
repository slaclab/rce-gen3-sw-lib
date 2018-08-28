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



#include "conversion/BSWP.h"
#include "conversion/impl/BSWP.ih"
#include "atca/Client.hh"

using service::dsl::Location;
using service::dsl::Tag;

using namespace service::atca;

const char* space_name = "DSL_ATCA";

/** @brief Construct the DSL ATCA client.
 */
Client::Client() :
 _client(space_name)
 {
 }

/** @brief Construct the DSL ATCA client, bound to a particular IPV4 address.
    @param ifaddr The IPV4 address in network byte order
 */
Client::Client(uint32_t ifaddr) :
 _client(space_name,ifaddr)
 {
 }

/** @brief Construct the DSL ATCA client, bound to a particular network interface.
    @param ifname The network interface name.
 */
Client::Client(const char *ifname) :
 _client(space_name,ifname)
 {
 }

/** @brief Look up the RCE at a particular Address.
    @param address The RCE's address to look up.
    @return A pointer to the Location of the RCE, or 0 if there is no response.

    Constructs and sends a UDP packet that queries the existence of
    a particular RCE.  If a remote Server self identifies as that RCE,
    it will send a packet back with IP and ethernet information.

    If no response is received (timeout) then zero is returned.
 */
Location* Client::lookup(const Address& address)
 {

 if(!address.isValid()) return (Location*)0;
 
 Tag tag;

 /* store tag values in network order */
 tag.value.w64[0] = BSWP__swap64b(_client.hash(address.shelf()));
 tag.value.w32[2] = BSWP__swap32b(address.slot());
 tag.value.w32[3] = BSWP__swap32b(address.cmb());
 tag.value.w32[4] = BSWP__swap32b(address.element());
 
 return _client.lookup(&tag);
 }


namespace service {
  namespace atca {

/**
   @class Client
   @brief The client for the DSL ATCA service.

 */


  }
}
