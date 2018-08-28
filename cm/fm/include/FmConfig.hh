#ifndef FmConfig_hh
#define FmConfig_hh


#include "fm/SwmFidTableEntry.hh"
#include "fm/SwmVlanTableEntry.hh"
#include "fm/SwmMacTable.hh"
#include "fm/SwmPortFwdTable.hh"
#include "fm/SwmPortVlanConfig.hh"
#include "fm/FmPort.hh"
//
//  configuration masks of the 24 physical ports
//

//namespace cm {
//  namespace fm {
    class FmPortConfig {
    public:
      FmPortConfig();
      FmPortConfig(const FmPortConfig&);

      enum PortParameter { X_10Mb, X_100Mb, X_1Gb, X_2_5Gb, X_4Gb, X_10Gb,
  			 X_RxFlip, X_TxFlip, X_Loopback };
      FmPortMask update (PortParameter, FmPortMask);
      void       clear  (FmPortMask);

      void load(const char*&);
      void save(char*&) const;

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
    };

    class FmHostIfConfig {
    public:
      FmHostIfConfig();
      FmHostIfConfig(const FmHostIfConfig&);

      void load(const char*&);
      void save(char*&) const;

      char     if_name   [8];
      unsigned ip_addr;
      unsigned ip_netmask;
      unsigned char mac_addr[6];
      unsigned char rsvd[2];  // padding
    };

    class FmSwitchConfig {
    public:
      FmSwitchConfig();
      FmSwitchConfig(const FmSwitchConfig&);

      void load(const char*&);
      void save(char*&) const;

      enum {Entries=0x1000};
      SwmFidTableEntry  fid_table[Entries];
      SwmVlanTableEntry vid_table[Entries];
      SwmMacTable       mac_table;
      SwmPortFwdTable   portfwd_table;
      SwmPortVlanConfig port_vlan;
      enum {Trunks=12};
      FmPortMask        trunk[Trunks];
    };

    class FmTriggerConfig {
    public:
      FmTriggerConfig();
      FmTriggerConfig(const FmTriggerConfig&);

      void load(const char*&);
      void save(char*&) const;

      unsigned mac_entry_trigger;
      unsigned new_priority;
      FmPort   mirror_port;
      enum Action { ForwardNormally=0,
  		  RedirectToMirror,
  		  Mirror,
  		  Discard,
  		  ForwardAtNewPriority,
  		  RedirectAtNewPriority,
  		  MirrorAtNewPriority,
  		  Reserved };
      Action action;
      enum XCast { Unicast, Multicast, Broadcast, Any };
      XCast      xcast;
      unsigned   vlanMatch;
      enum MACMatch { Hit, Miss, Either, OneHit };
      MACMatch   sourceMacMatch;
      MACMatch   destinationMacMatch;
      unsigned   priorityMask;
      FmPortMask sourcePortMask;
      FmPortMask destinationPortMask;
      unsigned   enable;
    };

    class FmProtocolConfig {
    public:
      FmProtocolConfig();
      FmProtocolConfig(const FmProtocolConfig&);

      void load(const char*&);
      void save(char*&) const;

      enum IGMPVsn { NoIGMP=0, IGMPv1, IGMPv2, IGMPv3 };
      IGMPVsn igmpVsn;
      enum GARPImpl { NoGARP=0, MGRP=1, VLAN=2 };
      GARPImpl garpImpl;
      enum STPImpl { NoSTP=0, STP=1, RSTP=2, MSTP=3 };
      STPImpl stpImpl;

      FmPortMask mcastFlood;
    };

    class FmConfig {
    public:
      FmConfig();
      FmConfig(const FmConfig&);

      void print() const;
      void load(const char*&);
      void save(char*&) const;

      enum { NumberOfTriggers = 16 };
      FmPortConfig     portCfg;
      FmHostIfConfig   hostCfg;
      FmSwitchConfig   swCfg;
      FmTriggerConfig  trigCfg[NumberOfTriggers];
      FmProtocolConfig protocolCfg;
    };
  //};
//};

#endif
