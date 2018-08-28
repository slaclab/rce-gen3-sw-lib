
#include "fmb/SwmIngressFidTableEntry.hh"

using namespace Bali;

SwmIngressFidTableEntry::SwmIngressFidTableEntry() :
  _listening (0),
  _learning  (0),
  _forwarding(0)
{
}

SwmIngressFidTableEntry::SwmIngressFidTableEntry(FmPortMask listening,
				   FmPortMask learning,
				   FmPortMask forwarding) :
  _listening (listening),
  _learning  (learning),
  _forwarding(forwarding)
{
}

SwmIngressFidTableEntry& SwmIngressFidTableEntry::operator=(const SwmIngressFidTableEntry& c)
{
  _listening  = c._listening;
  _learning   = c._learning;
  _forwarding = c._forwarding;
  return *this;
}
