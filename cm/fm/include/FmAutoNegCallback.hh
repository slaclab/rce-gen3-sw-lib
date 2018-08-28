#ifndef FmAutoNegCallback_hh
#define FmAutoNegCallback_hh


#include "phy/PhyAutoNegCallback.hh"

#include "fm/FmPort.hh"

class FmTahoeMgr;

class FmAutoNegCallback : public PhyAutoNegCallback {
public:
  FmAutoNegCallback(FmTahoeMgr& mgr, FmPort port);
  virtual ~FmAutoNegCallback();

  void autonegComplete(PhyAutoNegCallback::Result);

private:
  FmTahoeMgr& _mgr;
  FmPort      _port;
};

#endif
