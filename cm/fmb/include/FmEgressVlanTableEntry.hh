#ifndef Bali_FmEgressVlanTableEntry_hh
#define Bali_FmEgressVlanTableEntry_hh


#include "fm/FmMReg.hh"

class SwmVlanTableEntry;

namespace Bali {
  class SwmEgressVlanTableEntry;

  class FmEgressVlanTableEntry {
  public:
    FmEgressVlanTableEntry() {}
    FmEgressVlanTableEntry(const SwmEgressVlanTableEntry&);
    FmEgressVlanTableEntry(const SwmVlanTableEntry&);
    ~FmEgressVlanTableEntry() {}

    FmEgressVlanTableEntry& operator=(const SwmEgressVlanTableEntry&);
    FmEgressVlanTableEntry& operator=(const SwmVlanTableEntry&);

    operator SwmEgressVlanTableEntry() const;

  private:
    FmMReg _data[2];
  };
};

#endif
