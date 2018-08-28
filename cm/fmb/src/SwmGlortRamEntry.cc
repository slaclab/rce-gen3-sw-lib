
#include "fmb/SwmGlortRamEntry.hh"

#include "fmb/FmGlortRamEntry.hh"

#include <string.h>

using namespace Bali;

SwmGlortRamEntry::SwmGlortRamEntry()
{
}

SwmGlortRamEntry::SwmGlortRamEntry(unsigned dest_index)
{
  a = (0x6) | (dest_index<<3) | (1<<31);
  b = 0;
}


SwmGlortRamEntry::SwmGlortRamEntry(unsigned dest_index,
                                   unsigned sub_indexa_pos,
                                   unsigned sub_indexa_nb,
                                   unsigned sub_indexb_pos,
                                   unsigned sub_indexb_nb,
                                   unsigned dest_count,
                                   unsigned rotation) 
{
  a = (0x4) | (dest_index<<3) | 
    (sub_indexa_pos<<15) | (sub_indexa_nb<<19) |
    (sub_indexb_pos<<23) | (sub_indexb_nb<<27) |
    (dest_count<<31);
  b = ((dest_count&0xf)>>1);
}


SwmGlortRamEntry::~SwmGlortRamEntry()
{
}

SwmGlortRamEntry& SwmGlortRamEntry::operator=(const SwmGlortRamEntry& c)
{
  a = c.a;
  b = c.b;
  return *this;
}

bool SwmGlortRamEntry::parityError() const 
{
  return a&1;
}

unsigned SwmGlortRamEntry::strict() const 
{
  return (a>>1)&3;
}

unsigned SwmGlortRamEntry::destIndex() const 
{
  return (a>>3)&0xfff;
}

unsigned SwmGlortRamEntry::subIndexA_pos() const 
{
  return (a>>15)&0xf;
}

unsigned SwmGlortRamEntry::subIndexA_nb() const 
{
  return (a>>19)&0xf;
}

unsigned SwmGlortRamEntry::subIndexB_pos() const 
{
  return (a>>23)&0xf;
}

unsigned SwmGlortRamEntry::subIndexB_nb() const 
{
  return (a>>27)&0xf;
}

unsigned SwmGlortRamEntry::destCount() const 
{
  return (a>>31) | ((b&0x7)<<1);
}

unsigned SwmGlortRamEntry::hashRotation() const 
{
  return (b>>3)&1;
}
