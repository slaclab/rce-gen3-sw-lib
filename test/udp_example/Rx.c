#include <stdio.h>

#include "elf/linker.h"
#include "memory/cpu/resources.h"
#include "memory/mem.h"
#include "sas/Sas.h"
#include "xaui/Xaui.h"
#include "Socket.h"

#if defined(__cplusplus)
extern "C" {
#endif

static const char Message[] = "Hello back from xaui RX Example";

void Task_Start(int argc, const char** argv)
  {

  Socket *socket = new Socket(32);
  
  printf("Rx example socket waiting...\n");
  
  Datagram *rx = socket->wait();
  
  if(!rx)
  {
  printf("Socket receive error\n");
  return;
  }

  printf("Rx example received new datagram 0x%x from host 0x%x port 0x%x len %d\n",
          rx,rx->host(),rx->port(),rx->length());

  uint32_t *data = (uint32_t*)rx->content();

  printf("Datagram 0x%x message: %s\n",rx,(char*)data);

  uint32_t dst = rx->host();

  Datagram *datagram = new Datagram();

  strncpy((char*)datagram->content(),Message,sizeof(Message));

  uint32_t port = rx->port();

  socket->sendTo(dst,port,(uint32_t*)datagram,128); 
  
  socket->close(); 
}

void Task_Rundown()
{
}

#if defined(__cplusplus)
} // extern "C"
#endif

