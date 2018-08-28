
#include "fmb/SwmEgressFidTableEntry.hh"

using namespace Bali;

SwmEgressFidTableEntry::SwmEgressFidTableEntry() :
  _forwarding(0)
{
}

SwmEgressFidTableEntry::SwmEgressFidTableEntry(FmPortMask forwarding) :
  _forwarding(forwarding)
{
}

SwmEgressFidTableEntry& SwmEgressFidTableEntry::operator=(const SwmEgressFidTableEntry& c)
{
  _forwarding = c._forwarding;
  return *this;
}
