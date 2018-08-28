
#include "fmb/FmGlortDestEntry.hh"

#include "fmb/SwmGlortDestEntry.hh"

using namespace Bali;

FmGlortDestEntry& FmGlortDestEntry::operator=( const SwmGlortDestEntry& c )
{
  a = c.a;
  b = c.b;
  return* this;
}

FmGlortDestEntry::operator SwmGlortDestEntry() const
{
  const FmGlortDestEntry& c = *this;
  SwmGlortDestEntry v;
  v.a = c.a;
  v.b = c.b;
  return v;
}

