#include "fm/SwmPortVlanConfig.hh"

#include <string.h>

SwmPortVlanConfig::SwmPortVlanConfig(Option) :
  strict      (0),
  tagged      (0),
  untagged    (0)
{
  for(unsigned i=0; i<FmPort::MAX_PORT; i++)
    defaultVlan[i] = 1;
}
