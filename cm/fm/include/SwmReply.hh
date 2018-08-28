#ifndef SwmReply_hh
#define SwmReply_hh



class SwmReply {
public:
  enum Type { RegRead,
	      EplRead,
	      PortMIB,
	      PortMSC,
	      GlobalMIB,
	      PktRead,
	      MacEntryRead,
	      MacTableRead,
	      MacTableDirect,
	      VlanEntryRead,
	      FidEntryRead,
	      StreamStatus,
	      AllPortMIB,
	      AllPortEPL,
	      AllTriggers,
              GlortDestEntryRead,
              GlortRamEntryRead,
	      NTypes
  };
  SwmReply() {}
  SwmReply(Type type) :
    _type(type)
  {}

  Type type() const { return _type; }

  void* operator new(unsigned,void* p) { return p; }
  void  operator delete(void*) {}
private:
  Type _type;
};


class FmMReg;
#include "fm/SwmEplRegs.hh"
class FmEplRegs;
#include "fm/SwmPortMibCounters.hh"
class FmPortMibCounters;
#include "fm/SwmPortMscCounters.hh"
class FmPortMscCounters;
#include "fm/SwmGlobalMibCounters.hh"
class FmTahoe;
#include "fm/SwmPacket.hh"
#include "fm/SwmMacTableEntry.hh"
#include "fm/SwmMacTable.hh"
#include "fm/SwmMacTableD.hh"
class FmVlanTableEntry;
#include "fm/SwmVlanTableEntry.hh"
class FmFidTableEntry;
#include "fm/SwmFidTableEntry.hh"
#include "fm/SwmStreamStatus.hh"
#include "fm/SwmAllPortMibCounters.hh"
#include "fm/SwmTrigger.hh"

template <class TF, class TS, SwmReply::Type v>
class SwmReplyT : public SwmReply {
public:
  SwmReplyT(TF* r) : SwmReply(v), _reg(r), _result(*r) {}

  TF*       reg()    const { return _reg; }
  const TS& result() const { return _result; }
private:
  TF* _reg;
  TS  _result;
};

typedef SwmReplyT<FmMReg           ,unsigned            ,SwmReply::RegRead  > SwmRegReadReply;
typedef SwmReplyT<FmEplRegs        ,SwmEplRegs          ,SwmReply::EplRead  > SwmEplReadReply;
typedef SwmReplyT<FmPortMibCounters,SwmPortMibCounters  ,SwmReply::PortMIB  > SwmPortMibReply;
typedef SwmReplyT<FmPortMscCounters,SwmPortMscCounters  ,SwmReply::PortMSC  > SwmPortMscReply;
typedef SwmReplyT<FmTahoe          ,SwmGlobalMibCounters,SwmReply::GlobalMIB> SwmGlobalMibReply;
typedef SwmReplyT<FmTahoe          ,SwmPacket           ,SwmReply::PktRead  > SwmPktReadReply;
typedef SwmReplyT<FmVlanTableEntry ,SwmVlanTableEntry   ,SwmReply::VlanEntryRead > SwmVlanEntryReadReply;
typedef SwmReplyT<FmFidTableEntry  ,SwmFidTableEntry    ,SwmReply::FidEntryRead  > SwmFidEntryReadReply;
typedef SwmReplyT<FmTahoe          ,SwmStreamStatus     ,SwmReply::StreamStatus  > SwmStreamStatusReply;
typedef SwmReplyT<FmTahoe          ,SwmTriggerSet       ,SwmReply::AllTriggers   > SwmAllTriggersReply;

template <class TF, class TS, SwmReply::Type v>
class SwmReplyTV : public SwmReply {
public:
  SwmReplyTV(TF* r, const TS& val) : SwmReply(v), _reg(r), _result(val) {}

  TF*       reg()    const { return _reg; }
  const TS& result() const { return _result; }
private:
  TF* _reg;
  TS  _result;
};

typedef SwmReplyTV<FmTahoe, SwmMacTableEntry, SwmReply::MacEntryRead > SwmMacEntryReadReply;
typedef SwmReplyTV<FmTahoe, SwmEplRegs[24]  , SwmReply::AllPortEPL > SwmAllPortEplReply;

template <class TF, class TS, class TV, SwmReply::Type v>
class SwmReplyTSV : public SwmReply {
public:
  SwmReplyTSV(TF* r, const TV& val) : SwmReply(v), _reg(r), _result(val) {}

  TF*       reg()    const { return _reg; }
  const TS& result() const { return _result; }
private:
  TF* _reg;
  TS  _result;
};

