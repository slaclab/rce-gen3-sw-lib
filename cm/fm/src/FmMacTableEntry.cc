
#include "fm/FmMacTableEntry.hh"

#include "fm/SwmMacTableEntry.hh"

FmMacTableEntry&  FmMacTableEntry::operator=(const SwmMacTableEntry& e)
{
  _data[0] = e._data[0];
  _data[1] = e._data[1];
  _data[2] = e._data[2];
  return *this;
}

FmMacTableEntry::operator SwmMacTableEntry() const
{
  SwmMacTableEntry e;
  e._data[0] = _data[0];
  e._data[1] = _data[1];
  e._data[2] = _data[2];
  return e;
}
