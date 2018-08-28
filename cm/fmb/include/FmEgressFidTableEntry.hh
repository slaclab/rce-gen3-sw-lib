#ifndef Bali_FmEgressFidTableEntry_hh
#define Bali_FmEgressFidTableEntry_hh


#include "fm/FmMReg.hh"

class SwmFidTableEntry;

namespace Bali {
  class SwmEgressFidTableEntry;

  class FmEgressFidTableEntry {
  public:
    FmEgressFidTableEntry() {}
    FmEgressFidTableEntry(const SwmEgressFidTableEntry&);
    ~FmEgressFidTableEntry() {}

    FmEgressFidTableEntry& operator=(const SwmEgressFidTableEntry&);
    FmEgressFidTableEntry& operator=(const SwmFidTableEntry&);

    operator SwmEgressFidTableEntry() const;

  private:
    FmMReg _data;
  };
};

#endif
