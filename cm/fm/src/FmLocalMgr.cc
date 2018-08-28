
#include "fm/FmLocalMgr.hh"

#include "fm/SwmRequest.hh"
#include "fm/SwmReply.hh"

#include "fm/FmMReg.hh"
#include "fm/FmTahoe.hh"
#include "fm/FmTahoeMgr.hh"
#include "fm/SwmPacket.hh"
#include "fm/FmMacTable.hh"
#include "fm/FmAutoNegCallback.hh"
#include "fm/FmConfig.hh"
#include "fm/FmInterruptTask.hh"
#include "fm/FmFifoInterruptTask.hh"

#include "debug/Print.hh"

#include "concurrency/OldThread.hh"

#include <stdio.h>

#define DBUG

static FmLocalMgr* _mgr = 0;


template <unsigned device>
int _rtems_attach(struct rtems_bsdnet_ifconfig *ifconfig, int attaching)
{
  return _mgr->lciRtemsAttach(ifconfig,attaching,device);
}

static rtems_attach _attach_fcn[] = { &_rtems_attach<0>,
                                      &_rtems_attach<1> };



FmLocalMgr* FmLocalMgr::instance()
{
  if (!_mgr)
    service::debug::printv("No Fulcrum instance found");

  return _mgr;
}

//
//  Initialization of Fulcrum Management
//
FmLocalMgr::FmLocalMgr(int nDevices, FmConfig* fmCfg) :
  _nDevices(nDevices)
{
  _mgr = this;

  _mgrs = new FmTahoeMgr*[nDevices];

  for(int i=0; i<nDevices; i++) {
    FmTahoe* t = new(i) FmTahoe;
    _mgrs[i] = new FmTahoeMgr( *t, fmCfg[i], _attach_fcn[t->id()] );

  }

  //  Start the task-level threads for handling interrupts

  _interruptTask = new FmInterruptTask(_mgrs,_nDevices);
  new tool::concurrency::OldThread("fmISR", 1, 0x1000, *_interruptTask);

  _fifoInterruptTask = new FmFifoInterruptTask(_mgrs);
  new tool::concurrency::OldThread("fmFifoISR", 1, 0x1000, *_fifoInterruptTask);

  for(unsigned k=0; k<_nDevices; k++)
    FmTahoe::enable_rx_fifo_interrupt(k);

}

FmLocalMgr::~FmLocalMgr()
{
  _mgr = 0;
}

int FmLocalMgr::extract(FmConfig* fmCfg) const
{
  int result=0;
  for(unsigned i=0; i<_nDevices; i++) {
    result |= _mgrs[i]->extract(fmCfg[i]);
  }
  return result;
}

void FmLocalMgr::process(const SwmRequest& req,
                          void*            reply,
                          unsigned&        reply_len)
{
  reply_len = 0;

  switch(req.type()) {
  case SwmRequest::Reset :
    {
      const SwmResetRequest& rreq = (const SwmResetRequest&)req;
      rreq.reg()->resetChip();
      asm volatile("eieio");
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
      const SwmPktWriteRequest& wreq = (const SwmPktWriteRequest&)req;
      const SwmPacket& pkt = wreq.data();
      _mgrs[wreq.reg()->id()]->txPacket(pkt.data(),pkt.len(),pkt.dst());
    }
    break;
  case SwmRequest::MacEntryRead :
    {
      const SwmMacEntryReadRequest& rreq = (const SwmMacEntryReadRequest&)req;
      FmMacTable& table = _mgrs[rreq.reg()->id()]->macTable();
      int index = table.findMatch(rreq.data());
      if (index >= 0)
        service::debug::printv("Reading index %x",index);
      else
        service::debug::printv("Unable to match");
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
      service::debug::printv("Writing to device (%p) %d",wreq.reg(),wreq.reg()->id());
      wreq.data().print();
#endif
      FmMacTable& table = _mgrs[wreq.reg()->id()]->macTable();
      int index = table.findMatch(wreq.data());
      if (index >= 0) {
        service::debug::printv("Updating index %x",index);
        table.updateEntry(index,wreq.data());
      }
      else if ((index = table.addEntry(wreq.data())) >= 0) {
#ifdef DBUG
        service::debug::printv("Added index %x",index);
#endif
      }
      else {
        service::debug::printv("Unable to add entry -");
        wreq.data().print();
      }
    }
    break;
  case SwmRequest::MacTableRead :
    {
      const SwmMacTableReadRequest& rreq = (const SwmMacTableReadRequest&)req;
#ifdef DBUG
      service::debug::printv("Reading MAC table for device %d",rreq.reg()->id());
#endif
      FmMacTable& table = _mgrs[rreq.reg()->id()]->macTable();
      new(reply) SwmMacTableReadReply ( rreq.reg(), SwmMacTable(table) );
      reply_len = sizeof(SwmMacTableReadReply);
    }
    break;
  case SwmRequest::MacTableDirect :
    {
      const SwmMacTableDirectRequest& rreq = (const SwmMacTableDirectRequest&)req;
#ifdef DBUG
      service::debug::printv("Reading MAC table directly for device %d",rreq.reg()->id());
#endif
      FmMacTable& table = _mgrs[rreq.reg()->id()]->macTable();
      new(reply) SwmMacTableDirectReply ( rreq.reg(), table.direct() );
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
      service::debug::printv("Writing FID entry %p",wreq.reg());
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
  default:
    service::debug::printv("Unknown request of type 0x%x",req.type());
    break;
  }
  //  service::debug::printv("reply (%p) len (%p) 0x%x",reply, &reply_len, reply_len);
}

void FmLocalMgr::isr()
{
  for(unsigned i=0; i<_nDevices; i++)
    FmTahoe::disableInterrupts(i);

#ifdef DBUG
  service::debug::printv("FmLocalMgr::isr");
#endif

  // handle them at task level
  _interruptTask->unblock();
}

void FmLocalMgr::fifo_isr()
{
  unsigned enable;
  asm volatile("mfdcr %0,%1" : "=r"(enable) : "i"(TAHOE_FIFO_IS_REG));
  unsigned mask;
  asm volatile("mfdcr %0,%1" : "=r"(mask)   : "i"(TAHOE_FIFO_IM_REG));

#ifdef DBUG
  service::debug::printv("FmLocalMgr::fifo_isr %x %x",enable,mask);
#endif

  mask &= enable;

  if (mask & 0x55555555)
    _fifoInterruptTask->unblock();

  for(unsigned id=0; mask!=0; id++, mask>>=2) {
    if (mask&1) FmTahoe::disable_rx_fifo_interrupt(id);
    if (mask&2) _mgrs[id]->tx_fifo_isr();
  }
}

int FmLocalMgr::lciRtemsAttach(struct rtems_bsdnet_ifconfig* aBsdConfig,
                               int attaching,
                               unsigned device)
{
  if (device >= _nDevices) {
    return 0;
  }

  return _mgrs[device]->rtemsAttach(aBsdConfig,attaching);
}

PhyAutoNegCallback* FmLocalMgr::autonegCallback(unsigned dev,
                                                FmPort   port)
{
  return new FmAutoNegCallback(*_mgrs[dev],port);
}
