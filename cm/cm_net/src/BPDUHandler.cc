
#include "cm_net/BPDUHandler.hh"

#include "fm/FmLCIPacket.hh"
#include "fm/FmTahoeMgr.hh"

#include <netinet/in.h>
#include <sys/types.h>
#include <net/ethernet.h> // for ethernet type defines

#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "bpdu/Frame.hh"
#include "bpdu/RSTPManager.hh"

using namespace cm::net;

#define DUMP

BPDUHandler::BPDUHandler(FmTahoeMgr& m,
			 cm::net::Driver& drv) :
  _rstp(new cm::bpdu::RSTPManager(m,drv)),
  _id  (m.id()) {}

BPDUHandler::~BPDUHandler() { delete _rstp; }

bool BPDUHandler::handle(const FmLCIPacket& pkt)
{
  struct ether_header& eh = *reinterpret_cast<struct ether_header*>(pkt.payload());
  if (eh.ether_type > ETHER_MAX_LEN)
    return false;

  if (memcmp(reinterpret_cast<const char*>(eh.ether_dhost),
	     reinterpret_cast<const char*>(&cm::bpdu::bpdu_mac_address), 6))
    return false;

#ifdef DUMP
  static int ndump=6;

  if (ndump) {
    printf("BPDUHandler: sw %d  port 0x%x  ethertype 0x%x\n",
	   _id, unsigned(pkt.source_port()), eh.ether_type);

    const unsigned* p = reinterpret_cast<unsigned*>(pkt.payload());
    const unsigned* e = p + (pkt.payload_size()>>2);
    while( p < e ) {
      printf(" %08x %08x %08x %08x\n",*p, *(p+1), *(p+2), *(p+3));
      p += 4;
    }

    reinterpret_cast<const cm::bpdu::Frame*>(&eh+1)->dump();
    ndump--;
  }
  return true;
#else

  const cm::bpdu::Frame& bf = *reinterpret_cast<const cm::bpdu::Frame*>(&eh+1);

  // 802.1D-2004 Clause 9.3.4 (a)
//   if (bf.type==cm::bpdu::Configuration && eh.length_type>34 && (bf.msg_age < bf.max_age) &&
//       bf.bridgeId != _bridgeId &&
//       bf.portId&0x3ff != pkt.source_port()) {
  if (bf.type==cm::bpdu::Configuration) ;
  // 802.1D-2004 Clause 9.3.4 (b)
  else if (bf.type==cm::bpdu::Change) ;
  // 802.1D-2004 Clause 9.3.4 (c)
  else if (bf.type==cm::bpdu::RSTP)
    _rstp->rcvdBpdu(bf,pkt.source_port());
  else
    return false;

  return true;
#endif
}
