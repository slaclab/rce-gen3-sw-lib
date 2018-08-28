#ifndef FmConfig_h
#define FmConfig_h

#include <inttypes.h>

#include "FmPort.h"

//
//  configuration masks of the 24 physical ports
//

typedef struct {
  // port speed
  FmPortMask ports10Mb;
  FmPortMask ports100Mb;
  FmPortMask ports1Gb;
  FmPortMask ports2_5Gb;
  FmPortMask ports4Gb;
  FmPortMask ports10Gb;
  // lane reversal
  FmPortMask portsRxFlip;
  FmPortMask portsTxFlip;
  // features
  FmPortMask portsLoopback;
}  FmPortConfig;

typedef struct {
  FmPortMask forward[MAX_PORT];
} FmPortFwdConfig;

typedef struct {
  FmPortMask listening;
  FmPortMask learning;
  FmPortMask forwarding;
} SwmFidTableEntry;

typedef struct {
  uint32_t _data[2];
} SwmVlanTableEntry;

typedef struct {
  char     if_name   [8];
  uint32_t ip_addr;
  uint32_t ip_netmask;
  unsigned char mac_addr[6];
  unsigned char rsvd[2];  // padding
} FmHostIfConfig;

typedef struct {
  uint32_t _data[4];
} SwmMacTableEntry;

typedef struct {
  SwmMacTableEntry entry[0x4000];
} SwmMacTable;

typedef struct {
  FmPortMask forward[MAX_PORT];
} SwmPortFwdTable;

typedef struct {
  FmPortMask strict;        // require port is member (on ingress)
  FmPortMask tagged;        // require tagged
  FmPortMask untagged;      // require untagged
  unsigned    defaultVlan[MAX_PORT];  // apply to untagged
} SwmPortVlanConfig;

enum {Entries=0x1000};
enum {Trunks=12};

typedef struct {
  SwmFidTableEntry  fid_table[Entries];
  SwmVlanTableEntry vid_table[Entries];
  SwmMacTable       mac_table;
  SwmPortFwdTable   portfwd_table;
  SwmPortVlanConfig port_vlan;
  FmPortMask        trunk[Trunks];
} FmSwitchConfig;

typedef enum { ForwardNormally=0,
               RedirectToMirror,
               Mirror,
               Discard,
               ForwardAtNewPriority,
               RedirectAtNewPriority,
               MirrorAtNewPriority,
               Reserved } Action;
typedef enum { Unicast, Multicast, Broadcast, Any } XCast;
typedef enum { Hit, Miss, Either, OneHit } MACMatch;

typedef struct {
  unsigned mac_entry_trigger;
  unsigned new_priority;
  FmPort   mirror_port;
  Action action;
  XCast      xcast;
  unsigned   vlanMatch;
  MACMatch   sourceMacMatch;
  MACMatch   destinationMacMatch;
  unsigned   priorityMask;
  FmPortMask sourcePortMask;
  FmPortMask destinationPortMask;
  unsigned   enable;
} FmTriggerConfig;

typedef enum { NoIGMP=0, IGMPv1, IGMPv2, IGMPv3 } IGMPVsn;
typedef enum { NoGARP=0, MGRP=1, VLAN=2 } GARPImpl;
typedef enum { NoSTP=0, STP=1, RSTP=2, MSTP=3 } STPImpl;

typedef struct {
  IGMPVsn igmpVsn;
  GARPImpl garpImpl;
  STPImpl stpImpl;
  FmPortMask mcastFlood;
} FmProtocolConfig;

enum { NumberOfTriggers = 16 };

typedef struct {
  FmPortConfig      portCfg;
  FmHostIfConfig   hostCfg;
  FmSwitchConfig   swCfg;
  FmTriggerConfig  trigCfg[NumberOfTriggers];
  FmProtocolConfig protocolCfg;
} FmConfig;

enum { TxRdy=1<<0,
       RxRdy=1<<1,
       RxEOT=1<<2,
       IS   =1<<3 };

FmConfig* FmConfig_init(void);

void SwmVlanTableEntry_init(SwmVlanTableEntry* this,
                            unsigned port_membership_mask, unsigned port_tag_mask,
                            unsigned trig_id, unsigned counter, unsigned reflect);


unsigned SwmVlanTableEntry_members(SwmVlanTableEntry* this);

#endif
