
#include "fmb/SwmMacTableEntry.hh"

#include "fmb/FmMacTableEntry.hh"

using Bali::SwmMacTableEntry;

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
  _data[0] = adx;

  unsigned adx2 =
    (addr.ether_addr_octet[1]) |
    (addr.ether_addr_octet[0]<<8);
  _data[1] =
    (adx2 << 0) |
    ((fid&0xfff) << 16) |
    ((valid==0 ? 0 : (lock==0 ? 2:3)) << 28);

  _data[2] =
    ((trig_id&0x1f) << 26) |
    (unsigned(portmask) << 0);
}

SwmMacTableEntry::SwmMacTableEntry(const ether_addr_t& addr,
				   unsigned   glort,
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
  _data[0] = adx;

  unsigned adx2 =
    (addr.ether_addr_octet[1]) |
    (addr.ether_addr_octet[0]<<8);
  _data[1] =
    (adx2 << 0) |
    ((fid&0xfff) << 16) |
    ((valid==0 ? 0 : (lock==0 ? 2:3)) << 28);

  _data[2] =
    ((trig_id&0x1f) << 26) |
    (1<<25) |
    (glort<<0);
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
  a.ether_addr_octet[0] = (_data[1]>> 8)&0xff;
  a.ether_addr_octet[1] = (_data[1]>> 0)&0xff;
  a.ether_addr_octet[2] = (_data[0]>>24)&0xff;
  a.ether_addr_octet[3] = (_data[0]>>16)&0xff;
  a.ether_addr_octet[4] = (_data[0]>> 8)&0xff;
  a.ether_addr_octet[5] = (_data[0]>> 0)&0xff;
  return a;
}

unsigned char SwmMacTableEntry::addr_octet(unsigned i) const
{
  switch(i) {
  case 0: return (_data[0]>> 0) & 0xff;
  case 1: return (_data[0]>> 8) & 0xff;
  case 2: return (_data[0]>>16) & 0xff;
  case 3: return (_data[0]>>24) & 0xff;
  case 4: return (_data[1]>> 0) & 0xff;
  case 5: return (_data[1]>> 8) & 0xff;
  default: return 0;
  }
}

bool SwmMacTableEntry::dst_is_glort() const
{
  return _data[2]&(1<<25);
}

FmPortMask SwmMacTableEntry::port_mask() const
{
  return FmPortMask(_data[2] >> 0);
}

unsigned SwmMacTableEntry::glort() const
{
  return (_data[2] & 0xffff);
}

unsigned SwmMacTableEntry::learning_group() const
{
  return (_data[1] >> 16) & 0xfff;
}

unsigned SwmMacTableEntry::trigger() const
{
  return (_data[2]>>26) & 0x3f;
}

unsigned SwmMacTableEntry::locked() const
{
  return (_data[1] & 0x30000000)==0x30000000;
}

unsigned SwmMacTableEntry::valid() const
{
  return (_data[1] & 0x30000000)!=0;
}

unsigned SwmMacTableEntry::parity() const
{
  return (_data[1]>>30) & 1;
}

void SwmMacTableEntry::invalidate()
{
  _data[2] &= ~0x30000000;
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
  _data[2] &= ~0x1ffffff;
  _data[2] |= unsigned(m);
}

void SwmMacTableEntry::set_trigger(unsigned m)
{
  _data[2] &= ~0xfc000000;
  _data[2] |= (m<<26);
}

void SwmMacTableEntry::set_lock(unsigned m)
{
  if (m)
    _data[1] |= 0x30000000;
  else {
    _data[1] &= ~0x30000000;
    _data[2] |=  0x20000000;
  }
}

