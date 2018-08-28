#ifndef FmPortMscCounters_h
#define FmPortMscCounters_h



#include "FmMReg.h"

typedef struct {
  unsigned reserved1[0x26];
  FmMReg txPause;      // 0x80026+0x400*(N-1)
  FmMReg txFcsErrors;
  unsigned reserved2;
  FmMReg rxJabbers;    // 0x80029+0x400*(N-1)
  unsigned reserved3[0x3d6];
} FmPortMscCounters;

#endif
