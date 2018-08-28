
#include "fm/FmReg.hh"
#include "cm_svc/PlxBridge.hh"

#include <stdio.h>
#include <stdlib.h>

static int _status = 0;

int  FmReg::status() { return _status; }
void FmReg::clear () { _status = 0; }

//  Memory map PCIE interface
//    (assume PLX setup is done elsewhere and persists)
void FmReg::init() { printf("FmReg::init\n"); }

//  Write a register over PCIE
FmReg& FmReg::operator=(const unsigned v)
{
  uint32_t* addr = reinterpret_cast<uint32_t*>(this);
  cm::svc::PlxBridge::instance().write(addr,v);
  return *this;
}

//  Read a register over PCIE
FmReg::operator unsigned() const
{
  const uint32_t* addr = reinterpret_cast<const uint32_t*>(this);
  return cm::svc::PlxBridge::instance().read(addr);
}
