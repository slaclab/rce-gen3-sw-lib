
#include "bpdu/BridgeId.hh"

#include <string.h>
#include <stdio.h>

using namespace cm::bpdu;

BridgeId::BridgeId() {}

BridgeId::BridgeId(ether_addr& hwaddr, unsigned priority, unsigned localid)
{
  _value[0] = ((priority&0xf)<<4) | (localid>>8);
  _value[1] = localid&0xff;
  memcpy(&_value[2],&hwaddr,sizeof(hwaddr));
}

BridgeId::BridgeId(const BridgeId& b)
{
  memcpy(this,&b,sizeof(*this));
}

bool BridgeId::operator==(const BridgeId& b) const
{
  return memcmp(this,&b,sizeof(*this))==0;
}

bool BridgeId::operator!=(const BridgeId& b) const
{
  return memcmp(this,&b,sizeof(*this))!=0;
}

bool BridgeId::operator<(const BridgeId& b) const
{
  return memcmp(this,&b,sizeof(*this))<0;
}

BridgeId& BridgeId::operator=(const BridgeId& b)
{
  memcpy(this,&b,sizeof(*this));
  return *this;
}

ether_addr BridgeId::hwaddr() const 
{ 
  ether_addr t;
  memcpy(&t,&_value[2],sizeof(t));
  return t;
}

void BridgeId::dump(char* o) const
{
  for(unsigned i=0; i<8; i++)
    o += sprintf(o,"%02x%c",_value[i],i==7?'\0':':');
}
