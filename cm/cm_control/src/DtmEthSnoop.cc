
#include "control/DtmEthSnoop.hh"

DtmEthSnoop::DtmEthSnoop() :
  _rxsem(tool::concurrency::Semaphore::Red)
{
}

void DtmEthSnoop::run()
{
  while(1) {
    _rxsem.take();
    const unsigned* const beg = rxDmaFIFO + (_mgr.id()<<10);
    const unsigned* const end = (const unsigned* const)_read_rx_fifo_ends[_mgr.id()]();

    //    printf("Driver::receive %p:%p\n", beg, end);

    unsigned len = end - beg;
    if (len != 0) {
      FmLCIPacket pkt(beg, len);

      //      _dumpPacket(beg,len);

      const HandlerPtr* empty = _handlers.empty();
      HandlerPtr* hdlr = _handlers.head();
      while( hdlr != empty &&
	     !hdlr->handler->handle(pkt) ) {
	hdlr = hdlr->flink();
      }
    }

    FmTahoe::enable_rx_fifo_interrupt(_mgr.id());
  }
}

void DtmEthSnoop::unblockRx()
{
}
