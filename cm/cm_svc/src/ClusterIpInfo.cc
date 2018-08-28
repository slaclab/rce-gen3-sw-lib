#include "cm_svc/ClusterIpInfo.hh"
#include "bsi/Bsi_Cfg.h"
#include "bsi/Bsi.h"
#include "map/Lookup.h"

#include "arpa/inet.h"

using namespace cm::svc;

ClusterIpInfo::ClusterIpInfo() 
{
  Bsi bsi(LookupBsi());
  //
  //  Wait for all information to be valid
  //

  unsigned slot;

  uint32_t v;
  v          = BsiRead32(bsi,BSI_CLUSTER_ADDR_OFFSET);
  slot       = BSI_CLUSTER_FROM_CLUSTER_ADDR(v);

  v          = BsiRead32(bsi, BSI_CLUSTER_IP_INFO_OFFSET+0);
  if (!v)  v = 1;  // default VLAN=1 (don't apply)
  _vlan      = v;

  v          = BsiRead32(bsi, BSI_CLUSTER_IP_INFO_OFFSET+1);
  _ingress_rules = v;

  v          = ntohl(BsiRead32(bsi, BSI_CLUSTER_IP_INFO_OFFSET+2));
  if (!v)  v = 0xc0a80001; // 192.168.0.1
  if (!(v&0xff)) 
           v += 1; // don't include local address of 0
  _ip_begin = v;
  

  v          = ntohl(BsiRead32(bsi, BSI_CLUSTER_IP_INFO_OFFSET+3));
  if (!v)  v = 0xc0a800fe; // 192.168.0.254
  if ((v&0xff)==0xff)
           v -= 1; // don't include broacast address of 255
  _ip        = v-(slot-1);
  _ip_end    = v-16;

  v          = ntohl(BsiRead32(bsi, BSI_CLUSTER_IP_INFO_OFFSET+4));
  if (!v)  v = 0xffff0000; // 255.255.0.0
  _netmask   = v;
  { 
    unsigned i=32;
    while((v&1)==0) { v>>=1; i--; }
    _netmask_len = i;
  }

  v          = ntohl(BsiRead32(bsi, BSI_CLUSTER_IP_INFO_OFFSET+5));
  if (!v)  v = 0xc0a80001; // 192.168.0.1
  _gateway = v;
}

bool     ClusterIpInfo::use_vlan       () const
{ return _vlan&0x80000000; }

unsigned ClusterIpInfo::vlan_id        () const
{ return _vlan&0xfff; }

bool     ClusterIpInfo::reject_untagged  () const
{ return _ingress_rules&1; }

bool     ClusterIpInfo::reject_tagged() const
{ return _ingress_rules&2; }

bool     ClusterIpInfo::reject_boundary_violation() const
{ return _ingress_rules&4; }
