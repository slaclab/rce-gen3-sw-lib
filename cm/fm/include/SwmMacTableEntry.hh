#ifndef SwmMacTableEntry_hh
#define SwmMacTableEntry_hh


#include "fm/FmPort.hh"

class SwmMacTableEntry {
public:
  SwmMacTableEntry() {}
  SwmMacTableEntry(const ether_addr_t& addr,
		   FmPortMask portmask,
		   unsigned   fid,
		   unsigned   trig_id,
		   unsigned   lock,
		   unsigned   valid=1);
  SwmMacTableEntry(const SwmMacTableEntry&);

  SwmMacTableEntry& operator=(const SwmMacTableEntry&);

  int operator==(const SwmMacTableEntry&) const;

  ether_addr_t  addr() const;
  unsigned char addr_octet(unsigned) const;
  FmPortMask    port_mask() const;
  unsigned      learning_group() const;
  unsigned      trigger() const;
  unsigned      locked() const;
  unsigned      valid() const;
  unsigned      parity() const;

public:
  void          invalidate();
  void          set_port_mask(FmPortMask);
  void          set_trigger  (unsigned);
  void          set_lock     (unsigned);
  void          print() const;

private:
  unsigned _data[4];
  friend class FmMacTableEntry;
};

//  Tests Mac Address and Learning Group/VLAN
inline int SwmMacTableEntry::operator==(const SwmMacTableEntry& a) const
{
  return !(((_data[0]^a._data[0])&~3L) | ((_data[1]^a._data[1])&~0xc0000000));
}

#endif
