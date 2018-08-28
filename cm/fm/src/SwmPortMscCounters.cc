
#include "fm/SwmPortMscCounters.hh"

#include "fm/FmPortMscCounters.hh"

SwmPortMscCounters::SwmPortMscCounters()
{
}

SwmPortMscCounters::SwmPortMscCounters( FmPortMscCounters& c )
{
  txPause = c.txPause;
  txFcsErrors = c.txFcsErrors;
  rxJabbers = c.rxJabbers;
}

SwmPortMscCounters::~SwmPortMscCounters()
{
}

SwmPortMscCounters operator-(const SwmPortMscCounters& a,
			     const SwmPortMscCounters& b)
{
  SwmPortMscCounters v;
  //
  //  This implementation is causing strange side effects
  //  ("double" output format shows no base only exponent)
  //
#if 0
  unsigned* p_a = (unsigned*)&a;
  unsigned* p_b = (unsigned*)&b;
  unsigned* p_v = (unsigned*)&v;
  unsigned* p_end = (unsigned*)(&v+1);
  while( p_v < p_end )
    *p_v++ = *p_a++ - *p_b++;
#else
  v.txPause     = a.txPause     - b.txPause;
  v.txFcsErrors = a.txFcsErrors - b.txFcsErrors;
  v.rxJabbers   = a.rxJabbers   - b.rxJabbers;
#endif
  return v;
}
