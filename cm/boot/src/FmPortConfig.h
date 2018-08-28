#ifndef FmPortConfig_h
#define FmPortConfig_h



#include "FmPort.h"

//
//  configuration masks of the 24 physical ports
//

typedef uint32_t FmPortMask;

typedef struct {
  // port speed
  FmPortMask ports10Mb;
  FmPortMask ports100Mb;
  FmPortMask ports1Gb;
  FmPortMask ports2_5Gb;
  FmPortMask ports4Gb;
  FmPortMask ports10Gb;
  // lane reversal
  FmPortMask portsRxFlip;
  FmPortMask portsTxFlip;
  // features
  FmPortMask portsLoopback;
}  FmPortConfig;

#endif
