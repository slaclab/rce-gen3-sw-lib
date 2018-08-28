
#include "cm_net/DLFHandler.hh"

#include "fm/FmLCIPacket.hh"
#include "fm/FmTahoeMgr.hh"

#include <netinet/in.h>
#include <sys/types.h>
#include <net/ethernet.h> // for ethernet type defines

#include <string.h>
#include <stdio.h>
#include <errno.h>

using namespace cm::net;

static const int DLFPriority = 1;

DLFHandler::DLFHandler(FmTahoeMgr& m) : _fm(m)
{
  // set a trigger to mirror DLFs to the CPU
  FmTriggerConfig cfg;
  cfg.mac_entry_trigger = 0;
  cfg.new_priority = DLFPriority;
  cfg.mirror_port = FmPort(FmPort::CPU_PORT);
  //  cfg.action = FmTriggerConfig::Mirror;
  cfg.action = FmTriggerConfig::MirrorAtNewPriority;
  cfg.vlanMatch = 0;
  cfg.sourceMacMatch      = FmTriggerConfig::Either;
  cfg.destinationMacMatch = FmTriggerConfig::Miss;
  cfg.priorityMask  = 0;
  cfg.sourcePortMask      = FmPortMask( 0 );
  cfg.destinationPortMask = FmPortMask( 0 );
  cfg.enable = 1;

  unsigned id;

  cfg.xcast  = FmTriggerConfig::Multicast;
  id = m.lowestPriorityUnusedTrigger();
  m.configureTrigger(id, cfg);

  cfg.xcast  = FmTriggerConfig::Unicast;
  id = m.lowestPriorityUnusedTrigger();
  m.configureTrigger(id, cfg);
}

DLFHandler::~DLFHandler() {}

bool DLFHandler::handle(const FmLCIPacket& pkt)
{
//   struct ether_header& eh = *reinterpret_cast<struct ether_header*>(pkt.payload());
//   if (eh.ether_dhost[0]!=0x01)
//     return false;

  if (pkt.priority() != DLFPriority)
    return false;

  const char* p = reinterpret_cast<const char*>(pkt.payload());

  printf("DLFHandler: port %d/0x%x bytes 0x%x",
	 _fm.id(), unsigned(pkt.source_port()), pkt.payload_size());

  printf(" dst");
  for(unsigned i=0; i<6; i++,p++) printf("%c%02x",(i==0)?' ':':',*p);

  printf(" src");
  for(unsigned i=0; i<6; i++,p++) printf("%c%02x",(i==0)?' ':':',*p);

  printf(" tln %02x%02x\n", p[0],p[1]);

  return true;
}
