
#include "fm/FmAutoNegCallback.hh"

#include "fm/FmTahoeMgr.hh"

FmAutoNegCallback::FmAutoNegCallback(FmTahoeMgr& mgr,
				     FmPort port) :
  _mgr(mgr),
  _port(port)
{
}

FmAutoNegCallback::~FmAutoNegCallback()
{
}

void FmAutoNegCallback::autonegComplete(PhyAutoNegCallback::Result r)
{
  cm::fm::PortSpeed s;
  switch(r) {
  case PhyAutoNegCallback::an10Mb  : s = cm::fm::X10Mb ; break;
  case PhyAutoNegCallback::an100Mb : s = cm::fm::X100Mb; break;
  case PhyAutoNegCallback::an1000Mb: s = cm::fm::X1Gb  ; break;
  default                          : s = cm::fm::None; break;
  }
  _mgr.configurePort(_port,s);
}

