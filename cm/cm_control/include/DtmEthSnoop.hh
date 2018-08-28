#ifndef DtmEthSnoop_hh
#define DtmEthSnoop_hh


#include "concurrency/Procedure.hh"
#include "concurrency/Semaphore.hh"

class DtmEthSnoop : public RceSvc::Procedure {
public:
  DtmEthSnoop();
public:
  virtual void run();

  void unblockRx();
private:
  tool::concurrency::Semaphore        _rxsem;
};
#endif
