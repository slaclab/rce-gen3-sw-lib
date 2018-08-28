#ifndef cm_svc_ClusterIpInfo_hh
#define cm_svc_ClusterIpInfo_hh

#include "bsi/Bsi.h"

enum {
  VlanID_mask = 0x00000fff,
  Vlan_use    = 0x80000000
};

enum {
  RejectUntagged=1,
  RejectTagged  =2,
  RejectBV      =4
};

typedef struct {
  unsigned _vlan;
  unsigned _ingress_rules;
  unsigned _ip;
  unsigned _netmask;
  unsigned _netmask_len;
  unsigned _ip_begin;
  unsigned _ip_end;
  unsigned _gateway;
} ClusterIpInfo;

ClusterIpInfo* ClusterIpInfo_init(Bsi);

#endif
