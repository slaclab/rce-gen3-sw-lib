
#include "Manager.hh"

#include "fm/SwmRequest.hh"
#include "fm/SwmReply.hh"

#include "fm/FmMReg.hh"
#include "fm/FmTahoe.hh"
#include "fm/FmTahoeMgr.hh"
#include "fm/SwmPacket.hh"
#include "fm/FmMacTable.hh"
#include "fm/FmConfig.hh"
#include "cm_svc/ChannelMap.hh"
#include "cm_svc/Print.hh"
#include "cm_net/Driver.hh"

#include <stdio.h>
#include <string.h>

using namespace cm::net;

class SwmEplRegArray {
public:
  SwmEplRegs reg[24];
};

static Manager* _mgr;

Manager::Manager(const FmConfig* cfg) :
  _nDevices(1)
{
  _mgr = this;

  _mgrs   = new FmTahoeMgr*[_nDevices];

  for(unsigned i=0; i<_nDevices; i++) {
    _mgrs  [i] = new FmTahoeMgr( *new(i) FmTahoe, cfg[i] );
  }
}

Manager::Manager(int nDevices, const FmConfig* cfg) :
  _nDevices(1)
{
  _mgr = this;

  _mgrs   = new FmTahoeMgr*[_nDevices];

  for(unsigned i=0; i<_nDevices; i++) {
    FmTahoeMgr& m = *new FmTahoeMgr( *new(i) FmTahoe, cfg[i] );
    //    Driver&     d = *new Driver(m);
    _mgrs  [i] = &m;
  }
}

Manager::~Manager()
{
  for(unsigned k=0; k<_nDevices; k++) {
    delete _mgrs[k];
  }
  delete[] _mgrs;
}

int Manager::read     (FmConfig* fmCfg)
{
  int result=0;
  for(unsigned i=0; i<_nDevices; i++) {
    result |= _mgrs[i]->read(fmCfg[i]);
  }
  return result;
}

int Manager::configure(FmConfig* fmCfg)
{
  int result=0;
  for(unsigned i=0; i<_nDevices; i++) {
    result |= _mgrs[i]->configure(fmCfg[i]);
  }
  return result;
}

int Manager::configure(unsigned i,const FmConfig& fmCfg)
{
  return _mgrs[i]->configure(fmCfg);
}


