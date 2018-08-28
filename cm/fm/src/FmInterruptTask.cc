
#include "fm/FmInterruptTask.hh"

#include "fm/FmTahoeMgr.hh"
#include "fm/FmTahoe.hh"

#include "debug/Print.hh"

//#define DBUG

FmInterruptTask::FmInterruptTask(FmTahoeMgr**& mgrs,
				 unsigned      n) :
  _mgrs(mgrs),
  _n   (n),
  _sem (tool::concurrency::Semaphore::Red)
{
}

FmInterruptTask::~FmInterruptTask()
{
}


void FmInterruptTask::run()
{
  using namespace service::debug;

  while(1) {
    unsigned v;
    asm volatile("mfdcr %0,%1" : "=r"(v) : "i"(TAHOE_STAT_REG));
#ifdef DBUG
    printv("FmLocalMgr found interrupt status %x",v);
    for(unsigned i=0; i<_n; i++) {
      if ( (v&(0x1<<(4*i))) == 0 )
	printv("Device %d interrupt",i);
      if ( (v&(0x2<<(4*i))) != 0 )
	printv("Device %d RxEOT",i);
      if ( (v&(0x4<<(4*i))) != 0 )
	printv("Device %d RxRdy (FIFO not empty)",i);
      if ( (v&(0x8<<(4*i))) != 0 )
	printv("Device %d TxRdy (FIFO not full)",i);
    }
#endif
    v = ~v & 0x11111111;
    for(unsigned i=0; v!=0; i++) {
      if ( v&1 )
	_mgrs[i]->handle_interrupt();
      v >>= 4;
    }

    for(unsigned i=0; i<_n; i++)
      FmTahoe::enableInterrupts(i);

    _sem.take();
  }
}
