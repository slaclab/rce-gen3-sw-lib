
#include "fmb/FmEgressFidTableEntry.hh"

#include "fmb/SwmEgressFidTableEntry.hh"
#include "fm/SwmFidTableEntry.hh"

using namespace Bali;

FmEgressFidTableEntry::FmEgressFidTableEntry(const SwmEgressFidTableEntry& e)
{
  *this = e;
}


FmEgressFidTableEntry::operator SwmEgressFidTableEntry() const
{
  FmPortMask forwarding( (_data&0x1fffffe)|1 );
  return SwmEgressFidTableEntry( forwarding );
}

FmEgressFidTableEntry& FmEgressFidTableEntry::operator=(const SwmEgressFidTableEntry& e)
{
  unsigned forwarding = e.forwarding();
  _data = forwarding & ~1;
  return *this;
}

FmEgressFidTableEntry& FmEgressFidTableEntry::operator=(const SwmFidTableEntry& e)
{
  unsigned forwarding = e.forwarding();
  _data = forwarding & ~1;
  return *this;
}

