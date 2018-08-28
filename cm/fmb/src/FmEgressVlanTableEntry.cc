
#include "fmb/FmEgressVlanTableEntry.hh"

#include "fmb/SwmEgressVlanTableEntry.hh"
#include "fm/SwmVlanTableEntry.hh"

using namespace Bali;

FmEgressVlanTableEntry::FmEgressVlanTableEntry(const SwmEgressVlanTableEntry& e)
{
  _data[0] = e._data[0];
  _data[1] = e._data[1];
}


FmEgressVlanTableEntry::FmEgressVlanTableEntry(const SwmVlanTableEntry& e)
{
  _data[0] = (e.members())<<4;
  _data[1] = 0;
}


FmEgressVlanTableEntry::operator SwmEgressVlanTableEntry() const
{
  SwmEgressVlanTableEntry e;
  e._data[0] = _data[0];
  e._data[1] = _data[1];
  return e;
}

FmEgressVlanTableEntry& FmEgressVlanTableEntry::operator=(const SwmEgressVlanTableEntry& e)
{
  _data[0] = e._data[0];
  _data[1] = e._data[1];
  return *this;
}

FmEgressVlanTableEntry& FmEgressVlanTableEntry::operator=(const SwmVlanTableEntry& e)
{
  _data[0] = (e.members())<<4;
  _data[1] = 0;
  return *this;
}
