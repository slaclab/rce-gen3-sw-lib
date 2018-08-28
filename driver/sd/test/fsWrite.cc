/**
* @file      fsWrite.cc
*
* @brief     Module for writing a test file composed of a pattern.
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

void fsWrite()
{
  const unsigned blockSize   = 512;
  const char     devName[]   = "/dev/sdA2";
  const char     mount_dir[] = "/mnt/sd";
  const char     fileSpec[]  = "/mnt/sd/data.bin";
  int            rv;

  const uint32_t  bufLen = (64 * blockSize) / sizeof(uint32_t);
  uint32_t*       buffer = new uint32_t[bufLen];
  if (buffer == 0) {
    printk("%s: No memory for %u blocks of buffer space\n", __func__, bufLen);
    return;
  }

  rv = mkdir( mount_dir, S_IRWXU | S_IRWXG | S_IRWXO );
  rtems_test_assert ( ((0 == rv) || (EEXIST == errno)), return );

  rv = mount( devName,
              mount_dir,
              RTEMS_FILESYSTEM_TYPE_DOSFS,
              RTEMS_FILESYSTEM_READ_WRITE,
              NULL );
  rtems_test_assert( 0 == rv, return );

  FILE* file = fopen(fileSpec, "wb");
  if (file == 0) {
    printk("%s: fopen of %s failed: %s\n", __func__, fileSpec, strerror(errno));
    delete [] buffer;
    return;
  }

  uint32_t i;
  uint32_t n = tool::time::lticks();
  printk("%s: Pattern starts from %08lx\n", __func__, ++n);
  for (i = 0; i < 1024; ++i) {
    uint32_t j;
    for (j = 0; j < bufLen; ++j) {
      buffer[j] = n++;
    }

    size_t cnt = fwrite(buffer, sizeof(*buffer), bufLen, file);
    if (cnt != bufLen) {
      printk("%s: fwrite failed at %lu: %s\n", __func__, i, strerror(errno));
      break;
    }
  }

  fclose(file);

  rv = unmount( mount_dir );
  rtems_test_assert( 0 == rv, return );

  delete [] buffer;
}


extern "C" void rce_appmain()
{
  uint64_t t0 = tool::time::lticks();

  fsWrite();

  struct timespec t = tool::time::t2s(tool::time::lticks() - t0);

  tool::debug::printv("fsWrite: Exiting; Run time: %u.%03u seconds\n",
                      t.tv_sec, t.tv_nsec / 1000000);
}

    } // test

  } // sd

} // driver
