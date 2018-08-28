// -*-Mode: C++;-*-
/**
@file
@brief Main program for the codeTiming application.
@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
DAT

@author
Steve Tether <tether@slac.stanford.edu>

@par Date created:
2013/10/02

@par Last commit:
\$Date: 2014-06-25 14:01:35 -0700 (Wed, 25 Jun 2014) $ by \$Author: tether $.

@par Revision number:
\$Revision: 3419 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/time/test/armCA9-rtems/codeTiming.cc $

@par Credits:
SLAC
*/

#include <rtems.h>

#include "debug/print.h"
#include "shell/redirect.h"
#include "system/statusCode.h"
#include "task/Task.h"
#include "time/platform/time.h"

// The saved fd of the parent's telnet socket.
static int socketFd = -1;

/* The parent task calls this function via the dynamic linker, e.g.,
   lnk_load().
*/
extern "C" int lnk_prelude(void* prefs, void* elf) {
  socketFd = shell_getTtyFd();
  return STS_K_SUCCESS;
}

#include "logger/Logger.hh"
using service::logger::Logger;

#include "concurrency/Thread.hh"
using tool::concurrency::Thread;

void memrefTimes();
void callTimes();
void yieldTimes();
void memopTimes();


namespace {

  class TaskThread: public Thread {
  public:
    TaskThread() : Thread(false) {}
    virtual ~TaskThread() {}
    void body() {
      Logger().info("Each test is run twice to try to ensure that the code is cached");
      Logger().info("the second time around. For most tests only the second run's");
      Logger().info("times are printed.");
      Logger().info("");
      Logger().info("One billion ticks are about equal to %u micoseconds.", TOOL_TIME_t2uS(1000000000U));
      Logger().info("Also, on the Zynq one tick is two CPU clock cycles.");
      memrefTimes();
      callTimes();
      yieldTimes();
      memopTimes();
      Logger().info("");
      Logger().info("End of code timing tests.");
    }
  };

}


extern "C" void Task_Start(int argc, const char** argv) {
  shell_redirectStdio(socketFd);
  TaskThread thr;
  thr.graduate();
}

extern "C" void Task_Rundown() {}
