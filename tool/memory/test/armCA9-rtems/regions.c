// -*-Mode: C;-*-
/**
@file Print the table of Regions.
@brief 
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
2014/06/12

@par Last commit:
\$Date: 2014-06-13 13:06:08 -0700 (Fri, 13 Jun 2014) $ by \$Author: tether $.

@par Revision number:
\$Revision: 3386 $

@par Credits:
SLAC
*/
#include <stdarg.h>
#include <stdio.h>

#include "memory/mem.h"
#include "system/statusCode.h"
#include "shell/redirect.h"
#include "task/Task.h"         



// The saved fd of the parent's telnet socket.
static int socketFd = -1;

/* The parent task calls this function via the dynamic linker, e.g.,
   lnk_load().
*/
int lnk_prelude(void* prefs, void* elf) {
  socketFd = shell_getTtyFd();
  return STS_K_SUCCESS;
}


static void print(const char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  vprintf(fmt, va);
  va_end(va);
}

void Task_Start(int argc, const char** argv) {
  mem_Region_printConfig(print);
}


void Task_Rundown() {}
