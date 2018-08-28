#ifndef PortMonitor_hh
#define PortMonitor_hh


#include "concurrency/Procedure.hh"

#include "fm/SwmAllPortMibCounters.hh"

class FmTahoeMgr;
class FmPortMask;
class PortCounters;
class GlobalCounters;
class SwmGlobalMibCounters;

class PortMonitor : public tool::concurrency::Procedure {
public:
  PortMonitor(FmTahoeMgr&, FmTahoeMgr&);
  ~PortMonitor();

  virtual void run();
private:
  void update_counters(const char*, FmTahoe&, FmPortMask,
		       SwmAllPortMibCounters&, SwmAllPortMibCounters&,
		       SwmGlobalMibCounters& , SwmGlobalMibCounters&);
private:
  FmTahoeMgr& _mgr0;
  FmTahoeMgr& _mgr1;
  FmTahoe& _fm0;
  FmTahoe& _fm1;
  SwmAllPortMibCounters* _fm0_counters;
  SwmAllPortMibCounters* _fm1_counters;
  SwmAllPortMibCounters* _fm0_counters_d;
  SwmAllPortMibCounters* _fm1_counters_d;
  PortCounters*          _monitor;
  SwmGlobalMibCounters* _fm0_globals;
  SwmGlobalMibCounters* _fm1_globals;
  SwmGlobalMibCounters* _fm0_globals_d;
  SwmGlobalMibCounters* _fm1_globals_d;
  GlobalCounters*       _gmonitor;
};

#endif

