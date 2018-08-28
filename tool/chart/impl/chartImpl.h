/*!
*
* @brief   Private portions of the chart package.
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
#ifndef _TOOL_CHART_CHART_H_
#define _TOOL_CHART_CHART_H_

struct Chart {
  uint32_t marks;
  point_t* base;
  point_t* current;
  point_t* end;
  point_t  reference;
};

#endif
