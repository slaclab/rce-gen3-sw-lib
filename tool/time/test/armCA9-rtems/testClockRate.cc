// -*-Mode: C++;-*-
/**
@file
@brief Print the values returned by time_getZynqClockRate().


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
2014/06/18

@par Last commit:
\$Date: 2015-07-14 10:55:53 -0700 (Tue, 14 Jul 2015) $ by \$Author: smaldona $.

@par Revision number:
\$Revision: 4078 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/time/test/armCA9-rtems/testClockRate.cc $

@par Credits:
SLAC

*/
#include <stdio.h>

#include "shell/redirect.h"
#include "system/statusCode.h"
#include "task/Task.h"

#include "time/cpu/clockRate.h"

// The saved fd of the parent's telnet socket.
static int socketFd = -1;

/* The parent task calls this function via the dynamic linker, e.g.,
   lnk_load().
*/
extern "C" int lnk_prelude(void* prefs, void* elf) {
  socketFd = shell_getTtyFd();
  return STS_K_SUCCESS;
}

extern "C" void Task_Start(int argc, const char** argv) {
  shell_redirectStdio(socketFd);

  printf("\nZynq clock rates in Hz.\n");
  printf("CPU_6x4x: %10u\n", time_getZynqClockRate(ZYNQ_CLOCK_CPU_6X4X));
  printf("CPU_3x2x: %10u\n", time_getZynqClockRate(ZYNQ_CLOCK_CPU_3X2X));
  printf("CPU_2x:   %10u\n", time_getZynqClockRate(ZYNQ_CLOCK_CPU_2X));
  printf("CPU_1X:   %10u\n", time_getZynqClockRate(ZYNQ_CLOCK_CPU_1X_REF));
  printf("UART ref: %10u\n", time_getZynqClockRate(ZYNQ_CLOCK_UART_REF));
  printf("Done.\n");
}


extern "C" void Task_Rundown() {}
