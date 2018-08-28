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

using service::atca::Client;
using service::atca::Address;
using service::dsl::Location;

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
  
 char str[64];
 
 printf(CONTROL, result->layer3.addr_p(str)); 
  
}
