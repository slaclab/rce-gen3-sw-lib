#ifndef CimBPDUFrame_hh
#define CimBPDUFrame_hh


#include "bpdu/BridgeId.hh"

#include <stdint.h>
#include <sys/types.h>
#include <net/ethernet.h> // for ethernet type defines

namespace cm {
  namespace bpdu {

    static const struct ether_addr bpdu_mac_address = { {'\1','\x80','\xc2','\0','\0','\0'} };

    enum BpduType { Configuration = 0, Change = 0x80, RSTP = 2 };

    class LLCHeader {
      enum { BSTP = 0x42 };  // Bridge Spanning Tree Protocol
    public:
      LLCHeader() : dst(BSTP), src(BSTP), len(sizeof(*this)) {}
    public:
      uint8_t     dst;
      uint8_t     src;
      uint8_t     len;
    };

#define SWAP16(w) (((w&0xff00)>>8) | ((w&0xff)<<8))

#pragma pack(push,1)
    class Frame {
    public:
      Frame();
    public:
      enum Role { Unknown=0, AlternateBackup=1, Root=2, Designated=3 };
      Role portRole() const;
    public:
      void dump() const;
    public:
      LLCHeader    llc;
      uint16_t     protocolId;
      uint8_t      protocolVsn;
      enum { Configuration = 0, RSTP = 2, TCN = 0x80 };
      uint8_t      type;      // { RSTP = 2 }
      enum { Topology_Change = 0x1, Proposal = 0x2, Port_Role = 0xc, Learning = 0x10, Forwarding = 0x20, Agreement = 0x40 };
      uint8_t      flags;
      BridgeId     rootId;
      uint32_t     rootPathCost;
      BridgeId     bridgeId;
      uint16_t     portId;
      uint16_t     msg_age;
      uint16_t     max_age;
      uint16_t     hello;
      uint16_t     forward_delay;
      uint8_t      version1_len;
    };
#pragma pack(pop)
  };
};

inline cm::bpdu::Frame::Role cm::bpdu::Frame::portRole() const
{ return cm::bpdu::Frame::Role((flags>>2)&3); }

#endif
