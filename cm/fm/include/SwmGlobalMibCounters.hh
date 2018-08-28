#ifndef SwmGlobalMibCounters_hh
#define SwmGlobalMibCounters_hh


#include "fm/SwmMibCounter.hh"

class FmTahoe;

class SwmGlobalMibCounters {
public:
  SwmGlobalMibCounters();
  SwmGlobalMibCounters( FmTahoe& );
  ~SwmGlobalMibCounters();

  SwmMibCounter cmGlobalLowDrops;
  SwmMibCounter cmGlobalHighDrops;
  SwmMibCounter cmGlobalPrivilegeDrops;
  SwmMibCounter cmTxDrops[25];
  SwmMibCounter trigCount[17];
  SwmMibCounter vlanUnicast      [32];
  SwmMibCounter vlanXcast        [32];
  SwmMibCounter vlanUnicastOctets[32];
  SwmMibCounter vlanXcastOctets  [32];
};

SwmGlobalMibCounters operator-(const SwmGlobalMibCounters&,
			       const SwmGlobalMibCounters&);

#endif
