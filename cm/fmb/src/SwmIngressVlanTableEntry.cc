
#include "fmb/SwmIngressVlanTableEntry.hh"

using Bali::SwmIngressVlanTableEntry;

SwmIngressVlanTableEntry::SwmIngressVlanTableEntry(unsigned fid,
                                                   unsigned port_membership_mask,
                                                   unsigned trig_id, 
                                                   unsigned counter, 
                                                   unsigned reflect,
                                                   unsigned trap_igmp)
{
  _data[0] = (reflect!=0) ? 2 : 0;
  _data[0] |= (counter&0x3f)<<2;
  _data[0] |= (trig_id&0x1f)<<8;
  _data[1] = 0;
  unsigned j=0;
  while( j < 9 ) {
    if (port_membership_mask & (1<<j))
      _data[0] |= 1<<(2*j+15);
    j++;
  }
  while( j < 25 ) {
    if (port_membership_mask & (1<<j))
      _data[1] |= 1<<(2*j-17);
    j++;
  }
  _data[2] = (fid&0xfff) | (trap_igmp ? 0x2000 : 0);
  _data[3] = 0;
}

SwmIngressVlanTableEntry& SwmIngressVlanTableEntry::operator=(const SwmIngressVlanTableEntry& e)
{
  _data[0] = e._data[0];
  _data[1] = e._data[1];
  _data[2] = e._data[2];
  _data[3] = e._data[3];
  return *this;
}

unsigned SwmIngressVlanTableEntry::members() const
{
  unsigned m = 0;
  unsigned v = _data[0]>>15;
  for(unsigned j=0; j<9; j++,v>>=1)
    m |= v&(1<<j);
  unsigned n = 0;
  v = _data[1]>>1;
  for(unsigned j=9; j<=24; j++,v>>=1)
    n |= v&(1<<(j-9));
  return (m|(n<<9));
}

unsigned SwmIngressVlanTableEntry::tagged() const
{
  return members();
}
