//
//  IGMPv3Handler
//
//    Floods the IGMPv3 group queries and reports.  Also snoops the IGMPv3 reports to learn about
//    membership and adjust the MAC table for multicast data routing.  Note that this will break
//    any non-IGMPv3 multicast routing, because report flooding will cause IGMPv2 hosts to suppress
//    their membership reports.  Report flooding is how we let neighbor L2 switches learn of group
//    membership.
//

#include "cm_net/IGMPv3Handler.hh"

#include "cm_net/Service.hh"
#include "IGMP.hh"
#include "fm/FmLCIPacket.hh"
#include "fm/FmTahoe.hh"
#include "fm/FmTahoeMgr.hh"
#include "fm/FmMacTable.hh"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/ethernet.h> // for ethernet type defines
typedef u_long n_long;
#include <netinet/ip.h>
#include <unistd.h>

#include <string.h>
#include <stdio.h>
#include <errno.h>

//#define DBUG

using namespace cm::net;

const unsigned    IGMPv3_REPORT_GROUP  = 0xe0000016;
struct ether_addr IGMPv3_REPORT_MACDST = { {'\1','\0','\x5e','\0','\0','\x16'} }; // { 1, 0, 0x5e, 0, 0, 0x16 };
struct ether_addr IGMP_ALL_HOSTS_MACDST = { {'\1','\0','\x5e','\0','\0','\1'} }; // { 1, 0, 0x5e, 0, 0, 0x01 };

static const SwmMacTableEntry REPORT_ENTRY ( (ether_addr_t&)IGMPv3_REPORT_MACDST,
					     FmPortMask::allPorts(), 0, 0, 1, 1);
static const SwmMacTableEntry HOSTS_ENTRY  ( (ether_addr_t&)IGMP_ALL_HOSTS_MACDST,
					     FmPortMask::allPorts(), 0, 0, 1, 1);
static const SwmMacTableEntry ROUTERS_ENTRY( (ether_addr_t) {{'\1','\0','\x5e','\0','\0','\x02'}},
					     FmPortMask::phyPorts(), 0, 0, 1, 1);
static const SwmMacTableEntry PIM_ENTRY    ( (ether_addr_t) {{'\1','\0','\x5e','\0','\0','\x0d'}},
					     FmPortMask::phyPorts(), 0, 0, 1, 1);

/*
static void dumpMAC(const u_char* mac, char c)
{
  printf("%02x:%02x:%02x:%02x:%02x:%02x%c",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],c);
}
*/

#define ADD_MAC_ENTRY( entry ) {		       \
  if ( (index=_fm.macTable().findMatch( entry )) >= 0) \
    _fm.macTable().updateEntry( index, entry );	       \
  else						       \
    index = _fm.macTable().addEntry( entry );	       \
  }

IGMPv3Handler::IGMPv3Handler(FmTahoeMgr& fm,
			     unsigned    ip) :
  _fm(fm),
  _routerPort(FmPort::CPU_PORT),
  _trigger_id(-1)
{
  //
  //  Create the MAC entry
  //
  int index;
  ADD_MAC_ENTRY( REPORT_ENTRY );
  ADD_MAC_ENTRY( ROUTERS_ENTRY );
  ADD_MAC_ENTRY( HOSTS_ENTRY );
  ADD_MAC_ENTRY( PIM_ENTRY );

  //
  //  Join the IGMPv3 REPORT_GROUP.  (The network stack will not forward any messages to this socket)
  //
  { sockaddr_in servaddr;
    servaddr.sin_family      = AF_INET;
    servaddr.sin_port        = htons(0);
    servaddr.sin_addr.s_addr = IGMPv3_REPORT_GROUP;

    struct ip_mreq ipMreq;
    memset ((char*)&ipMreq, 0, sizeof(ipMreq));
    ipMreq.imr_multiaddr.s_addr = IGMPv3_REPORT_GROUP;
    ipMreq.imr_interface.s_addr = ip;

    printf("IGMPv3Handler joining %x on i/f %x\n",IGMPv3_REPORT_GROUP,ip);

    int result;
    if ((_socket = ::socket(AF_INET, SOCK_DGRAM, 0))<0)
      printf("IGMPv3Handler error opening socket\n");
    else if ((result=::bind(_socket, (sockaddr*)&servaddr, sizeof(servaddr)))<0)
      printf("IGMPv3Handler Error binding socket: %s\n", strerror(result));
    else if ((result=::setsockopt(_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
				  (char*)&ipMreq, sizeof(ipMreq)))<0)
      printf("IGMPv3Handler Error joining on socket: %s\n", strerror(result));
  }

  fm.trapIGMP(false);
}

