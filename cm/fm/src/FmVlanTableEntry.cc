
#include "fm/FmVlanTableEntry.hh"

#include "fm/SwmVlanTableEntry.hh"

FmVlanTableEntry::FmVlanTableEntry(const SwmVlanTableEntry& e)
{
  _data[0] = e._data[0];
  _data[1] = e._data[1];
}


FmVlanTableEntry::operator SwmVlanTableEntry() const
{
  SwmVlanTableEntry e;
  e._data[0] = _data[0];
  e._data[1] = _data[1];
  return e;
}

FmVlanTableEntry& FmVlanTableEntry::operator=(const SwmVlanTableEntry& e)
{
  _data[0] = e._data[0];
  _data[1] = e._data[1];
  return *this;
}
