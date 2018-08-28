// -*-Mode: C;-*-
/**
@file Print the attributes of all of memory using the mem_getFlagsXxx() functions.
@brief 
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
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2013/06/24

@par Last commit:
\$Date: 2014-06-13 13:06:08 -0700 (Fri, 13 Jun 2014) $ by \$Author: tether $.

@par Revision number:
\$Revision: 3386 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/memory/test/armCA9-rtems/memscan.c $

@par Credits:
SLAC
*/
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


void Task_Start(int argc, const char** argv) {
  printf("\nScan memory, printing the memory attribute flags when they change.\n");
  printf("For virtual address in [0 GB, 4 GB) stepping by 4 KB.\n");
  printf("r=readable  w=writable  c=cached  d=device  h=handshake  s=shared\n");
  printf("\nV-address  flags\n");
  printf(  "---------- ----------\n");
  const unsigned kb4 = 0x1000;
  int prevFlags = -1;
  unsigned  i;
  for (i = 0; i < 0x100000U; ++i) {
    int flags = mem_getFlagsWrite(i * kb4);
    if (flags == 0) {
      /* Write probe failed, try a read probe. */
      flags = mem_getFlagsRead(i * kb4);
    }
    if (flags != prevFlags) {
      /* Memory attributes have changed. */
      prevFlags = flags;
      printf("0x%08x ", i * kb4);
      printf("%s", (flags & MEM_READ)      ? "r" : " ");
      printf("%s", (flags & MEM_WRITE)     ? "w" : " ");
      printf("%s", (flags & MEM_CACHED)    ? "c" : " ");
      printf("%s", (flags & MEM_DEVICE)    ? "d" : " ");
      printf("%s", (flags & MEM_HANDSHAKE) ? "h" : " ");
      printf("%s", (flags & MEM_SHARED)    ? "s" : " ");
      printf("\n");
    }
  }
}


void Task_Rundown() {}
