
#include "cm_net/LACPDUHandler.hh"

#include "fm/FmLCIPacket.hh"
#include "fm/FmTahoeMgr.hh"

#include <netinet/in.h>
#include <sys/types.h>
#include <net/ethernet.h> // for ethernet type defines

#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "lacp/Frame.hh"
#include "lacp/Manager.hh"

using namespace cm::net;

#define DUMP

LACPDUHandler::LACPDUHandler(FmTahoeMgr& m,
                             cm::net::Driver& drv) :
  _mgr (new cm::lacp::Manager(m,drv)),
  _id  (m.id()) {}

LACPDUHandler::~LACPDUHandler() { delete _mgr; }

bool LACPDUHandler::handle(const FmLCIPacket& pkt)
{
  struct ether_header& eh = *reinterpret_cast<struct ether_header*>(pkt.payload());
  if (eh.ether_type > ETHER_MAX_LEN)
    return false;

  if (!cm::lacp::is_lacp(eh)) 
    return false;

  const cm::lacp::Frame& f = *reinterpret_cast<const cm::lacp::Frame*>(&eh+1);

  return true;
}
