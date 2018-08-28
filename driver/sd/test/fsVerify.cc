/**
* @file      fsVerify.cc
*
* @brief     Module for verifying the contents of a test file written by fsWrite.
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      October 28, 2013 -- Created
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

#include <stdio.h>
#include <errno.h>
#include <rtems.h>
#include <rtems/fsmount.h>
#include <rtems/dosfs.h>
#include <rtems/bdpart.h>
#include <rtems/error.h>
#include <rtems/status-checks.h>


#include "debug/print.hh"
#include "time/Time.hh"

#define rtems_test_exit(__exp)  __exp
#define rtems_test_assert(__exp1, __exp2)                         \
  do {                                                            \
    if (!(__exp1)) {                                              \
      printf( "%s: %d %s\n", __FILE__, __LINE__, #__exp1 );       \
      rtems_test_exit(__exp2);                                    \
    }                                                             \
  } while (0)


namespace driver {

  namespace sd {

    namespace test {

void fsVerify()
{
  const unsigned blockSize   = 512;
  const char     devName[]   = "/dev/sdA2";
  const char     mount_dir[] = "/mnt/sd";
  const char     fileSpec[]  = "/mnt/sd/data.bin";
  int            rv;

  // Allocate multiple blocks of memory
  const uint32_t bufLen  = (64 * blockSize) / sizeof(uint32_t);
  uint32_t*      buffer = new uint32_t[bufLen];
  if (buffer == 0) {
    printk("%s: No memory for %u blocks of buffer space\n", __func__, bufLen);
    return;
  }

  rv = mkdir( mount_dir, S_IRWXU | S_IRWXG | S_IRWXO );
  rtems_test_assert ( ((0 == rv) || (EEXIST == errno)), return );

  // This mount is necessary to initialize libfs internals, for some reason
  rv = mount( devName,
              mount_dir,
              RTEMS_FILESYSTEM_TYPE_DOSFS,
              RTEMS_FILESYSTEM_READ_WRITE,
              NULL );
  rtems_test_assert( 0 == rv, return );

  FILE* file = fopen(fileSpec, "rb");
  if (file == 0) {
    printk("%s: fopen of %s failed: %s\n", __func__, fileSpec, strerror(errno));
    delete [] buffer;
    return;
  }

  uint32_t i;
  uint32_t n = 0;
  unsigned cntDown = 5;
  for (i = 0; i < 1024; ++i) {
    size_t cnt = fread(buffer, sizeof(*buffer), bufLen, file);
    if (cnt != bufLen) {
      printk("%s: fread failed at %lu: %s\n", __func__, i, strerror(errno));
      break;
    }

    uint32_t j;
    for (j = 0; j < bufLen; ++j) {
      if (buffer[j] == ++n) {
        cntDown = 5;
      } else {
        if (i == 0 && j == 0) {
          n = buffer[0];
          printk("%s: Pattern starts from %08lx\n", __func__, n);
        } else {
          if (cntDown) {
            printk("%s: Pattern match failed at (i,j) = (%lu,%lu): %08lx ^ %08lx = %08lx\n",
                   __func__, i, j, buffer[j], n, buffer[j] ^ n);
            --cntDown;
          }
        }
      }
    }
  }

  fclose(file);

  rv = unmount( mount_dir );
  rtems_test_assert( 0 == rv, return );

  // Free memory
  delete [] buffer;
}


extern "C" void rce_appmain()
{
  uint64_t t0 = tool::time::lticks();

  fsVerify();

  struct timespec t = tool::time::t2s(tool::time::lticks() - t0);

  tool::debug::printv("fsVerify: Exiting; Run time: %u.%03u seconds\n",
                      t.tv_sec, t.tv_nsec / 1000000);
}

    } // test

  } // sd

} // driver
