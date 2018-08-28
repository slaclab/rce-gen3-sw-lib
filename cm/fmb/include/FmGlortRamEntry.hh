#ifndef Bali_FmGlortRamEntry_hh
#define Bali_FmGlortRamEntry_hh

#include "fm/FmMReg.hh"
#include "fm/FmPort.hh"

namespace Bali {
  class FmBali;
  class SwmGlortRamEntry;

  class FmGlortRamEntry {
  public:
    FmGlortRamEntry() {}

    FmGlortRamEntry& operator=(const SwmGlortRamEntry&);
    operator SwmGlortRamEntry() const;

  public:
    FmMReg       a;
    FmMReg       b;
  private:
    ~FmGlortRamEntry() {}
    friend class FmBali;
  };
};

#endif
