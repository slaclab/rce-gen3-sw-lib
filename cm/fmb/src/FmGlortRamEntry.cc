
#include "fmb/FmGlortRamEntry.hh"

#include "fmb/SwmGlortRamEntry.hh"

using namespace Bali;

FmGlortRamEntry& FmGlortRamEntry::operator=( const SwmGlortRamEntry& c )
{
  a = c.a;
  b = c.b;
  return* this;
}

FmGlortRamEntry::operator SwmGlortRamEntry() const
{
  const FmGlortRamEntry& c = *this;
  SwmGlortRamEntry v;
  v.a = c.a;
  v.b = c.b;
  return v;
}

