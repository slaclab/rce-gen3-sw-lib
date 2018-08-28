#ifndef FmFifoInterruptTask_hh
#define FmFifoInterruptTask_hh


#include "concurrency/Procedure.hh"
#include "concurrency/Semaphore.hh"

class FmTahoeMgr;

class FmFifoInterruptTask : public tool::concurrency::Procedure {
public:
  FmFifoInterruptTask(FmTahoeMgr**&);
  ~FmFifoInterruptTask();

  void unblock();

  void run();

private:
  FmTahoeMgr**&                _mgrs;
  tool::concurrency::Semaphore _sem;
};


inline void FmFifoInterruptTask::unblock()
{
  _sem.give();
}

#endif
