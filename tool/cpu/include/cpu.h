// -*-Mode: C;-*-
/*!@file     cpu.h
*
* @brief     CPU definitions
*
*            This header provides CPU-related prototypes.
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      May 30, 2012 -- Created
*
* @verbatim                    Copyright 2012
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef TOOL_CPU_CPU_H
#define TOOL_CPU_CPU_H

/** @brief Perform CPU initialization */
inline void TOOL_CPU_initialize();

/** @brief Reset the CPU accoring to @a rst type
 *  @param rst The type of reset to issue
 */
inline void TOOL_CPU_reset(uint32_t rst);

#include "cpu/gen/cpu-inl.h"

#endif
