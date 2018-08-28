/**
* @file      printStatus.cc
*
* @brief     Module for issuing the ioctl() command to print driver status.
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      September 14, 2013 -- Created
*
* $Revision: 3121 $
*
* @verbatim                    Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <rtems.h>


#include "sd/mmc.h"
#include "debug/print.hh"
#include "time/Time.hh"


namespace driver {

  namespace sd {

static void printStatus()
{
  const char* devName = "/dev/sdA2";

  int fd = open(devName, O_RDONLY, 0);
  if (fd < 0) {
    printf("%s: driver open of %s failed: %s\n", __func__,
           devName, strerror(errno));
    return;
  }

  if (ioctl(fd, RTEMS_MMC_IOCTL_PRINT_STATUS) < 0) {
    printf("%s: driver ioctl print status failed: %s\n", __func__,
           strerror(errno));
    return;
  }

  close(fd);
}


extern "C" void rce_appmain()
{
  uint64_t t0 = tool::time::lticks();

  printStatus();

  struct timespec t = tool::time::t2s(tool::time::lticks() - t0);

  tool::debug::printv("printStatus: Exiting; Run time: %u.%03u seconds\n",
                      t.tv_sec, t.tv_nsec / 1000000);
}

  } // sd

} // driver

