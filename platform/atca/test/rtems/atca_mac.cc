/*
** ++
**  Package:
**	
**
**  Abstract:
**      
**
**  Author:
**      Sergio Maldonado, SLAC <smaldona@slac.stanford.edu>
**
**  Creation Date:
**	    000 - May 24, 2012
**
**  Revision History:
**	    None.
**
** --
*/



#include "stdio.h"

#include "atca/Client.hh"
#include "atca/Address.hh"
#include "net/MacAddress.hh"

using service::atca::Client;
using service::atca::Address;
using service::dsl::Location;
using service::net::MacAddress;

const char* NONE    = "";
const char* CONTROL = "%s";
const char* BOARD = "keynot/5/2/0";

extern "C" void rce_appmain(void*) {
 
 Address address(BOARD);

 Client client;
 
 const Location* result = client.lookup(address);  
 
 if(!result)
   {
   printf(NONE);
   return;
   }
  
 MacAddress mac(result->layer2);
 
 printf(CONTROL, mac.string());  
  
}
