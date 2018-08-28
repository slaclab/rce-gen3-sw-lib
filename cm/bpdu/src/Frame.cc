
#include "bpdu/Frame.hh"

#include <stdio.h>

using namespace cm::bpdu;

Frame::Frame() {}
/*
  protocolId   (0),      // Rapid Spanning Tree Protocol
  protocolVsn  (2),
  type         (2),
  flags        (0x7c),   //
  msg_age      (0),      // in units of 1/256 seconds
  max_age      (0x1400),
  hello        (0x200),
  forward_delay(0x0f00),
  version1_len (0)
*/

void Frame::dump() const
{
  char buff[32];

  printf("== BPDU Frame ==\n");
  printf("protocol Id/Vsn  %x/%x\n", protocolId, protocolVsn);
  printf("type/flags %x/%x\n", type,flags);

  rootId.dump(buff);
  printf("rootId %s  cost %x\n",buff,rootPathCost);

  bridgeId.dump(buff);
  printf("bridgeId %s  port %x\n",buff,portId);

  printf("msg/max age %x/%x\n",msg_age,max_age);
  printf("hello/fwd delay %x/%x\n",hello,forward_delay);
}
