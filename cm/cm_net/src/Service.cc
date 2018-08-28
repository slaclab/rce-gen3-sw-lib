
#include "cm_net/Service.hh"

using namespace cm::net;

//
//  Assumes that len is even number of bytes and < 2**16 and d is aligned on a 2B boundary
//
int Service::in_cksum(void*    d,
		      unsigned len)
{
  unsigned short* w   = reinterpret_cast<unsigned short*>(d);
  unsigned short* end = w + (len>>1);
  unsigned sum = 0;

  while( w < end ) {
    sum += *w++;
  }
  unsigned sum_u;
  while( (sum_u = sum>>16) )
    sum = (sum&0xffff) + sum_u;

 return (~sum & 0xffff);
}
