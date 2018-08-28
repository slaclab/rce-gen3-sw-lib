
#include "fmb/FmMacTcnFifoEntry.hh"

#include "fmb/SwmMacTcnFifoEntry.hh"
#include "fmb/SwmMacTableEntry.hh"

using namespace Bali;

FmMacTcnFifoEntry&  FmMacTcnFifoEntry::operator=(const SwmMacTcnFifoEntry& e)
{
  _data[0] = e.entry()._data[0];
  _data[1] = e.entry()._data[1];
  _data[2] = e.entry()._data[2];
  _data[3] = 
    ((e.index()&0xfff)<< 0) |
    ((e.set  ()&  0x7)<<12) |
    ((e.type ()&  0x7)<<15);
  return *this;
}

FmMacTcnFifoEntry::operator SwmMacTcnFifoEntry() const
{
  SwmMacTableEntry   m;
  m._data[0] = _data[0];
  m._data[1] = _data[1];
  m._data[2] = _data[2];

  SwmMacTcnFifoEntry e(m,
                       (_data[3]>>0)&0xfff,
                       (_data[3]>>12)&0x7,
                       (_data[3]>>15)&0x7,
                       0);
  return e;
}
