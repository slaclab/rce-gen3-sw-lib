
#include "fm/SwmFidTableEntry.hh"

SwmFidTableEntry::SwmFidTableEntry() :
  _listening (0),
  _learning  (0),
  _forwarding(0)
{
}

SwmFidTableEntry::SwmFidTableEntry(FmPortMask listening,
				   FmPortMask learning,
				   FmPortMask forwarding) :
  _listening (listening),
  _learning  (learning),
  _forwarding(forwarding)
{
}

SwmFidTableEntry& SwmFidTableEntry::operator=(const SwmFidTableEntry& c)
{
  _listening  = c._listening;
  _learning   = c._learning;
  _forwarding = c._forwarding;
  return *this;
}
