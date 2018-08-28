#ifndef FmInterruptTask_hh
#define FmInterruptTask_hh


#include "concurrency/Procedure.hh"
#include "concurrency/Semaphore.hh"

class FmTahoeMgr;

class FmInterruptTask : public tool::concurrency::Procedure {
public:
  FmInterruptTask(FmTahoeMgr**&,unsigned);
  ~FmInterruptTask();

  void unblock();

  void run();

private:
  FmTahoeMgr**&                _mgrs;
  unsigned                     _n;
  tool::concurrency::Semaphore _sem;
};


inline void FmInterruptTask::unblock()
{
  _sem.give();
}

#endif
