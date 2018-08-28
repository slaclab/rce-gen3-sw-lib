#ifndef FmCReg_hh
#define FmCReg_hh


#include "fm/FmReg.hh"

namespace Bali { class FmBali; };

//
//
//
class FmCReg : public FmReg {
public:
  FmCReg() {}
  FmCReg& operator=(const unsigned);
  operator unsigned() const;
private:
  ~FmCReg() {}

  friend class FmTahoe;
  friend class FmMacTableEntry;
  friend class FmVlanTableEntry;
  friend class FmFidTableEntry;
  friend class FmEplRegs;
  friend class FmSoftReset;

  friend class Bali::FmBali;
};

//
//  Write the value to the device
//
inline FmCReg& FmCReg::operator=(const unsigned v)
{
  FmReg::operator=(v);
  return *this;
}

//
//  Read the value directly from the device
//
inline FmCReg::operator unsigned() const
{
  return FmReg::operator unsigned();
}

#endif
