#include "../ClusterIpInfo.h"
#include "bsi/Bsi.h"
#include <malloc.h>

static inline unsigned _ntohl_(unsigned v)
{
  return 
    (((v>>24)&0xff)<< 0) |
    (((v>>16)&0xff)<< 8) |
    (((v>> 8)&0xff)<<16) |
    (((v>> 0)&0xff)<<24);
}

ClusterIpInfo* ClusterIpInfo_init(Bsi bsi)
{
  ClusterIpInfo* info = malloc(sizeof(ClusterIpInfo));

  //
  //  Wait for all information to be valid
  //

  unsigned slot;

  uint32_t v;
  v          = BsiRead32(bsi,BSI_CLUSTER_ADDR_OFFSET);
  slot       = BSI_CLUSTER_FROM_CLUSTER_ADDR(v);

  v          = BsiRead32(bsi, BSI_CLUSTER_IP_INFO_OFFSET+0);
  if (!v)  v = 1;  // default VLAN=1 (don't apply)
  info->_vlan      = v;

  v          = BsiRead32(bsi, BSI_CLUSTER_IP_INFO_OFFSET+1);
  info->_ingress_rules = v;

  v          = _ntohl_(BsiRead32(bsi, BSI_CLUSTER_IP_INFO_OFFSET+2));
  if (!v)  v = 0xc0a80001; // 192.168.0.1
  if (!(v&0xff)) 
           v += 1; // don't include local address of 0
  info->_ip_begin = v;
  

  v          = _ntohl_(BsiRead32(bsi, BSI_CLUSTER_IP_INFO_OFFSET+3));
  if (!v)  v = 0xc0a800fe; // 192.168.0.254
  if ((v&0xff)==0xff)
           v -= 1; // don't include broadcast address of 255

  if (slot&0x40000000) {
    info->_ip        = v;
    v               -= 1;
  }
  else
    info->_ip        = 0;

  info->_ip_end    = v;

  v          = _ntohl_(BsiRead32(bsi, BSI_CLUSTER_IP_INFO_OFFSET+4));
  if (!v)  v = 0xffff0000; // 255.255.0.0
  info->_netmask   = v;
  { 
    unsigned i=32;
    while((v&1)==0) { v>>=1; i--; }
    info->_netmask_len = i;
  }

  v          = _ntohl_(BsiRead32(bsi, BSI_CLUSTER_IP_INFO_OFFSET+5));
  if (!v)  v = 0xc0a80001; // 192.168.0.1
  info->_gateway = v;

  return info;
}
