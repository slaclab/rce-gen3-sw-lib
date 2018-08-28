#ifndef FmSReg_hh
#define FmSReg_hh


#include "fm/FmReg.hh"

namespace Bali { class FmBali; class FmEplRegs; };

//
//
//

class FmSReg : public FmReg {
public:
  operator unsigned() const;
private:
  FmSReg() {}
  ~FmSReg() {}

  friend class FmTahoe;
  friend class FmBootStatus;
  friend class FmMibCounter;

  friend class Bali::FmBali;
  friend class Bali::FmEplRegs;
};

//
//  Read the value directly from the device
//
inline FmSReg::operator unsigned() const
{
  return FmReg::operator unsigned();
}

#endif
