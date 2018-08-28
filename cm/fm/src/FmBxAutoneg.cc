
#include "fm/FmBxAutoneg.hh"

#include "fm/FmTahoeMgr.hh"

FmBxAutoneg::FmBxAutoneg(FmTahoeMgr& mgr,
			 FmPort      port) :
  _mgr(mgr),
  _port(port)
{
}

FmBxAutoneg::~FmBxAutoneg()
{
}

void FmBxAutoneg::autonegComplete(PhyAutoNegCallback::Result r)
{
  cm::fm::PortSpeed s;
  switch(r) {
  case PhyAutoNegCallback::an10Mb  : s = cm::fm::X10Mb ; break;
  case PhyAutoNegCallback::an100Mb : s = cm::fm::X100Mb; break;
  case PhyAutoNegCallback::an1000Mb: s = cm::fm::X1Gb  ; break;
  default                          : s = cm::fm::None; return;
  }
  _mgr.configurePort(_port,s,1,1);
}

