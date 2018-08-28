/*
** ++
**  Package:
**	
**  Abstract:
**
**  Author:
**      Michael Huffer, SLAC <mehsys@slac.stanford.edu>
**
**  Creation Date:
**	000 - , 2007
**
**  Revision History:
**	None.
**
** --
*/



#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "atca/Client.hh"
#include "atca/Address.hh"
#include "net/MacAddress.hh"

using service::atca::Client;
using service::atca::Address;
using service::dsl::Location;
using service::net::MacAddress;

const char* NONE    = "";
const char* CONTROL = "%s";

const char* OPT_IFADDR = "--ifaddr";
const char* OPT_IFNAME = "--ifname";

struct sockaddr_in sa;

int main(int argc, char** argv)
 {

 Location* result;
 
 if(argc < 2)
   {
   printf(NONE);
   return 0;
   }
 
 Address address(argv[1]);
 
 if (argc == 2)
   {
   Client client;   
   result = client.lookup(address);
   }
 else if (argc == 4)
   {
   if(!strcmp(argv[2],OPT_IFADDR))
     {
     if(!inet_pton(AF_INET, argv[3], &(sa.sin_addr)))
     {
       printf(NONE);
       return -1;
     }
     Client client(sa.sin_addr.s_addr);
     result = client.lookup(address);
     }
   else if( !strcmp(argv[2],OPT_IFNAME))
     {
     Client client(argv[3]);
     result = client.lookup(address);
     }
   else
     {
     printf(NONE);
     return -1;
     }
   }
  else
    {
    printf(NONE);
    return 0;
    }
  
  if(!result)
    {
    printf(NONE);
    return 0;
    }

 MacAddress mac(result->layer2);
 
 printf(CONTROL, mac.string());  
 
 return 0;
 }

