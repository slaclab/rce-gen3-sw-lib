#ifndef Bali_FmGlortDestEntry_hh
#define Bali_FmGlortDestEntry_hh

#include "fm/FmMReg.hh"
#include "fm/FmPort.hh"

namespace Bali {
  class FmBali;
  class SwmGlortDestEntry;

  class FmGlortDestEntry {
  public:
    FmGlortDestEntry() {}

    FmGlortDestEntry& operator=(const SwmGlortDestEntry&);
    operator SwmGlortDestEntry() const;

  public:
    FmMReg       a;
    FmMReg       b;
  private:
    ~FmGlortDestEntry() {}
    friend class FmBali;
  };
};

#endif
