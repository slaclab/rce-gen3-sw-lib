// -*-Mode: C++;-*-
/*!
*
* @brief   Functions for handling times
*
* @author  Anonymous -- REG/DRD - (someone@slac.stanford.edu)
*
* @date    March 29, 2011 -- Created
*
* $Revision: 25 $
*
* @verbatim
*                               Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef TOOL_TIME_TIME_IMPL_HH
#define TOOL_TIME_TIME_IMPL_HH

#include "time/platform/time.h"


namespace tool {
  namespace time {
    // CPU clock ticks
    unsigned ticks(); // return time registers least significant word
    void ticks(unsigned& up, unsigned& low);
    unsigned long long lticks();
    struct timespec t2s(unsigned long long dT);
  }; // time
}; // tool

inline unsigned tool::time::ticks()
{
  return TOOL_TIME_ticks();
}

inline void tool::time::ticks(unsigned& up, unsigned& low)
{
  return TOOL_TIME_ticks2(&up, &low);
}

inline unsigned long long tool::time::lticks()
{
  return TOOL_TIME_lticks();
}

inline struct timespec tool::time::t2s(unsigned long long dT)
{
  return TOOL_TIME_t2s(dT);
}


#endif // TOOL_TIME_TIME_IMPL_HH
