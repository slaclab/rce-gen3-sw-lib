// -*-Mode: C;-*-
/*!@file     init.c
*
* @brief     Initialization code for launching the SD driver.
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      August 25, 2012 -- Created
*
* $Revision: 2593 $
*
* @verbatim                    Copyright 2012
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


#include "sd/init.h"
#include "system/fs.h"

rtems_status_code sd_initialize(void)
{
  rtems_status_code sc          = RTEMS_SUCCESSFUL;

  sc = sd_card_register();
  RTEMS_CHECK_SC(sc, "Register SD Card");

  sc = rtems_bdpart_register_from_disk(SD_DEVICE_FILE);
  if (sc != RTEMS_SUCCESSFUL) {
    printf("Partition table read failed: %s\n", rtems_status_text(sc));
  }

  return sc;
}
