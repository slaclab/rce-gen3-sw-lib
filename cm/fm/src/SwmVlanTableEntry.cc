
#include "fm/SwmVlanTableEntry.hh"

SwmVlanTableEntry::SwmVlanTableEntry(unsigned port_membership_mask, unsigned port_tag_mask,
				     unsigned trig_id, unsigned counter, unsigned reflect)
{
  _data[0] = (reflect!=0) ? 2 : 0;
  _data[0] |= (counter&0x3f)<<2;
  _data[0] |= (trig_id&0x1f)<<8;
  _data[1] = 0;
  unsigned j=0;
  while( j < 9 ) {
    if (port_tag_mask & (1<<j))
      _data[0] |= 1<<(2*j+14);
    if (port_membership_mask & (1<<j))
      _data[0] |= 1<<(2*j+15);
    j++;
  }
  while( j < 25 ) {
    if (port_tag_mask & (1<<j))
      _data[1] |= 1<<(2*j-18);
    if (port_membership_mask & (1<<j))
      _data[1] |= 1<<(2*j-17);
    j++;
  }

  unsigned parity = reflect ? 1 : 0;
  unsigned p = port_membership_mask ^ port_tag_mask ^ (trig_id&0x1f) ^ (counter&0x3f);
  while( p ) {
    if (p&1) parity++;
    p >>= 1;
  }
  _data[0] |= parity&1;
}

SwmVlanTableEntry& SwmVlanTableEntry::operator=(const SwmVlanTableEntry& e)
{
  _data[0] = e._data[0];
  _data[1] = e._data[1];
  return *this;
}

unsigned SwmVlanTableEntry::members() const
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

unsigned SwmVlanTableEntry::tagged () const
{
  unsigned m = 0;
  unsigned v = _data[0]>>14;
  for(unsigned j=0; j<9; j++,v>>=1)
    m |= v&(1<<j);
  unsigned n = 0;
  v = _data[1]>>0;
  for(unsigned j=9; j<=24; j++,v>>=1)
    n |= v&(1<<(j-9));
  return (m|(n<<9));
}
