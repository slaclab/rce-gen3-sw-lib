
#include "fm/SwmAllPortMibCounters.hh"

#include "fm/FmTahoe.hh"

SwmAllPortMibCounters::SwmAllPortMibCounters() {}

SwmAllPortMibCounters::SwmAllPortMibCounters( const FmTahoe& t )
{
  for(unsigned j=0; j<NPORTS; j++)
    _port[j] = SwmPortMibCounters( t.port_mib[j]);
}

SwmAllPortMibCounters::~SwmAllPortMibCounters() {}

SwmAllPortMibCounters& SwmAllPortMibCounters::operator=(const SwmAllPortMibCounters& c)
{
  for(unsigned j=0; j<NPORTS; j++)
    _port[j] = c._port[j];
  return *this;
}

SwmAllPortMibCounters& SwmAllPortMibCounters::operator-=(const SwmAllPortMibCounters& c)
{
  for(unsigned j=0; j<NPORTS; j++)
    _port[j] = _port[j] - c._port[j];
  return *this;
}

