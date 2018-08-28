#ifndef Bali_FmIngressFidTableEntry_hh
#define Bali_FmIngressFidTableEntry_hh


#include "fm/FmMReg.hh"

class SwmFidTableEntry;

namespace Bali {

  class FmIngressFidTableEntry {
  public:
    FmIngressFidTableEntry() {}
    FmIngressFidTableEntry(const SwmFidTableEntry&);
    ~FmIngressFidTableEntry() {}

    FmIngressFidTableEntry& operator=(const SwmFidTableEntry&);

    operator SwmFidTableEntry() const;

  private:
    FmMReg _data[2];
  };
};

#endif