IGMPv3Handler::~IGMPv3Handler()
{
  if (_trigger_id >= 0) {
    FmTriggerConfig& t = _fm._config.trigCfg[_trigger_id];
    t.enable = 0;
    _fm.configureTrigger(_trigger_id,t);
  }

  if (_socket>=0) {
    printf("IGMPv3Handler resigning %x\n",IGMPv3_REPORT_GROUP);
    ::close(_socket);  // resigns
  }

  //
  //  Remove the MAC entry
  //
  _fm.macTable().deleteEntry( ROUTERS_ENTRY );
  _fm.macTable().deleteEntry( REPORT_ENTRY );
//   _fm.macTable().deleteEntry( QUERY_ENTRY );
}

//
//  Setup trigger for multicast DLFs
//
void IGMPv3Handler::_setup_trigger()
{
  if (_trigger_id < 0)
    _trigger_id = _fm.lowestPriorityUnusedTrigger();

  FmTriggerConfig cfg;
  cfg.mac_entry_trigger   = 0;
  cfg.new_priority        = 1;
  cfg.mirror_port         = _routerPort;
  cfg.action              = FmTriggerConfig::RedirectToMirror,
  cfg.vlanMatch           = 0;
  cfg.sourceMacMatch      = FmTriggerConfig::Either;
  cfg.destinationMacMatch = FmTriggerConfig::Miss;
  cfg.priorityMask        = 0;
  cfg.sourcePortMask      = FmPortMask( unsigned(FmPortMask::allPorts()) & ~(1<<_routerPort));
  cfg.destinationPortMask = FmPortMask( 0 );
  cfg.enable              = 1;

  cfg.xcast  = FmTriggerConfig::Multicast;
  _fm.configureTrigger(_trigger_id, cfg);
}

//
//  Multicast listeners send IGMPv3 Reports to 224.0.0.22.
//  We'll snoop on those packets and add those listeners to the forwarding table for
//  the multicast address(es) listed in the contents.
//  Return false if some other handler may see it.
//  Return true if no other handler needs to see it.
//
bool IGMPv3Handler::handle(const FmLCIPacket& pkt)
{
  struct ether_header& eh = *reinterpret_cast<struct ether_header*>(pkt.payload());

  /*
  //  dump ether header
  printf("IGMPH: ");
  dumpMAC(eh.ether_dhost,' ');
  dumpMAC(eh.ether_shost,' ');
  printf("%d/%d\n", _fm.dev().id(),unsigned(pkt.source_port()));
  */

  if (eh.ether_type != ETHERTYPE_IP)
    return false;

  if (memcmp(reinterpret_cast<const char*>(eh.ether_dhost),
	     reinterpret_cast<const char*>(&IGMP_ALL_HOSTS_MACDST), 6)==0) {
    if (_routerPort != pkt.source_port()) {
      printf("Removing router port %x\n",unsigned(_routerPort));

      _routerPort = pkt.source_port();
      _setup_trigger();

      printf("Discovered router port %d/%x\n",_fm.dev().id(),unsigned(_routerPort));
    }
    return false;
  }

  if (memcmp(reinterpret_cast<const char*>(eh.ether_dhost),
	     reinterpret_cast<const char*>(&IGMPv3_REPORT_MACDST), 6)==0) {

    struct ip& ip = *reinterpret_cast<struct ip*>(&eh+1);

    if (ip.ip_dst.s_addr != IGMPv3_REPORT_GROUP)
      return false;

    if (ip.ip_v!=4)
      return false;

    if (ip.ip_p != IPPROTO_IGMP)
      return false;

    unsigned iphlen = ip.ip_hl<<2;

    // test IP checksum for corruption
    if (Service::in_cksum(&ip,iphlen))
      return true;

    IGMPv3Report& ig = *reinterpret_cast<IGMPv3Report*>(reinterpret_cast<char*>(&ip)+iphlen);

    if (!ig.isV3MembershipReport())
      //    return true;
      return false; // allow the network stack to handle our mcast registrations

    unsigned n = ig.numberOfGroupRecords;
    const IGMPGroupRecord* ir = &ig.groupRecords;
    while(n--) {
      bool isListening=true;
      switch( ir->type ) {
      case IGMPGroupRecord::MODE_IS_INCLUDE:
      case IGMPGroupRecord::CHANGE_TO_INCLUDE_MODE:
	if (ir->numberOfSources==0)
	  isListening=false;
	break;
      default:
	break;
      }

#ifdef DBUG
      printf("IGMPv3Handler: changing port mcast membership  port 0x%x  mcast 0x%x  state %c\n",
	     unsigned(pkt.source_port()), ir->mcastAddress, isListening ? 'J' : 'R');
#endif
      _fm.changePortMulticastGroupMembership(FmPortMask(1<<pkt.source_port()), ir->mcastAddress, isListening);

      ir = ir->next();
    }

    return true;
  }

  return false;
}
