// -*-Mode: C;-*-
/**
@file
@brief See if LDREX and STREX work on OCM mapped with /dev/ocm.

This code must be compiled for Cortex-A9 so that the compile builtin
__sync_loc_test_and_set() uses the LDREX and STREX instructions
instead of the older, deprecated SWP instruction.

The ocmdev kernel module must be loaded.

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
yyyy/mm/dd

@par Last commit:
\$Date: 2014-09-11 16:51:16 -0700 (Thu, 11 Sep 2014) $ by \$Author: tether $.

@par Credits:
SLAC
*/
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define OCM_SIZE     65536


int main(void) {
  int const fd1 = open("/dev/ocm", O_RDWR);
  if (fd1 < 0) {
    perror("open() of dev/ocm");
    return 1;
  }
  unsigned* const ocm = (unsigned*)mmap(NULL, OCM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd1, 0);
  if (ocm == (unsigned*)MAP_FAILED) {
    perror("mmap() of /dev/ocm");
    return 1;
  }

  printf("About to call __sync_lock_test_and_set() on the first word of the OCM.\n"
         "It should return an old lock value of zero and set a new lock value of one.\n"
         "But if the memory attributes are wrong, as they were when mapping\n"
         "/dev/mem, then __sync_lock_test_and_set() will loop endlessly\n"
           "because STREX will always report a failure to store the new value.\n\n");

  *ocm = 0;
  unsigned const oldval = __sync_lock_test_and_set(ocm, 1);
  printf("Old lock value %u, new value %u.\n", oldval, *ocm);
  return 0;
}
