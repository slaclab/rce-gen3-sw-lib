#ifndef Bali_SwmIngressFidTableEntry_hh
#define Bali_SwmIngressFidTableEntry_hh


#include "fm/FmPort.hh"

namespace Bali {
  class SwmIngressFidTableEntry {
  public:
    SwmIngressFidTableEntry();
    SwmIngressFidTableEntry(FmPortMask listening,
                            FmPortMask learning,
                            FmPortMask forwarding);

    SwmIngressFidTableEntry& operator=(const SwmIngressFidTableEntry&);

    FmPortMask       listening () const { return _listening; }
    FmPortMask       learning  () const { return _learning; }
    FmPortMask       forwarding() const { return _forwarding; }

  private:
    FmPortMask _listening, _learning, _forwarding;
  };
};

#endif
