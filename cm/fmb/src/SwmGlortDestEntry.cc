
#include "fmb/SwmGlortDestEntry.hh"

#include "fmb/FmGlortDestEntry.hh"

#include <string.h>

using namespace Bali;

SwmGlortDestEntry::SwmGlortDestEntry()
{
}

SwmGlortDestEntry::SwmGlortDestEntry(FmPortMask dest_mask,
                                     unsigned   ip_mcast)
{
  a = (unsigned(dest_mask)<<1) | (ip_mcast<<26);
  b = (ip_mcast>>6);
}


SwmGlortDestEntry::~SwmGlortDestEntry()
{
}

SwmGlortDestEntry& SwmGlortDestEntry::operator=(const SwmGlortDestEntry& c)
{
  a = c.a;
  b = c.b;
  return *this;
}

bool SwmGlortDestEntry::parityError() const 
{
  return a&1;
}

FmPortMask SwmGlortDestEntry::destMask() const 
{
  return FmPortMask((a>>1)&FmPortMask::allPorts());
}

unsigned SwmGlortDestEntry::ip_mcast_index() const 
{
  unsigned v = (a>>26) | ((b&0xff)<<6);
  return v;
}
