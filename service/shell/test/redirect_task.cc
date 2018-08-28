// -*-Mode: C;-*-
/**
@file
@brief An example of a task using shell_getTtyFd() and
shell_redirectStdio() to make printf() output go to the telnet socket
of the task that spawned it, e.g., a telnet shell task.

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
2014/02/25

@par Last commit:
\$Date: 2014-04-07 21:29:49 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3123 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/service/shell/test/redirect_task.cc $

@par Credits:
SLAC

*/
#include <stdio.h>

#include "system/statusCode.h"
#include "shell/redirect.h"
#include "task/Task.h"         


// The saved fd of the parent's telnet socket.
static int socketFd = -1;

/* The parent task calls this function via the dynamic linker, e.g.,
   lnk_load().
*/
extern "C" int lnk_prelude(void* prefs, void* elf) {
  socketFd = shell_getTtyFd();
  return STS_K_SUCCESS;
}


extern "C" void Task_Start(int argc, const char** argv)
{
  shell_redirectStdio(socketFd);
  printf("redirect_task was called with %d arguments @ %p: ",
         argc, argv);
  int i=0;
  while (i < argc) {
    printf("%s ", argv[i]);
    i++;
  }
  printf("\n");
  rtems_task_suspend(rtems_task_self());
}



extern "C" void Task_Rundown()
{
  printf("Ending redirect_task\n");
  return;
}
