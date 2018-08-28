
#include "fm/SwmMacTableEntry.hh"

#include "fm/FmMacTableEntry.hh"

#include <stdio.h>
#include <string.h>

SwmMacTableEntry::SwmMacTableEntry(const ether_addr_t& addr,
				   FmPortMask portmask,
				   unsigned   fid,
				   unsigned   trig_id,
				   unsigned   lock,
				   unsigned   valid)
{
  unsigned adx =
    (addr.ether_addr_octet[5])     |
    (addr.ether_addr_octet[4]<<8)  |
    (addr.ether_addr_octet[3]<<16) |
    (addr.ether_addr_octet[2]<<24);
  _data[0] = adx << 2;

  unsigned adx2 =
    (addr.ether_addr_octet[1]) |
    (addr.ether_addr_octet[0]<<8);
  _data[1] =
    (adx >> 30) |
    (adx2 << 2) |
    ((fid&0xfff) << 18) |
    ((trig_id&0x1f) << 30);

  _data[2] =
    ((trig_id&0x1f) >> 2) |
    (valid ? 0x8 : 0) |          // valid
    (lock ? 0x10 : 0) |          // lock
                                 // age
    (unsigned(portmask) << 6);
}

SwmMacTableEntry::SwmMacTableEntry(const SwmMacTableEntry& e)
{
  _data[0] = e._data[0];
  _data[1] = e._data[1];
  _data[2] = e._data[2];
}

SwmMacTableEntry& SwmMacTableEntry::operator=(const SwmMacTableEntry& e)
{
  _data[0] = e._data[0];
  _data[1] = e._data[1];
  _data[2] = e._data[2];
  return *this;
}

ether_addr_t  SwmMacTableEntry::addr() const
{
  ether_addr_t a;
  a.ether_addr_octet[0] = (_data[1]>>10)&0xff;
  a.ether_addr_octet[1] = (_data[1]>> 2)&0xff;
  a.ether_addr_octet[2] = (_data[0]>>26)&0xff;
  a.ether_addr_octet[3] = (_data[0]>>18)&0xff;
  a.ether_addr_octet[4] = (_data[0]>>10)&0xff;
  a.ether_addr_octet[5] = (_data[0]>> 2)&0xff;
  return a;
}

unsigned char SwmMacTableEntry::addr_octet(unsigned i) const
{
  switch(i) {
  case 0: return (_data[0]>> 2) & 0xff;
  case 1: return (_data[0]>>10) & 0xff;
  case 2: return (_data[0]>>18) & 0xff;
  case 3: return ((_data[0]>>26) | (_data[1]<<6)) & 0xff;
  case 4: return (_data[1]>>2) & 0xff;
  case 5: return (_data[1]>>10) & 0xff;
  default: return 0;
  }
}

FmPortMask SwmMacTableEntry::port_mask() const
{
  return FmPortMask(_data[2] >> 6);
}

unsigned SwmMacTableEntry::learning_group() const
{
  return (_data[1] >> 18) & 0xfff;
}

unsigned SwmMacTableEntry::trigger() const
{
  return (_data[1]>>30) | ((_data[2]&0x7) << 2);
}

unsigned SwmMacTableEntry::locked() const
{
  return _data[2] & 0x10;
}

unsigned SwmMacTableEntry::valid() const
{
  return _data[2] & 0x8;
}

unsigned SwmMacTableEntry::parity() const
{
  return _data[0] & 0x1;
}

void SwmMacTableEntry::invalidate()
{
  _data[2] &= 0xfffffff7;
}

void SwmMacTableEntry::print() const
{
  char buf[256];
  sprintf(buf,"%02x:%02x:%02x:%02x:%02x:%02x ",
	  addr_octet(5),
	  addr_octet(4),
	  addr_octet(3),
	  addr_octet(2),
	  addr_octet(1),
	  addr_octet(0)
	  );
  sprintf(buf+strlen(buf),"%08x %s %08x %08x %x",
	  unsigned(port_mask()), locked() ? "Static" : "Dynamic",
	  learning_group(), trigger(), parity());
  printf("%s\n",buf);
}

void SwmMacTableEntry::set_port_mask(FmPortMask m)
{
  _data[2] &= 0x1f;
  _data[2] |= (unsigned(m) << 6);
}

void SwmMacTableEntry::set_trigger(unsigned m)
{
  _data[1] &= ~0xc0000000;
  _data[2] &= ~0x0000001c;
  _data[1] |= (m<<30);
  _data[2] |= (m>>2) & 0x7;
}

void SwmMacTableEntry::set_lock(unsigned m)
{
  if (m)
    _data[2] |= 0x10;
  else
    _data[2] &=~0x10;
}

