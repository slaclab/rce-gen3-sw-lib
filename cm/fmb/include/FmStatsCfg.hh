#ifndef Bali_FmStatsCfg_hh
#define Bali_FmStatsCfg_hh


#include "fm/FmMReg.hh"

namespace Bali {
  class FmStatsCfg {
  public:
    FmStatsCfg() {}
    FmStatsCfg& operator=(const unsigned);
    operator unsigned() const;
  private:
    ~FmStatsCfg() {}
    FmMReg _data[2];
    friend class FmBali;
  };
};

#endif