void Manager::process(const SwmRequest& req,
		      void*             reply,
		      unsigned&         reply_len)
{
  reply_len = 0;

  switch(req.type()) {
  case SwmRequest::Reset :
    {
      const SwmResetRequest& rreq = (const SwmResetRequest&)req;
      rreq.reg()->resetChip();
      //      asm volatile("eieio");
      rreq.reg()->releaseChip();
    }
    break;
  case SwmRequest::RegRead :
    {
      const SwmRegReadRequest& rreq = (const SwmRegReadRequest&)req;
      new(reply) SwmRegReadReply ( rreq.reg() );
      reply_len = sizeof(SwmRegReadReply);
    }
    break;
  case SwmRequest::RegWrite :
    {
      const SwmRegWriteRequest& wreq = (const SwmRegWriteRequest&)req;
      *wreq.reg() = wreq.data();
    }
    break;
  case SwmRequest::EplRead :
    {
      const SwmEplReadRequest& rreq = (const SwmEplReadRequest&)req;
      new(reply) SwmEplReadReply ( rreq.reg() );
      reply_len = sizeof(SwmEplReadReply);
    }
    break;
  case SwmRequest::EplWrite :
    {
      const SwmEplWriteRequest& wreq = (const SwmEplWriteRequest&)req;
      *wreq.reg() = wreq.data();
    }
    break;
  case SwmRequest::PortMIB :
    {
      const SwmPortMibRequest& rreq = (const SwmPortMibRequest&)req;
      new(reply) SwmPortMibReply ( rreq.reg() );
      reply_len = sizeof(SwmPortMibReply);
    }
    break;
  case SwmRequest::PortMSC :
    {
      const SwmPortMscRequest& rreq = (const SwmPortMscRequest&)req;
      new(reply) SwmPortMscReply ( rreq.reg() );
      reply_len = sizeof(SwmPortMscReply);
    }
    break;
  case SwmRequest::GlobalMIB :
    {
      const SwmGlobalMibRequest& rreq = (const SwmGlobalMibRequest&)req;
      new(reply) SwmGlobalMibReply ( rreq.reg() );
      reply_len = sizeof(SwmGlobalMibReply);
    }
    break;
  case SwmRequest::PktRead :
    {
      const SwmPktReadRequest& rreq = (const SwmPktReadRequest&)req;
      new(reply) SwmPktReadReply ( rreq.reg() );
      reply_len = sizeof(SwmPktReadReply) +
	((SwmPktReadReply*)reply)->result().len()*sizeof(unsigned);
    }
    break;
  case SwmRequest::PktWrite :
    {
      //      const SwmPktWriteRequest& wreq = (const SwmPktWriteRequest&)req;
      //      const SwmPacket& pkt = wreq.data();
      //      _mgrs[wreq.reg()->id()]->txPacket(pkt.data(),pkt.len(),pkt.dst());
    }
    break;
  case SwmRequest::MacEntryRead :
    {
      const SwmMacEntryReadRequest& rreq = (const SwmMacEntryReadRequest&)req;
      FmMacTable& table = _mgrs[rreq.reg()->id()]->macTable();
      int index = table.findMatch(rreq.data());
      if (index >= 0)
	cm::service::printv("Reading index %x\n",index);
      else
	cm::service::printv("Unable to match\n");
      new(reply) SwmMacEntryReadReply ( rreq.reg(),
					index < 0 ?
					rreq.data() : table.entry(index) );
      reply_len = sizeof(SwmMacEntryReadReply);
    }
    break;
  case SwmRequest::MacEntryWrite :
    {
      const SwmMacEntryWriteRequest& wreq = (const SwmMacEntryWriteRequest&)req;
#ifdef DBUG
      cm::service::printv("Writing to device (%p) %d\n",wreq.reg(),wreq.reg()->id());
      wreq.data().print();
#endif
      FmMacTable& table = _mgrs[wreq.reg()->id()]->macTable();
      int index = table.findMatch(wreq.data());
      if (index >= 0) {
	cm::service::printv("Updating index %x\n",index);
	table.updateEntry(index,wreq.data());
      }
      else if ((index = table.addEntry(wreq.data())) >= 0) {
#ifdef DBUG
	cm::service::printv("Added index %x\n",index);
#endif
      }
      else {
	cm::service::printv("Unable to add entry -");
	wreq.data().print();
      }
    }
    break;
  case SwmRequest::MacTableRead :
    {
      const SwmMacTableReadRequest& rreq = (const SwmMacTableReadRequest&)req;
#ifdef DBUG
      cm::service::printv("Reading MAC table for device %d\n",rreq.reg()->id());
#endif
      const FmMacTable& table = _mgrs[rreq.reg()->id()]->macTable();
      new(reply) SwmMacTableReadReply ( rreq.reg(), table );
      reply_len = sizeof(SwmMacTableReadReply);
    }
    break;
  case SwmRequest::MacTableDirect :
    {
      const SwmMacTableDirectRequest& rreq = (const SwmMacTableDirectRequest&)req;
#ifdef DBUG
      cm::service::printv("Reading MAC table directly for device %d\n",rreq.reg()->id());
#endif
      const FmMacTable& table = _mgrs[rreq.reg()->id()]->macTable();
      new(reply) SwmMacTableDirectReply ( rreq.reg(), table );
      reply_len = sizeof(SwmMacTableDirectReply);
    }
    break;
  case SwmRequest::VlanEntryRead :
    {
      const SwmVlanEntryReadRequest& rreq = (const SwmVlanEntryReadRequest&)req;
      new(reply) SwmVlanEntryReadReply ( rreq.reg() );
      reply_len = sizeof(SwmVlanEntryReadReply);
    }
    break;
  case SwmRequest::VlanEntryWrite :
    {
      const SwmVlanEntryWriteRequest& wreq = (const SwmVlanEntryWriteRequest&)req;
      *wreq.reg() = wreq.data();
    }
    break;
  case SwmRequest::FidEntryRead :
    {
      const SwmFidEntryReadRequest& rreq = (const SwmFidEntryReadRequest&)req;
      new(reply) SwmFidEntryReadReply ( rreq.reg() );
      reply_len = sizeof(SwmFidEntryReadReply);
    }
    break;
  case SwmRequest::FidEntryWrite :
    {
      const SwmFidEntryWriteRequest& wreq = (const SwmFidEntryWriteRequest&)req;
      cm::service::printv("Writing FID entry %p\n",wreq.reg());
      *wreq.reg() = wreq.data();
    }
    break;
  case SwmRequest::StreamStatus:
    {
      const SwmStreamStatusRequest& rreq = (const SwmStreamStatusRequest&)req;
      new(reply) SwmStreamStatusReply ( rreq.reg() );
      reply_len = sizeof(SwmStreamStatusReply);
    }
    break;
  case SwmRequest::AllPortMIB :
    {
      const SwmAllPortMibRequest& rreq = (const SwmAllPortMibRequest&)req;
      new(reply) SwmAllPortMibReply ( rreq.reg(), *rreq.reg() );
      reply_len = sizeof(SwmAllPortMibReply);
    }
    break;
  case SwmRequest::AllPortEPL :
    {
      const SwmAllPortEplRequest& rreq = (const SwmAllPortEplRequest&)req;

      SwmEplRegArray* regs = new SwmEplRegArray;
      for(unsigned j=0; j<24; j++)
	regs->reg[j] = SwmEplRegs( rreq.reg()->epl_phy[j] );

      new(reply) SwmAllPortEplReply ( rreq.reg(), regs->reg );
      delete regs;
      reply_len = sizeof(SwmAllPortEplReply);
    }
    break;
  case SwmRequest::AllTriggers:
    {
      const SwmAllTriggersRequest& rreq = (const SwmAllTriggersRequest&)req;
      new(reply) SwmAllTriggersReply( rreq.reg() );
      reply_len = sizeof(SwmAllTriggersReply);
    }
    break;
  default:
    cm::service::printv("Unknown request of type 0x%x\n",req.type());
    break;
  }
  //  cm::service::printv("reply (%p) len (%p) 0x%x",reply, &reply_len, reply_len);
}

const FmConfig& Manager::config(unsigned i) const {
  return _mgrs[i]->_config;
}

static cm::svc::ChannelMap _map(32,25);

const cm::svc::ChannelMap& Manager::map(unsigned) const 
{
  _map.add( 0 , 13 );  // Zone 2  Slot 2
  _map.add( 1 , 14 );  // Zone 2  Slot 3
  _map.add( 2 , 15 );  // Zone 2  Slot 4
  _map.add( 3 , 16 );
  _map.add( 4 , 17 );
  _map.add( 5 , 18 );
  _map.add( 6 , 19 );
  _map.add( 7 , 20 );
  _map.add( 8 , 21 );
  _map.add( 9 , 22 );
  _map.add( 10, 10 );
  _map.add( 11, 11 );
  _map.add( 12, 12 );
  _map.add( 16,  2 );  // DPM 0-0
  _map.add( 17,  3 );
  _map.add( 18,  4 );
  _map.add( 19,  5 );
  _map.add( 20,  8 );
  _map.add( 21,  9 );
  _map.add( 22,  6 );
  _map.add( 23,  7 );
  _map.add( 28, 23 );  // SFP
  _map.add( 29, 24 );  // SFP+
  _map.add( 31,  1 );  // DTM
  return _map;
}
