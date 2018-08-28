#ifndef FmMReg_hh
#define FmMReg_hh


#include "fm/FmReg.hh"

//
//
//
namespace Bali {  
  class FmBali;
  class FmEplRegs;
  class FmIngressFidTableEntry;
  class FmEgressFidTableEntry;
  class FmIngressVlanTableEntry;
  class FmEgressVlanTableEntry;
  class FmMacTableEntry;
  class FmMacTcnFifoEntry;
  class FmGlortDestEntry;
  class FmGlortRamEntry;
  class FmStatsCfg;
};

class FmMReg : public FmReg {
public:
  FmMReg() {}
  FmMReg& operator=(const unsigned);
  operator unsigned() const;
  FmMReg& operator|=(const unsigned);
  FmMReg& operator&=(const unsigned);
  void setBits(unsigned bits,unsigned value);
private:
  ~FmMReg() {}

  friend class FmTahoe;
  friend class FmMacTableEntry;
  friend class FmVlanTableEntry;
  friend class FmFidTableEntry;
  friend class FmEplRegs;
  friend class FmPortMscCounters;
  friend class FmSoftReset;
  
  friend class Bali::FmBali;
  friend class Bali::FmEplRegs;
  friend class Bali::FmIngressFidTableEntry;
  friend class Bali::FmEgressFidTableEntry;
  friend class Bali::FmIngressVlanTableEntry;
  friend class Bali::FmEgressVlanTableEntry;
  friend class Bali::FmMacTableEntry;
  friend class Bali::FmMacTcnFifoEntry;
  friend class Bali::FmGlortDestEntry;
  friend class Bali::FmGlortRamEntry;
  friend class Bali::FmStatsCfg;
};

//
//  Write the value to the device
//
inline FmMReg& FmMReg::operator=(const unsigned v)
{
  FmReg::operator=(v);
  return *this;
}

//
//  Read the value directly from the device
//
inline FmMReg::operator unsigned() const
{
  return FmReg::operator unsigned();
}

//
//  Modify the value
//
inline FmMReg& FmMReg::operator|=(const unsigned v)
{
  FmReg::operator=(v | FmReg::operator unsigned());
  return *this;
}

inline FmMReg& FmMReg::operator&=(const unsigned v)
{
  FmReg::operator=(v & FmReg::operator unsigned());
  return *this;
}

inline void FmMReg::setBits(unsigned bits, unsigned v)
{
  unsigned r = unsigned();
  r &= ~bits;
  r |= (v&bits);
}

#endif
