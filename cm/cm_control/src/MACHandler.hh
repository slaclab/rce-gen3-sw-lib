#ifndef cm_control_MACHandler_hh
#define cm_control_MACHandler_hh

#include "fm/FmHandler.hh"

#include "fm/FmPort.hh"

#include <set>

class FmTahoeMgr;

class MACHandler : public FmHandler {
public:
  MACHandler(FmTahoeMgr&,
             unsigned allow,
             unsigned deny);
  ~MACHandler();
public:
  void call      (unsigned);
  void clear_port(FmPort);
private:
  void _setup_entry(unsigned   ientry);
private:
  FmTahoeMgr& _fm;
  unsigned    _allow;
  unsigned    _deny;
  unsigned    _trig;
  std::set<unsigned short> _entries;
};

#endif
