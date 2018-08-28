#ifndef SwmAllPortMibCounters_hh
#define SwmAllPortMibCounters_hh


#include "fm/SwmPortMibCounters.hh"

class FmTahoe;

class SwmAllPortMibCounters {
public:
  enum { NPORTS=25 };
  SwmAllPortMibCounters();
  SwmAllPortMibCounters( const FmTahoe& );
  ~SwmAllPortMibCounters();

  SwmAllPortMibCounters& operator=(const SwmAllPortMibCounters& c);
  SwmAllPortMibCounters& operator-=(const SwmAllPortMibCounters& c);

  const SwmPortMibCounters& port(unsigned index) const { return _port[index]; }
private:
  SwmPortMibCounters _port[NPORTS];
};

#endif