typedef SwmReplyTSV<FmTahoe, SwmMacTable , FmMacTable, SwmReply::MacTableRead > SwmMacTableReadReply;
typedef SwmReplyTSV<FmTahoe, SwmMacTableD, FmMacTable, SwmReply::MacTableDirect > SwmMacTableDirectReply;
typedef SwmReplyTSV<FmTahoe, SwmAllPortMibCounters, FmTahoe, SwmReply::AllPortMIB > SwmAllPortMibReply;

/**
 **  Bali types
 **/
namespace Bali {
  class FmBali;
  class FmEplRegs;
};

#include "fmb/SwmEplRegs.hh"
#include "fmb/SwmAllPortMibCounters.hh"
#include "fmb/SwmMacTableEntry.hh"
#include "fmb/SwmMacTable.hh"
#include "fmb/SwmMacTableD.hh"
#include "fmb/FmMacTableEntry.hh"
#include "fmb/SwmIngressVlanTableEntry.hh"
#include "fmb/SwmEgressVlanTableEntry.hh"
#include "fmb/SwmIngressFidTableEntry.hh"
#include "fmb/SwmEgressFidTableEntry.hh"
#include "fmb/SwmGlortDestEntry.hh"
#include "fmb/SwmGlortRamEntry.hh"

typedef SwmReplyT<Bali::FmEplRegs        ,Bali::SwmEplRegs     ,SwmReply::EplRead  > SwmEplReadReplyB;
typedef SwmReplyT<Bali::FmPortMibCounters,Bali::SwmPortMibCounters,SwmReply::PortMIB  > SwmPortMibReplyB;
typedef SwmReplyT<Bali::FmBali           ,SwmGlobalMibCounters ,SwmReply::GlobalMIB> SwmGlobalMibReplyB;
typedef SwmReplyT<Bali::FmBali           ,SwmPacket           ,SwmReply::PktRead  > SwmPktReadReplyB;
typedef SwmReplyT<Bali::FmIngressVlanTableEntry ,Bali::SwmIngressVlanTableEntry ,SwmReply::VlanEntryRead > SwmIngVlanEntryReadReplyB;
typedef SwmReplyT<Bali::FmMacTableEntry, Bali::SwmMacTableEntry, SwmReply::MacEntryRead > SwmMacEntryReadReplyB;
typedef SwmReplyT<Bali::FmGlortDestEntry, Bali::SwmGlortDestEntry, SwmReply::GlortDestEntryRead > SwmGlortDestEntryReadReplyB;
typedef SwmReplyT<Bali::FmGlortRamEntry, Bali::SwmGlortRamEntry, SwmReply::GlortRamEntryRead > SwmGlortRamEntryReadReplyB;

/**
 **  Implement later
 **
typedef SwmReplyT<Bali::FmEgressVlanTableEntry  ,Bali::SwmEgressVlanTableEntry  ,SwmReply::VlanEntryRead > SwmEgrVlanEntryReadReplyB;
typedef SwmReplyT<Bali::FmIngressFidTableEntry ,Bali::SwmIngressFidTableEntry   ,SwmReply::FidEntryRead > SwmIngFidEntryReadReplyB;
typedef SwmReplyT<Bali::FmEgressFidTableEntry  ,Bali::SwmEgressFidTableEntry    ,SwmReply::FidEntryRead > SwmEgrFidEntryReadReplyB;
typedef SwmReplyT<Bali::FmBali                 ,SwmStreamStatus     ,SwmReply::StreamStatus  > SwmStreamStatusReplyB;
typedef SwmReplyT<Bali::FmBali                 ,SwmTriggerSet       ,SwmReply::AllTriggers   > SwmAllTriggersReplyB;

**/
typedef SwmReplyTV<Bali::FmBali, Bali::SwmEplRegs[24]  , SwmReply::AllPortEPL > SwmAllPortEplReplyB;

typedef SwmReplyTSV<Bali::FmBali, SwmMacTable , Bali::FmMacTable, SwmReply::MacTableRead > SwmMacTableReadReplyB;
typedef SwmReplyTSV<Bali::FmBali, SwmMacTableD, Bali::FmMacTable, SwmReply::MacTableDirect > SwmMacTableDirectReplyB;
typedef SwmReplyTSV<Bali::FmBali, Bali::SwmAllPortMibCounters, Bali::FmBali, SwmReply::AllPortMIB > SwmAllPortMibReplyB;


//const unsigned MaxReplyLen = sizeof(SwmEplReadReply);
const unsigned MaxReplyLen = sizeof(SwmMacTableReadReply);

#endif
