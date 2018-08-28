#ifndef FmPortMscCounters_hh
#define FmPortMscCounters_hh


#include "fm/FmMReg.hh"

class FmPortMscCounters {
private:
  unsigned reserved1[0x26];
public:
  FmMReg txPause;      // 0x80026+0x400*(N-1)
  FmMReg txFcsErrors;
private:
  unsigned reserved2;
public:
  FmMReg rxJabbers;    // 0x80029+0x400*(N-1)
private:
  unsigned reserved3[0x3d6];
};

#endif
