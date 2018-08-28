#ifndef Bali_SwmPortMibCounters_hh
#define Bali_SwmPortMibCounters_hh

#include "fmb/FmDefs.hh"
#include "fm/SwmMibCounter.hh"

namespace Bali {
class FmPortMibCounters;

class SwmPortMibCounters {
public:
  SwmPortMibCounters();
  SwmPortMibCounters( const FmPortMibCounters& );
  ~SwmPortMibCounters();

  SwmPortMibCounters& operator=(const SwmPortMibCounters& c);

#define ADD_RSRV(group,len)
#define ADD_STAT(group,member0,member1)         \
  SwmMibCounter member0;                        \
  SwmMibCounter member1

  ADD_STAT_PORTCOUNTERS

#undef ADD_STAT
#undef ADD_RSRV
};
};

Bali::SwmPortMibCounters operator-(const Bali::SwmPortMibCounters&,
                                   const Bali::SwmPortMibCounters&);

#endif
