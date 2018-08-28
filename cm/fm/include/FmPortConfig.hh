#ifndef FmPortConfig_hh
#define FmPortConfig_hh

//
//  configuration masks of the 24 physical ports
//


#include "fm/FmPort.hh"

class FmPortConfig {
public:
  // port speed
  FmPortMask ports10Mb;
  FmPortMask ports100Mb;
  FmPortMask ports1Gb;
  FmPortMask ports10Gb;
  // lane reversal
  FmPortMask portsRxFlip;
  FmPortMask portsTxFlip;
};

#endif
