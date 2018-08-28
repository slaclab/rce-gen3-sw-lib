/**
* @file      rawRate.cc
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


namespace driver {

  namespace sd {

    namespace test {

void acquire(unsigned n)
{
  const unsigned blockSize = 512;

  // Allocate multiple blocks of memory
  uint8_t* buffer = new uint8_t[n * blockSize];
  if (buffer == 0) {
    printf("%s: No memory for %u blocks of buffer space\n", __func__, n);
    return;
  }

  // Find struct mmc*
  dev_t              dev = rtems_filesystem_make_dev_t(3, 0);
  rtems_disk_device* dd  = rtems_disk_obtain(dev);
  struct mmc*        mmc = (struct mmc*)rtems_disk_get_driver_data(dd);
  if (mmc == 0) {
    printf("%s: MMC structure wasn't found\n", __func__);
    delete [] buffer;
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

  unsigned startBlk = 100000;
  uint32_t cnt      = 1;
  uint32_t i;
  for (i = cnt; i <= n; i <<= 1)
  {
    uint32_t nBlk = i < (1 << 16) ? i : (i - 1); // 64 K - 1 blocks is max
    // Do several trials
    uint32_t trials = 5;
    uint64_t tSum   = 0;
    uint32_t j;
    for (j = 0; j < trials; ++j)
    {
#if 0                                   // For when writing, but seems broken
      if (mmc_erase_blocks(mmc, startBlk, i) != 0) {
        printf("%s: Erase of %lu blocks failed\n", __func__, i);
        break;
      }
#endif

      // Time it
      uint64_t t0 = tool::time::lticks();

      // Change this to read or write blocks, as desired
      cnt = mmc_read_blocks(mmc, startBlk, nBlk, buffer);

      tSum += tool::time::lticks() - t0;

      //printf("%s: cnt = %lu, dT = %llu\n", __func__, cnt, tool::time::lticks() - t0);

      if (cnt != nBlk) {
        printf("%s: Read %lu blocks instead of expected %lu\n", __func__, cnt, nBlk);
        break;
      }

      startBlk += nBlk;
    }

    double uS   = double(tSum * 2 * CPU_CLOCK_FREQ_DEN) / double(trials * CPU_CLOCK_FREQ_NUM);
    double MBpS = double(nBlk * blockSize) / double(uS);

    printf("%s: blocks = %5lu, time = 0x%08llx ticks = %11.3f usecs, rate = %6.3f MB/s\n",
           __func__, nBlk, tSum / trials, uS, MBpS);
    fflush(stdout);
    rtems_task_wake_after(RTEMS_YIELD_PROCESSOR);

    TOOL_CHART_mark_XY(chart, i, int(1000. * MBpS + 0.5));
  }

  // Save the data into a file in the root directory.
  // Copy this file to an NFS directory to make it accessible for plotting
  TOOL_CHART_save(chart, "/rawRate.txt");

  rtems_disk_release(dd);

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

  tool::debug::printv("rawRate: Exiting; Run time: %u.%03u seconds\n",
                      t.tv_sec, t.tv_nsec / 1000000);
}


    } // test

  } // sd

} // driver

