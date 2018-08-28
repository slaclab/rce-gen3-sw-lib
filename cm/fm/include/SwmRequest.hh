#ifndef SwmRequest_hh
#define SwmRequest_hh



class SwmRequest {
public:
  enum Type { Reset,
	      RegRead,
	      RegWrite,
	      EplRead,
	      EplWrite,
	      PortMIB,
	      PortMSC,
	      GlobalMIB,
	      PktRead,
	      PktWrite,
	      MacEntryRead,
	      MacEntryWrite,
	      MacTableRead,
	      MacTableDirect,
	      VlanEntryRead,
	      VlanEntryWrite,
	      FidEntryRead,
	      FidEntryWrite,
	      StreamStatus,
	      AllPortMIB,
	      AllPortEPL,
              AllTriggers,
              GlortDestEntryRead,
              GlortRamEntryRead,
	      NTypes
   };
  SwmRequest(Type type) :
    _type(type)
  {}
  ~SwmRequest() {}

  Type type() const { return _type; }

private:
  Type _type;
};


class FmMReg;
class FmEplRegs;
class FmPortMibCounters;
class FmPortMscCounters;
class FmVlanTableEntry;
class FmFidTableEntry;
class FmTahoe;
#include "fm/SwmEplRegs.hh"
#include "fm/SwmPortMscCounters.hh"
#include "fm/SwmPacket.hh"
#include "fm/SwmMacTableEntry.hh"
#include "fm/SwmVlanTableEntry.hh"
#include "fm/SwmFidTableEntry.hh"

template <class TF, SwmRequest::Type v>
class SwmReadRequestT : public SwmRequest {
public:
  SwmReadRequestT(TF* r) : SwmRequest(v), _reg(r) {}

  TF* reg() const { return _reg; }
private:
  TF* _reg;
};

typedef SwmReadRequestT<FmTahoe          , SwmRequest::Reset        > SwmResetRequest;
typedef SwmReadRequestT<FmMReg           , SwmRequest::RegRead  > SwmRegReadRequest;
typedef SwmReadRequestT<FmEplRegs        , SwmRequest::EplRead  > SwmEplReadRequest;
typedef SwmReadRequestT<FmPortMibCounters, SwmRequest::PortMIB  > SwmPortMibRequest;
typedef SwmReadRequestT<FmPortMscCounters, SwmRequest::PortMSC  > SwmPortMscRequest;
typedef SwmReadRequestT<FmTahoe          , SwmRequest::GlobalMIB> SwmGlobalMibRequest;
typedef SwmReadRequestT<FmTahoe          , SwmRequest::PktRead  > SwmPktReadRequest;
typedef SwmReadRequestT<FmTahoe          , SwmRequest::MacTableRead > SwmMacTableReadRequest;
typedef SwmReadRequestT<FmTahoe          , SwmRequest::MacTableDirect > SwmMacTableDirectRequest;
typedef SwmReadRequestT<FmVlanTableEntry , SwmRequest::VlanEntryRead > SwmVlanEntryReadRequest;
typedef SwmReadRequestT<FmFidTableEntry  , SwmRequest::FidEntryRead  > SwmFidEntryReadRequest;
typedef SwmReadRequestT<FmTahoe          , SwmRequest::StreamStatus  > SwmStreamStatusRequest;
typedef SwmReadRequestT<FmTahoe          , SwmRequest::AllPortMIB  > SwmAllPortMibRequest;
typedef SwmReadRequestT<FmTahoe          , SwmRequest::AllPortEPL  > SwmAllPortEplRequest;
typedef SwmReadRequestT<FmTahoe          , SwmRequest::AllTriggers > SwmAllTriggersRequest;

template <class TF, class TS, SwmRequest::Type v>
class SwmWriteRequestT : public SwmRequest {
public:
  SwmWriteRequestT(TF* r,const TS& d) : SwmRequest(v), _reg(r), _data(d) {}

  TF*       reg()  const { return _reg; }
  const TS& data() const { return _data; }
private:
  TF* _reg;
  TS  _data;
};

typedef SwmWriteRequestT<FmMReg   ,unsigned         , SwmRequest::RegWrite     > SwmRegWriteRequest;
typedef SwmWriteRequestT<FmEplRegs,SwmEplRegs       , SwmRequest::EplWrite     > SwmEplWriteRequest;
typedef SwmWriteRequestT<FmTahoe  ,SwmPacket        , SwmRequest::PktWrite     > SwmPktWriteRequest;
typedef SwmWriteRequestT<FmTahoe  ,SwmMacTableEntry , SwmRequest::MacEntryRead > SwmMacEntryReadRequest;
typedef SwmWriteRequestT<FmTahoe  ,SwmMacTableEntry , SwmRequest::MacEntryWrite> SwmMacEntryWriteRequest;
typedef SwmWriteRequestT<FmVlanTableEntry, SwmVlanTableEntry , SwmRequest::VlanEntryWrite> SwmVlanEntryWriteRequest;
typedef SwmWriteRequestT<FmFidTableEntry , SwmFidTableEntry  , SwmRequest::FidEntryWrite> SwmFidEntryWriteRequest;

