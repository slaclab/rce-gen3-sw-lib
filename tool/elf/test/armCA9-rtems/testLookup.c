// -*-Mode: C;-*-
/**
@file
@brief Short test of lnk_lookup().

@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
DAT

@author
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2014/08/01

@par Last commit:
\$Date: 2014-08-01 18:11:40 -0700 (Fri, 01 Aug 2014) $ by \$Author: tether $.

@par Revision number:
\$Revision: 3601 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/elf/test/armCA9-rtems/testLookup.c $

@par Credits:
SLAC

*/
#include <stdio.h>

#include "elf/linker.h"
#include "system/statusCode.h"
#include "shell/redirect.h"
#include "task/Task.h"         


// The saved fd of the parent's telnet socket.
static int socketFd = -1;


// The saved address of this loaded ELF image.
static void* self;

int lnk_prelude(void* prefs, void* elf) {
  self = elf;
  socketFd = shell_getTtyFd();
  return STS_K_SUCCESS;
}


void Task_Start(int argc, const char** argv) {
  shell_redirectStdio(socketFd);

  void* const lookedup = lnk_lookup(self, "Task_Start");
  printf("Values of Task_Start() (this function). They should be equal.\n");
  printf("    From lnk_lookup(): %p\n", lookedup);
  printf("    From &Task_Start:  %p\n", &Task_Start);
}



void Task_Rundown() {}
