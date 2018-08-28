
#include "fmb/SwmPortMibCounters.hh"

#include "fmb/FmPortMibCounters.hh"

Bali::SwmPortMibCounters::SwmPortMibCounters()
{
}

#include <stdio.h>

Bali::SwmPortMibCounters::SwmPortMibCounters( const Bali::FmPortMibCounters& c )
{
#define ADD_STAT(group,member0,member1)         \
  member0 = c.Stat##group.member0;              \
  member1 = c.Stat##group.member1
#define ADD_RSRV(group,len) {}

  ADD_STAT_PORTCOUNTERS

#undef ADD_STAT
#undef ADD_RSRV
}

Bali::SwmPortMibCounters::~SwmPortMibCounters()
{
}

Bali::SwmPortMibCounters& Bali::SwmPortMibCounters::operator=(const Bali::SwmPortMibCounters& c)
{
#define ADD_STAT(group,member0,member1)         \
  member0 = c.member0;                          \
  member1 = c.member1
#define ADD_RSRV(group,len) {}

  ADD_STAT_PORTCOUNTERS

#undef ADD_STAT
#undef ADD_RSRV

  return *this;
}

Bali::SwmPortMibCounters operator-(const Bali::SwmPortMibCounters& a,
                                   const Bali::SwmPortMibCounters& b)
{
  Bali::SwmPortMibCounters v;
  SwmMibCounter* p_a = (SwmMibCounter*)&a;
  SwmMibCounter* p_b = (SwmMibCounter*)&b;
  SwmMibCounter* p_v = (SwmMibCounter*)&v;
  SwmMibCounter* p_end = (SwmMibCounter*)(&v + 1);
  while( p_v < p_end )
    *p_v++ = *p_a++ - *p_b++;
  return v;
}
