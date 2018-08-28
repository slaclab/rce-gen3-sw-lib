/*!
*
* @brief   Inline portions of the chart package.
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    August 22, 2013 -- Created
*
* $Revision: 2961 $
*
* @verbatim:
*                               Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#include "chart/impl/chartImpl.h"

static inline point_t* TOOL_CHART_base(chart_t* chart)
{
  return chart->base;
}

static inline point_t* TOOL_CHART_current(chart_t* chart)
{
  return chart->current;
}

static inline uint32_t TOOL_CHART_entries(chart_t* chart)
{
  return chart->end - chart->base;
}

static inline uint32_t TOOL_CHART_marks(chart_t* chart)
{
  return chart->marks;
}

static inline void TOOL_CHART_offset(chart_t* chart, int32_t x)
{
  chart->current->x = x;
}

static inline void TOOL_CHART_reference(chart_t* chart, int32_t x, int64_t y)
{
  chart->reference.x = x;
  chart->reference.y = y;
}

static inline void TOOL_CHART_mark_XY(chart_t* chart, int32_t x, int64_t y)
{
  chart->current->x = x;
  chart->current->y = y;

  if (++chart->current >= chart->end)
    chart->current = chart->base;

  ++chart->marks;
}

static inline void TOOL_CHART_mark_XdY(chart_t* chart, int32_t x, int64_t y)
{
  TOOL_CHART_mark_XY(chart, x, y - chart->reference.y);
  chart->reference.y = y;
}

static inline void TOOL_CHART_mark_dXdY(chart_t* chart, int32_t x, int64_t y)
{
  TOOL_CHART_mark_XY(chart, x - chart->reference.x, y - chart->reference.y);
  chart->reference.x = x;
  chart->reference.y = y;
}

static inline void TOOL_CHART_mark_Y(chart_t* chart, int64_t y)
{
  TOOL_CHART_mark_XY(chart, chart->current->x + 1, y);
}

static inline void TOOL_CHART_mark_dY(chart_t* chart, int64_t y)
{
  TOOL_CHART_mark_XdY(chart, chart->current->x + 1, y);
}
