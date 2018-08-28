#include "cm_svc/ClusterIpInfo.hh"
#include "bsi/Bsi_Cfg.h"
#include "bsi/Bsi.h"
#include "map/Lookup.h"

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

  v          = ntohl(BsiRead32(bsi, BSI_CLUSTER_IP_INFO_OFFSET+2));
  if (!v)  v = 0xc0a80000; // 192.168.0.0
  _ip_base = v;

  v          = ntohl(BsiRead32(bsi, BSI_CLUSTER_IP_INFO_OFFSET+3));
  if (!v)  v = 0xc0a8ffff; // 192.168.255.255
  _ip        = v-slot;
  _ip_end    = v-16;

  v          = ntohl(BsiRead32(bsi, BSI_CLUSTER_IP_INFO_OFFSET+4));
  if (!v)  v = 0xffff0000;
  _netmask   = v;
  { 
    unsigned i=32;
    while((v&1)==0) { v>>=1; i++; }
    _netmask_len = i;
  }

  v          = ntohl(BsiRead32(bsi, BSI_CLUSTER_IP_INFO_OFFSET+5));
  if (!v)  v = 0xc0a8ffff; // 192.168.255.255
  _gateway = v;
}
