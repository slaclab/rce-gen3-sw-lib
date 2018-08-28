#include <stdio.h>
#include <errno.h>
#include <rtems.h>
#include <rtems/fsmount.h>
#include <rtems/dosfs.h>
#include <rtems/bdpart.h>
#include <rtems/error.h>
#include <rtems/status-checks.h>


#include "chart/chart.h"
#include "histogram/Histogram.hh"


extern "C" chart_t* mmc_chart_rd;
extern "C" chart_t* mmc_chart_wr;

namespace driver {

  namespace sd {

    namespace test {

void my_analyze()
{
  using namespace tool::histogram;

  Histogram* histo_rd = new Histogram("Read rate (MB/s)", 50, 1., 0.0);

  for (point_t* pt = mmc_chart_rd->base; pt < mmc_chart_rd->current; ++pt)
  {
    histo_rd->bump(double(pt->x) / double(pt->y));
  }

  histo_rd->print();

  Histogram* histo_wr = new Histogram("Write rate (MB/s)", 50, 1., 0.0);

  for (point_t* pt = mmc_chart_wr->base; pt < mmc_chart_wr->current; ++pt)
  {
    histo_wr->bump(double(pt->x) / double(pt->y));
  }

  histo_wr->print();
}


extern "C" void rce_appmain()
{
  printf("\n%s: Read  chart = 0x%08x, base = 0x%08x, pt size = %08x\n\n", __func__,
         (uint)mmc_chart_rd, (uint)TOOL_CHART_base(mmc_chart_rd), sizeof(point_t));

  printf("\n%s: Write chart = 0x%08x, base = 0x%08x, pt size = %08x\n\n", __func__,
         (uint)mmc_chart_wr, (uint)TOOL_CHART_base(mmc_chart_wr), sizeof(point_t));

  my_analyze();
}


    } // test

  } // sd

} // driver

