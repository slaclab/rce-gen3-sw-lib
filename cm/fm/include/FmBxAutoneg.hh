#ifndef FmBxAutoneg_hh
#define FmBxAutoneg_hh


#include "phy/PhyAutoNegCallback.hh"

#include "fm/FmPort.hh"

class FmTahoeMgr;

class FmBxAutoneg : public PhyAutoNegCallback {
public:
  FmBxAutoneg(FmTahoeMgr& mgr, FmPort port);
  virtual ~FmBxAutoneg();

  void autonegComplete(PhyAutoNegCallback::Result);

private:
  FmTahoeMgr& _mgr;
  FmPort      _port;
};

#endif
