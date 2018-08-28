// -*-Mode: C++;-*-
/*!
*
* @brief   Functions for handling times
*
* @author  Anonymous -- REG/DRD - (someone@slac.stanford.edu)
*
* @date    March 29, 2011 -- Created
*
* $Revision: 3178 $
*
* @verbatim
*                               Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef TOOL_TIME_TIME_HH
#define TOOL_TIME_TIME_HH

namespace tool {
  namespace time {
    // Wall clock time
    unsigned long long time(); // returns time in nano seconds
    void time(unsigned& sec, unsigned& nsec);
    void settime(unsigned sec, unsigned nsec);

    // ASCII time
    const char* asctime(unsigned long long t, char* buf);
    const char* asctime(unsigned sec, unsigned nsec, char* buf);
  } // time
} // tool

#include "time/platform/TimeImpl.hh"

#endif // TOOL_TIME_TIME_HH
