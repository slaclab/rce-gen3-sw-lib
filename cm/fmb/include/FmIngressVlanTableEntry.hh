#ifndef Bali_FmIngressVlanTableEntry_hh
#define Bali_FmIngressVlanTableEntry_hh


#include "fm/FmMReg.hh"

class SwmVlanTableEntry;

namespace Bali {
  class SwmIngressVlanTableEntry;

  class FmIngressVlanTableEntry {
  public:
    FmIngressVlanTableEntry() {}
    FmIngressVlanTableEntry(const SwmIngressVlanTableEntry&);
    FmIngressVlanTableEntry(const SwmVlanTableEntry&);
    ~FmIngressVlanTableEntry() {}

    FmIngressVlanTableEntry& operator=(const SwmIngressVlanTableEntry&);
    FmIngressVlanTableEntry& operator=(const SwmVlanTableEntry&);

    operator SwmIngressVlanTableEntry() const;

  private:
    FmMReg _data[4];
  };
};

#endif
