/*!@file     handoff.c
*
* @brief     Transfer control to a loaded image via a well-known location
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      May 7, 2013 -- Created
*
* $Revision: 1684 $
*
* @verbatim                    Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#include "fsbl.h"
#include "xstatus.h"

#include "handoff.h"


int main(void)
{
  u32  status = XST_SUCCESS;
  u32* addr   = (u32*)HANDOFF_ADDR;

  FsblHandoffExit(*addr);

  return status;
}
