// -*-Mode: C++;-*-
/**
@file
@brief Use counter 0 of a Triple Timer Counter module to count off ten seconds.


@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
RTS

@author
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2014/06/09

@par Last commit:
\$Date: 2014-06-25 14:01:35 -0700 (Wed, 25 Jun 2014) $ by \$Author: tether $.

@par Revision number:
\$Revision: 3419 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/time/test/armCA9-rtems/testTtc.cc $

@par Credits:
SLAC

*/
#include <stdio.h>

#include <rtems.h>

#include "logger/Logger.hh"
using service::logger::Logger;

#include "shell/redirect.h"
#include "system/statusCode.h"
#include "task/Task.h"

#include "time/cpu/tripleTimer.h"

// The saved fd of the parent's telnet socket.
static int socketFd = -1;

/* The parent task calls this function via the dynamic linker, e.g.,
   lnk_load().
*/
extern "C" int lnk_prelude(void* prefs, void* elf) {
  socketFd = shell_getTtyFd();
  return STS_K_SUCCESS;
}


const int secondsToWait = 10;

extern "C" void Task_Start(int argc, const char** argv) {
  shell_redirectStdio(socketFd);

  // Figure out the prescaling and counter value needed to get a delay
  // of one second, or as close as we can manage.
  unsigned const rate = time_getTtcInternalClockRate();
  // Want count << prescale == rate and count <= 2^16 - 1.
  int prescale = 0;
  unsigned maxcount = rate;
  while (maxcount > 65535) { // 16-bit counter.
    if (++prescale > 16) {   // 4-bit prescale. ">" is not a mistake, see tripleHeader.h.
      Logger().info("Prescale count out of range!");
      return;
    }
    maxcount >>= 1;
  }
  Logger().info("TTC rate %u Hz. Prescale by 2^%d.", rate, prescale);
  Logger().info("Count to %u each second for %d seconds.", maxcount, secondsToWait);

  time_initTtc(prescale, TTC_CLOCK_INTERNAL);
  Logger().info("Starting counter.");
  time_startTtc(TTC_DONT_RESET);

  int countdown = secondsToWait;
  unsigned oldCount = 0;
  while (countdown > 0) {
    unsigned newCount = time_getTtcCounter(TTC_DONT_STOP);
    if ((newCount >= maxcount) || (newCount < oldCount)) {
      time_stopTtc();
      time_startTtc(TTC_RESET);
      oldCount = 0;
      --countdown;
    }
    else {
      oldCount = newCount;
    }
  }
  time_stopTtc();
  Logger().info("Stopping counter.");
}



extern "C" void Task_Rundown() {
  return;
}
