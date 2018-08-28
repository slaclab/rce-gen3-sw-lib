
#include "cm_svc/ChannelMap.hh"

using namespace cm::svc;

ChannelMap::ChannelMap(unsigned rangeA,
		       unsigned rangeB) :
  _a (new unsigned[rangeB]),
  _b (new unsigned[rangeA])
{
  for(unsigned i=0; i<rangeB; i++)
    _a[i] = -1;;
  for(unsigned i=0; i<rangeA; i++)
    _b[i] = -1;
}

ChannelMap::~ChannelMap()
{
  delete[] _a;
  delete[] _b;
}

void ChannelMap::add(unsigned aChan,
		     unsigned bChan)
{
  _a[bChan] = aChan;
  _b[aChan] = bChan;
}

unsigned ChannelMap::aMask(unsigned bMask) const
{
  unsigned v = 0;
#if defined(ppc405)||defined(ppc440)
  unsigned t = 32;
  while( bMask ) {
    unsigned tt;
    asm volatile( "cntlzw %0, %1" : "=r"(tt) : "r"(bMask) );
    bMask <<= (tt+1);
    v |= (1<<_a[t-=(tt+1)]);
  }
#else
  for(unsigned i=0; i<32; i++) {
    if (bMask&(1<<i)) 
      v |= (1<<_a[i]);
  }
#endif
  return v;
}

unsigned ChannelMap::bMask(unsigned aMask) const
{
  unsigned v = 0;
#if defined(ppc405)||defined(ppc440)
  unsigned t = 32;
  while( aMask ) {
    unsigned tt;
    asm volatile( "cntlzw %0, %1" : "=r"(tt) : "r"(aMask) );
    aMask <<= (tt+1);
    v |= (1<<_b[t-=(tt+1)]);
  }
#else
  for(unsigned i=0; i<32; i++) {
    if (aMask&(1<<i)) 
      v |= (1<<_b[i]);
  }
#endif
  return v;
}
