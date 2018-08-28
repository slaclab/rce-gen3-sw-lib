#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

#include "elf/linker.h"
#include "memory/cpu/resources.h"
#include "memory/mem.h"
#include "sas/Sas.h"
#include "xaui/Xaui.h"
#include "Socket.h"

#include "atca/Client.hh"
#include "atca/Address.hh"

using service::atca::Client;
using service::atca::Address;
using service::dsl::Location;

#if defined(__cplusplus)
extern "C" {
#endif

static const char Message[] = "Hello from xaui TX Example";

void Task_Start(int argc, const char** argv)
  {  
  Location* result;
   
  if(argc != 3) return;
    
  Address address(argv[1]);

  Client client;   

  result = client.lookup(address);

  uint32_t dst = result->layer3.addr();
 
  Socket *socket = new Socket(32);

  Datagram *datagram = new Datagram();
    
  strncpy((char*)datagram->content(),Message,sizeof(Message));

  uint32_t port = strtoul(argv[2], 0, 0);

  printf("Tx example send to ip dst 0x%x port 0x%x\n",dst,port);
  
  socket->sendTo(dst,port,(uint32_t*)datagram,128);

  printf("Tx example socket waiting for response...\n");

  Datagram *rx = socket->wait();

  if(!rx)
  {
  printf("Socket receive error\n");
  return;
  }

  printf("Tx example received new datagram 0x%x from host 0x%x port 0x%x len %d\n",
          rx,rx->host(),rx->port(),rx->length());

  uint32_t *data = (uint32_t*)rx->content();

  printf("Datagram message: %s\n",(char*)data);
  
  socket->close();
  }

void Task_Rundown()
{
}

#if defined(__cplusplus)
} // extern "C"
#endif

