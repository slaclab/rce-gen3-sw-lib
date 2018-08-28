
#include "fm/SwmPacket.hh"

#include "fm/FmTahoe.hh"

SwmPacket::SwmPacket( unsigned d ) : _dev(0), _len(0), _dst(d) {}

SwmPacket::SwmPacket( FmTahoe& t ) : _dev(&t), _len(0), _dst(0)
{
#ifdef ppc405
  // first word is extra_info
  // last word is frame_status
  // seems to be an extra word before frame_status
  unsigned l = t.rxPacket(&_dst);
  _len = (l>3) ? l - 3 : 0;
#endif
}

SwmPacket::~SwmPacket()
{
}

bool SwmPacket::operator==(const SwmPacket& p) const
{
  if (p._len != _len) return false;
  for(unsigned j=0; j<_len; j++)
    if (p._data[j] != _data[j]) return false;
  return true;
}

void SwmPacket::append(unsigned v)
{
  _data[_len++] = v;
}
