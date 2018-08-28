/*
 *  This routine is an implementation of the bsp_work_area_initialize()
 *  that can be used by all BSPs following linkcmds conventions
 *  regarding heap, stack, and workspace allocation.
 *
 *  COPYRIGHT (c) 1989-2008.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  Copyright (c) 2011-2012 embedded brains GmbH.
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 */
 


#include "memory/mem.h"
 
#include <bsp/bootcard.h>

void bsp_work_area_initialize(void)
{
  /* Give RTEMS the entire remaining WORKSPACE Region less some space we hold back. */
  unsigned const wappSpace   = mem_Region_workspaceHoldback();
  unsigned const wspaceSize  = mem_Region_remaining(MEM_REGION_WORKSPACE) - wappSpace;
  if (wspaceSize <= 0)
    dbg_bugcheck("bsp_work_area_initialize(): The remaining WORKSPACE Region is too small.\n"
                 "It must have at least %u bytes + space for RTEMS.\n",
                 wappSpace);
  void*    const wspaceStart = mem_Region_alloc(MEM_REGION_WORKSPACE, wspaceSize);
  if (!wspaceStart)
    dbg_bugcheck("bsp_work_area_initialize(): Can't allocate WORKSPACE Region space for RTEMS.\n");
  bsp_work_area_initialize_default(wspaceStart, wspaceSize);
}
