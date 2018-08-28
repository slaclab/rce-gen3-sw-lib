#ifndef Bali_FmPortMibCounters_hh
#define Bali_FmPortMibCounters_hh

#include "fmb/FmDefs.hh"
#include "fm/FmMibCounter.hh"

#include <stdint.h>

namespace Bali {
class FmPortMibCounters {
public:
#define ADD_RSRV(group,len)                     \
  FmSReg reserved_##group[len]
#define ADD_STAT(group,member0,member1)         \
  struct {                                      \
    FmMibCounter member0;                       \
    FmMibCounter member1;                       \
  } Stat##group;                                \
    FmSReg reserved_Stat##group[108]

  ADD_STAT_PORTCOUNTERS

#undef ADD_STAT
#undef ADD_RSRV
};
};

#endif
