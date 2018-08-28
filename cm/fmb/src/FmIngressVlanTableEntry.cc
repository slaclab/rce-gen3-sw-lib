
#include "fmb/FmIngressVlanTableEntry.hh"

#include "fmb/SwmIngressVlanTableEntry.hh"
#include "fm/SwmVlanTableEntry.hh"

using namespace Bali;

FmIngressVlanTableEntry::FmIngressVlanTableEntry(const SwmIngressVlanTableEntry& e)
{
  _data[0] = e._data[0];
  _data[1] = e._data[1];
  _data[2] = e._data[2];
  _data[3] = e._data[3];
}


FmIngressVlanTableEntry::FmIngressVlanTableEntry(const SwmVlanTableEntry& e)
{
  _data[0] = e._data[0];
  _data[1] = e._data[1];
  _data[2] = 0;  // default to FID 0
  _data[3] = 0;
}


FmIngressVlanTableEntry::operator SwmIngressVlanTableEntry() const
{
  SwmIngressVlanTableEntry e;
  e._data[0] = _data[0];
  e._data[1] = _data[1];
  e._data[2] = _data[2];
  e._data[3] = _data[3];
  return e;
}

FmIngressVlanTableEntry& FmIngressVlanTableEntry::operator=(const SwmIngressVlanTableEntry& e)
{
  _data[0] = e._data[0];
  _data[1] = e._data[1];
  _data[2] = e._data[2];
  _data[3] = e._data[3];
  return *this;
}

FmIngressVlanTableEntry& FmIngressVlanTableEntry::operator=(const SwmVlanTableEntry& e)
{
  _data[0] = e._data[0];
  _data[1] = e._data[1];
  _data[2] = 0;  // default to FID 0
  _data[3] = 0;
  return *this;
}
