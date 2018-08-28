/**
* @file      fsRate.cc
*
* @brief     Module for collecting data on the SD performance.
*
*            Note that this test will destroy data on the SD card
*            when used in write mode.
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      September 4, 2013 -- Created
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


#include "sd/mmc.h"
#include "sd/sdhci.h"
#include "debug/print.hh"
#include "time/Time.hh"
#include "chart/chart.h"
#include "histogram/Histogram.hh"

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

void acquire(unsigned n)
{
  const unsigned blockSize   = 512;
  const char*    devName     = "/dev/sdA2";

  // Allocate multiple blocks of memory
  uint8_t* buffer = new uint8_t[n * blockSize];
  if (buffer == 0) {
    printf("%s: No memory for %u blocks of buffer space\n", __func__, n);
    return;
  }

  // Make a chart
  chart_t* chart = TOOL_CHART_create((uint32_t)n);
  if (chart == 0) {
    printf("%s: No memory for chart of %u points\n", __func__, n);
    delete [] buffer;
    return;
  }

  printf("%s: chart = 0x%08x, base = 0x%08x\n", __func__,
         (uint)chart, (uint)TOOL_CHART_base(chart));

#if 0                 // By default, this partition is mounted at boot time now
  const char     mount_dir[] = "/mnt/sd";
  int            rv;

  rv = mkdir( mount_dir, S_IRWXU | S_IRWXG | S_IRWXO );
  rtems_test_assert ( ((0 == rv) || (EEXIST == errno)), return );

  // This mount is necessary to initialize libfs internals, for some reason
  rv = mount( devName,
              mount_dir,
              RTEMS_FILESYSTEM_TYPE_DOSFS,
              RTEMS_FILESYSTEM_READ_WRITE,
              NULL );
  rtems_test_assert( 0 == rv, return );

  // Since we don't need the partition to be mounted to do the test, unmount it
  rv = unmount( mount_dir );
  rtems_test_assert( 0 == rv, return );
#endif

  FILE* file = fopen(devName, "rb");
  if (file == 0) {
    printf("%s: fopen of %s failed: %s\n", __func__, devName, strerror(errno));
    delete [] buffer;
    return;
  }

  const unsigned startBlk  = 100000;
  fseek(file, startBlk * blockSize, SEEK_SET);

  // Loop over different sized reads
  uint32_t i;
  for (i = 1; i <= n; i <<= 1)
  {
    // Do several trials
    uint32_t trials = 5;
    uint64_t tSum   = 0;
    uint32_t cnt;
    uint32_t j;
    for (j = 0; j < trials; ++j)
    {
      // Time it
      uint64_t t0 = tool::time::lticks();

      // Change this to read or write blocks, as desired
      cnt = fread(buffer, blockSize, i, file);

      tSum += tool::time::lticks() - t0;

      //printf("%s: cnt = %lu, dT = %llu\n", __func__, cnt, tool::time::lticks() - t0);

      if (cnt != i) {
        printf("%s: Read %lu blocks instead of expected %lu\n", __func__, cnt, i);
        break;
      }
    }

    double uS   = double(tSum * 2 * CPU_CLOCK_FREQ_DEN) / double(trials * CPU_CLOCK_FREQ_NUM);
    double MBpS = double(cnt * blockSize) / double(uS);

    printf("%s: blocks = %5lu, time = 0x%08llx ticks = %11.3f usecs, rate = %6.3f MB/s\n",
           __func__, cnt, tSum / trials, uS, MBpS);
    fflush(stdout);
    rtems_task_wake_after(RTEMS_YIELD_PROCESSOR);

    TOOL_CHART_mark_XY(chart, cnt, int(1000. * MBpS + 0.5));
  }

  fclose(file);

  // Save the data into a file in the root directory.
  // Copy this file to an NFS directory to make it accessible for plotting
  TOOL_CHART_save(chart, "/fsRate.txt");

  // Free memory
  TOOL_CHART_delete(chart);
  delete [] buffer;
}


extern "C" void rce_appmain()
{
  uint64_t t0 = tool::time::lticks();

  // Argument is the maximum number of blocks to test with.
  // The acquire function loops by powers of 2 up to this number.
  acquire(1 << 16);

  struct timespec t = tool::time::t2s(tool::time::lticks() - t0);

  tool::debug::printv("fsRate: Exiting; Run time: %u.%03u seconds\n",
                      t.tv_sec, t.tv_nsec / 1000000);
}

    } // test

  } // sd

} // driver
