
#include "bpdu/Parameters.hh"

#include <string.h>
#include <stdio.h>

using namespace cm::bpdu;


static TimeParameters _default = { 2<<8, 20<<8, 15<<8, 0 };

const TimeParameters& TimeParameters::internal()
{
  return _default;
}

TimeParameters& TimeParameters::operator=(const TimeParameters& p)
{
  bridgeHelloTime = p.bridgeHelloTime;
  bridgeMaxAge    = p.bridgeMaxAge;
  bridgeFwdDelay  = p.bridgeFwdDelay;
  messageAge      = p.messageAge;
  return *this;
}

bool TimeParameters::operator==(const TimeParameters& p) const
{
  return memcmp(this,&p,sizeof(*this))==0;
}

bool TimeParameters::operator!=(const TimeParameters& p) const
{
  return memcmp(this,&p,sizeof(*this));
}

void TimeParameters::dump() const
{
  printf("hello %x  maxage %x  fwddelay %x  msgage %x\n",
	 bridgeHelloTime, bridgeMaxAge, bridgeFwdDelay, messageAge);
}
