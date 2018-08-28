
#include "cm_net/PauseHandler.hh"

#include "fm/FmLCIPacket.hh"

#include <netinet/in.h>
#include <sys/types.h>
#include <net/ethernet.h> // for ethernet type defines

#include <string.h>
#include <stdio.h>
#include <errno.h>

#define ETHERTYPE_PAUSE 0x0888

namespace cm {
  namespace net {
    class PauseFrame {
    public:
      ether_header hdr;        // dst, src, length_type(0x0888)
      uint16_t   opcode;       // 0x0100
      uint16_t   length;       // 512 bit times
      uint16_t   reserved[3];  // pad
    };
  };
};

using namespace cm::net;

struct ether_addr PAUSE_MACDST = { {'\1','\x80','\xc2','\0','\0','\1'} };

PauseHandler::PauseHandler() {}

PauseHandler::~PauseHandler() {}

bool PauseHandler::handle(const FmLCIPacket& pkt)
{
  struct ether_header& eh = *reinterpret_cast<struct ether_header*>(pkt.payload());
  if (eh.ether_type != ETHERTYPE_PAUSE)
    return false;

  if (memcmp(reinterpret_cast<const char*>(eh.ether_dhost),
	     reinterpret_cast<const char*>(&PAUSE_MACDST), 6))
    return false;

  const PauseFrame& pf = *reinterpret_cast<const PauseFrame*>(&eh+1);

  printf("PauseHandler: port 0x%x  pause 0x%x  512-bit times\n",
	 unsigned(pkt.source_port()), pf.length);

  return true;
}
