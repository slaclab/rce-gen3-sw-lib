
#include "fmb/SwmEgressVlanTableEntry.hh"

using Bali::SwmEgressVlanTableEntry;

SwmEgressVlanTableEntry::SwmEgressVlanTableEntry(unsigned fid,
                                                 unsigned mtu_index,
                                                 unsigned port_membership_mask,
                                                 unsigned trig_id)
{
  _data[0] = ((mtu_index&0x7)<<1) | 
    ((port_membership_mask&0x1ffffff)<<4);
  _data[1] = (fid&0xfff) |
    ((trig_id&0x3f)<<12);
}

SwmEgressVlanTableEntry& SwmEgressVlanTableEntry::operator=(const SwmEgressVlanTableEntry& e)
{
  _data[0] = e._data[0];
  _data[1] = e._data[1];
  return *this;
}


