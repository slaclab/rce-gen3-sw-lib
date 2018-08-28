
#include "fm/SwmGlobalMibCounters.hh"

#include "fm/FmTahoe.hh"

SwmGlobalMibCounters::SwmGlobalMibCounters()
{
}

SwmGlobalMibCounters::SwmGlobalMibCounters( FmTahoe& c )
{
 cmGlobalLowDrops = c.cmGlobalLowDrops;
 cmGlobalHighDrops = c.cmGlobalHighDrops;
 cmGlobalPrivilegeDrops = c.cmGlobalPrivilegeDrops;
 for(unsigned j=0; j<25; j++)
   cmTxDrops[j] = c.cmTxDrops[j];
 for(unsigned j=0; j<17; j++)
   trigCount[j] = c.trigCount[j];
 for(unsigned j=0; j<32; j++)
   vlanUnicast      [j] = c.vlanUnicast      [j];
 for(unsigned j=0; j<32; j++)
   vlanXcast        [j] = c.vlanXcast        [j];
 for(unsigned j=0; j<32; j++)
   vlanUnicastOctets[j] = c.vlanUnicastOctets[j];
 for(unsigned j=0; j<32; j++)
   vlanXcastOctets  [j] = c.vlanXcastOctets  [j];
}

SwmGlobalMibCounters::~SwmGlobalMibCounters()
{
}

SwmGlobalMibCounters operator-(const SwmGlobalMibCounters& a,
			       const SwmGlobalMibCounters& b)
{
  SwmGlobalMibCounters v;
  SwmMibCounter* p_a = (SwmMibCounter*)&a;
  SwmMibCounter* p_b = (SwmMibCounter*)&b;
  SwmMibCounter* p_v = (SwmMibCounter*)&v;
  SwmMibCounter* p_end = (SwmMibCounter*)(&v + 1);
  while( p_v < p_end )
    *p_v++ = *p_a++ - *p_b++;
  return v;
}