/**
 **  Bali types
 **/
#include "fmb/FmBali.hh"
#include "fmb/FmEplRegs.hh"
#include "fmb/FmPortMibCounters.hh"
#include "fmb/FmIngressVlanTableEntry.hh"
#include "fmb/FmEgressVlanTableEntry.hh"
#include "fmb/FmIngressFidTableEntry.hh"
#include "fmb/FmEgressFidTableEntry.hh"

typedef SwmReadRequestT<Bali::FmBali     , SwmRequest::Reset    > SwmResetRequestB;
typedef SwmReadRequestT<Bali::FmEplRegs  , SwmRequest::EplRead  > SwmEplReadRequestB;
typedef SwmReadRequestT<Bali::FmPortMibCounters, SwmRequest::PortMIB  > SwmPortMibRequestB;
typedef SwmReadRequestT<Bali::FmBali     , SwmRequest::GlobalMIB> SwmGlobalMibRequestB;
typedef SwmReadRequestT<Bali::FmIngressVlanTableEntry , SwmRequest::VlanEntryRead > SwmIngVlanEntryReadRequestB;
typedef SwmReadRequestT<Bali::FmMacTableEntry, SwmRequest::MacEntryRead> SwmMacEntryReadRequestB;
typedef SwmReadRequestT<Bali::FmGlortDestEntry, SwmRequest::GlortDestEntryRead> SwmGlortDestEntryReadRequestB;
typedef SwmReadRequestT<Bali::FmGlortRamEntry, SwmRequest::GlortRamEntryRead> SwmGlortRamEntryReadRequestB;

/**
 **  Implement later
 **
typedef SwmReadRequestT<Bali::FmBali     , SwmRequest::MacTableRead > SwmMacTableReadRequestB;
typedef SwmReadRequestT<Bali::FmBali     , SwmRequest::MacTableDirect > SwmMacTableDirectRequestB;
typedef SwmReadRequestT<Bali::FmBali     , SwmRequest::PktRead  > SwmPktReadRequestB;
typedef SwmReadRequestT<Bali::FmEgressVlanTableEntry  , SwmRequest::VlanEntryRead > SwmEgrVlanEntryReadRequestB;
typedef SwmReadRequestT<Bali::FmIngressFidTableEntry  , SwmRequest::FidEntryRead  > SwmIngFidEntryReadRequestB;
typedef SwmReadRequestT<Bali::FmEgressFidTableEntry   , SwmRequest::FidEntryRead  > SwmEgrFidEntryReadRequestB;
typedef SwmReadRequestT<Bali::FmBali     , SwmRequest::StreamStatus  > SwmStreamStatusRequestB;
**/
typedef SwmReadRequestT<Bali::FmBali     , SwmRequest::AllPortMIB  > SwmAllPortMibRequestB;
typedef SwmReadRequestT<Bali::FmBali     , SwmRequest::AllPortEPL  > SwmAllPortEplRequestB;
/**
typedef SwmReadRequestT<Bali::FmBali     , SwmRequest::AllTriggers > SwmAllTriggersRequestB;
**/
typedef SwmWriteRequestT<Bali::FmEplRegs,Bali::SwmEplRegs       , SwmRequest::EplWrite     > SwmEplWriteRequestB;
/**
typedef SwmWriteRequestT<Bali::FmBali   ,Bali::SwmMacTableEntry , SwmRequest::MacEntryWrite> SwmMacEntryWriteRequestB;
typedef SwmWriteRequestT<Bali::FmIngressVlanTableEntry, Bali::SwmIngressVlanTableEntry , SwmRequest::VlanEntryWrite> SwmIngVlanEntryWriteRequestB;
typedef SwmWriteRequestT<Bali::FmEgressVlanTableEntry , Bali::SwmEgressVlanTableEntry  , SwmRequest::VlanEntryWrite> SwmEgrVlanEntryWriteRequestB;
typedef SwmWriteRequestT<Bali::FmIngressFidTableEntry , Bali::SwmIngressFidTableEntry  , SwmRequest::FidEntryWrite> SwmIngFidEntryWriteRequestB;
typedef SwmWriteRequestT<Bali::FmEgressFidTableEntry  , Bali::SwmEgressFidTableEntry   , SwmRequest::FidEntryWrite> SwmEgrFidEntryWriteRequestB;
**/


#endif
