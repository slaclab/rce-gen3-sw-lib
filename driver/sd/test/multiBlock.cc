#include <stdio.h>
#include <errno.h>
#include <rtems.h>
#include <rtems/fsmount.h>
#include <rtems/dosfs.h>
#include <rtems/bdpart.h>
#include <rtems/error.h>
#include <rtems/status-checks.h>


#include "system/fs.hh"
#include "sd/mmc.h"
#include "sd/sdhci.h"
#include "time/Time.hh"
#include "chart/chart.h"
#include "histogram/Histogram.hh"


extern "C" chart_t* mmc_chart;

namespace driver {

  namespace sd {

    namespace test {

typedef struct block {
  uint8_t byte[512];
} block_t;


void acquire(unsigned n)
{
  using namespace configuration::system;

  int               rv          = 0;
  size_t            abort_index = 0;

  // Mount
  rv = rtems_fsmount(fs_table, sizeof(fs_table)/sizeof(*fs_table), &abort_index);
  if (rv != 0) {
    printf("%s: Mount failed: %s\n", __func__, strerror(errno));
    return;
  }

  // Allocate multiple blocks of memory
  block_t* buffer = new block_t[n];

  // Find struct mmc*
  dev_t              dev = rtems_filesystem_make_dev_t(3, 0);
  rtems_disk_device* dd  = rtems_disk_obtain(dev);
  struct mmc*        mmc = (struct mmc*)rtems_disk_get_driver_data(dd);

  // Time it
  uint64_t t0 = tool::time::lticks();

  // Read blocks
  uint32_t cnt  = mmc_read_blocks(mmc, 4000, n, buffer->byte);
  if (cnt != n)
    printf("%s: Read %lu blocks instead of expected %u\n", __func__, cnt, n);

  uint64_t dT   = tool::time::lticks() - t0;
  double   uS   = double(dT) * double(2 * CPU_CLOCK_FREQ_DEN) / double(CPU_CLOCK_FREQ_NUM);
  double   MBpS = double(n * sizeof(*buffer)) / double(uS);

  printf("\n%s: time = 0x%llx ticks = %f usecs, rate = %f MB/s\n\n",
         __func__, dT, uS, MBpS);

  rtems_disk_release(dd);

  // Free memory
  delete [] buffer;

  // Unmount
  unmount("/mnt/sd");
}


void analyze()
{
  using namespace tool::histogram;

  Histogram* histo = new Histogram("Delay (uS)", 100, 10., 0.0);

  for (point_t* pt = mmc_chart->base; pt < mmc_chart->current; ++pt)
  {
    if ((pt->x & 0xf) == 0x8)
      histo->bump(double(pt->y) / 333.);
  }

  histo->print();
}


extern "C" void rce_appmain()
{
  printf("\n%s: chart = 0x%08x, base = 0x%08x, pt size = %08x\n\n", __func__,
         (uint)mmc_chart, (uint)TOOL_CHART_base(mmc_chart), sizeof(point_t));

  //acquire(64);

  analyze();
}


    } // test

  } // sd

} // driver

