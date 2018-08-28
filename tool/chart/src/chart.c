/*!
*
* @brief   A set of functions to accumulate a series of (X,Y) points in memory.
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    August 22, 2013 -- Created
*
* $Revision: 3125 $
*
* @verbatim:
*                               Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#include <malloc.h>
#include <stdio.h>
#include <inttypes.h>

#include "chart/chart.h"


chart_t* TOOL_CHART_create(uint32_t entries)
{
  chart_t* chart = malloc(sizeof(*chart));
  if (chart == 0) {
    printf("%s: No memory for chart control structure\n", __func__);
    return 0;
  }

  chart->base = calloc(entries, sizeof(*chart->base));
  if (chart->base == 0) {
    printf("%s: No memory for chart of %08" PRIx32 " entries\n", __func__, entries);
    return 0;
  }

  chart->end = chart->base + entries;

  chart->current = chart->base;

  TOOL_CHART_offset(chart, 0);
  TOOL_CHART_reference(chart, 0, 0);

  chart->marks = 0;

  return chart;
}


void TOOL_CHART_reset(chart_t* chart)
{
  chart->current = chart->base;
  chart->marks   = 0;
}


#if 0                                   /* Currently unused and untested */
void TOOL_CHART_save(chart_t* chart, const char* fileSpec)
{
  size_t n;
  FILE*  file = fopen(fileSpec, "wb");
  if (!file)
  {
    printf("%s: Couldn't open file %s for write\n", __func__, fileSpec);
    return;
  }

  if (chart->marks >= chart->end - chart->base) {
    n = chart->end - chart->current;
    if (fwrite(chart->current, sizeof(point_t), n, file) != n)
      printf("%s: Unable to write file %s\n", __func__, filespec);
  }
  n = chart->current - chart->base;
  if (fwrite(chart->base, sizeof(point_t), n, file) != n)
    printf("%s: Unable to write file %s\n", __func__, filespec);

  if (fclose(file) == -1)
  {
    printf("%s: File %s didn't properly close\n", __func__, fileSpec);
  }
}
#endif

void TOOL_CHART_save(chart_t* chart, const char* fileSpec)
{
  point_t* pt;
  FILE*    file = fopen(fileSpec, "w");
  if (!file)
  {
    printf("%s: Couldn't open file %s for write\n", __func__, fileSpec);
    return;
  }

  if (chart->marks >= chart->end - chart->base) {
    for (pt = chart->current; pt < chart->end; ++pt)
      fprintf(file, "%" PRIu32 " %" PRIu64 " %" PRIu32 "\n", pt->x, pt->y, pt->z);
  }
  for (pt = chart->base; pt < chart->current; ++pt)
    fprintf(file, "%" PRIu32 " %" PRIu64 " %" PRIu32 "\n", pt->x, pt->y, pt->z);

  if (fclose(file) == -1)
  {
    printf("%s: File %s didn't properly close\n", __func__, fileSpec);
  }
}

void TOOL_CHART_delete(chart_t* chart)
{
  free(chart->base);
  free(chart);
}
