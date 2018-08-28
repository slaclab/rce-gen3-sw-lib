/*!
*
* @brief   A set of functions to accumulate a series of (X,Y) points in memory.
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
#ifndef TOOL_CHART_CHART_H
#define TOOL_CHART_CHART_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct Point {
  int32_t x;
  int32_t z;                          /* Dummy for now, needed for alignment */
  int64_t y;
} point_t;

typedef struct Chart chart_t;

/* Control functions */
chart_t* TOOL_CHART_create(uint32_t count);
void     TOOL_CHART_delete(chart_t* chart);
void     TOOL_CHART_reset(chart_t* chart);

/* Get functions */
static inline point_t* TOOL_CHART_base(chart_t* chart);
static inline point_t* TOOL_CHART_current(chart_t* chart);
static inline uint32_t TOOL_CHART_entries(chart_t* chart);
static inline uint32_t TOOL_CHART_marks(chart_t* chart);

/* Set functions */
static inline void TOOL_CHART_offset(chart_t* chart, int32_t x);
static inline void TOOL_CHART_reference(chart_t* chart, int32_t x, int64_t y);

/* Data accumulator functions */
static inline void TOOL_CHART_mark_XY(chart_t* chart, int32_t x, int64_t y);
static inline void TOOL_CHART_mark_XdY(chart_t* chart, int32_t x, int64_t y);
static inline void TOOL_CHART_mark_dXdY(chart_t* chart, int32_t x, int64_t y);
static inline void TOOL_CHART_mark_Y(chart_t* chart, int64_t y);
static inline void TOOL_CHART_mark_dY(chart_t* chart, int64_t y);

/**/
void TOOL_CHART_save(chart_t* chart, const char* fileSpec);


#include "chart/impl/chart-inl.h"

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
