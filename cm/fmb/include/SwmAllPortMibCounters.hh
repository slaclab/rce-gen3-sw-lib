#ifndef Bali_SwmAllPortMibCounters_hh
#define Bali_SwmAllPortMibCounters_hh


#include "fmb/SwmPortMibCounters.hh"

namespace Bali {
class FmBali;

class SwmAllPortMibCounters {
public:
  enum { NPORTS=25 };
  SwmAllPortMibCounters();
  SwmAllPortMibCounters( const FmBali& );
  ~SwmAllPortMibCounters();

  SwmAllPortMibCounters& operator=(const SwmAllPortMibCounters& c);
  SwmAllPortMibCounters& operator-=(const SwmAllPortMibCounters& c);

  const SwmPortMibCounters& port(unsigned index) const { return _port[index]; }
private:
  SwmPortMibCounters _port[NPORTS];
};
};

#endif
