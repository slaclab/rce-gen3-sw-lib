
#include "fm/FmMgrHandler.hh"

#include "fm/FmTahoe.hh"

#include "cm_svc/Print.hh"

FmMgrHandler::FmMgrHandler(FmTahoe* d) :
  _dev(d)
{
  volatile unsigned v;
  v = _dev->mgr_im = 0;
  _dev->mgr_im = v;
}

void FmMgrHandler::call(unsigned p)
{
  if ( p & 0x10 ) cm::service::printv("Boot complete\n");
  if ( p & 0x08 ) cm::service::printv("SPI error\n");
  _dev->mgr_im = p;
}
