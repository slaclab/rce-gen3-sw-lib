
#include "fm/FmFifoInterruptTask.hh"

#include "fm/FmTahoeMgr.hh"
#include "fm/FmTahoe.hh"

#include "debug/Print.hh"

//#define DBUG

FmFifoInterruptTask::FmFifoInterruptTask(FmTahoeMgr**& mgrs) :
  _mgrs(mgrs),
  _sem(tool::concurrency::Semaphore::Red)
{
}

FmFifoInterruptTask::~FmFifoInterruptTask()
{
}


void FmFifoInterruptTask::run()
{
  while(1) {
    _sem.take();
    unsigned enable;
    asm volatile("mfdcr %0,%1" : "=r"(enable) : "i"(TAHOE_FIFO_IS_REG));
    for(unsigned k=0; enable!=0; k++) {
      if (enable&1) {
	_mgrs[k]->rxEvent();
	FmTahoe::enable_rx_fifo_interrupt(k);
      }
      enable>>=2;
    }
  }
}
