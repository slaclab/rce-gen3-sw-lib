#ifndef Bali_FmMacTableEntry_hh
#define Bali_FmMacTableEntry_hh


#include "fm/FmMReg.hh"

namespace Bali {
  class SwmMacTableEntry;

  class FmMacTableEntry {
  public:
    FmMacTableEntry() {}

    FmMacTableEntry& operator=(const SwmMacTableEntry&);

    operator SwmMacTableEntry() const;

    FmMReg _data[4];

  private:
    ~FmMacTableEntry() {}
    friend class FmBali;
  };
};

#endif
