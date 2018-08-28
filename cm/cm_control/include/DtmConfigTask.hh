#ifndef DtmConfigTask_hh
#define DtmConfigTask_hh


#include "concurrency/Procedure.hh"

class FmTahoeMgr;

class DtmConfigTask : public tool::concurrency::Procedure {
public:
  DtmConfigTask(FmTahoeMgr&);
public:
  virtual void run();
private:
  FmTahoeMgr& _fm;
};
#endif
