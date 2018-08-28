#ifndef SwmPortVlanConfig_hh
#define SwmPortVlanConfig_hh

#include "fm/FmPort.hh"

class SwmPortVlanConfig {
public:
  enum Option {Init};
  SwmPortVlanConfig() {}
  SwmPortVlanConfig(Option);
  SwmPortVlanConfig(const SwmPortVlanConfig&);

  FmPortMask strict;        // require port is member (on ingress)
  FmPortMask tagged;        // require tagged
  FmPortMask untagged;      // require untagged

  unsigned    defaultVlan[FmPort::MAX_PORT];  // apply to untagged
};

#endif
