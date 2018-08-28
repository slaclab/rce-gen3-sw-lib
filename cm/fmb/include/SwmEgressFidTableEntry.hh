#ifndef Bali_SwmEgressFidTableEntry_hh
#define Bali_SwmEgressFidTableEntry_hh


#include "fm/FmPort.hh"

namespace Bali {
  class SwmEgressFidTableEntry {
  public:
    SwmEgressFidTableEntry();
    SwmEgressFidTableEntry(FmPortMask forwarding);

    SwmEgressFidTableEntry& operator=(const SwmEgressFidTableEntry&);

    FmPortMask       forwarding() const { return _forwarding; }

  private:
    FmPortMask _forwarding;
  };
};

#endif
