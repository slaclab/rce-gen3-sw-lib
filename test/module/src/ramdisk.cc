// -*-Mode: C++;-*-
/*!
*
* @brief   A test program to create a ram disk
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    June 27, 2012 -- Created
*
* $Revision: 1929 $
*
* @verbatim
*                               Copyright 2012
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#include <stdio.h>
#include <rtems.h>
#include <rtems/libio.h>
#include <rtems/ramdisk.h>
#include <rtems/dosfs.h>
#include <rtems/fsmount.h>

extern "C"
{

rtems_status_code create_ramdisk(
  const char *disk_name_path,
  uint32_t block_size,
  rtems_blkdev_bnum block_count
)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;
  rtems_device_major_number major = 0;
  ramdisk *rd = NULL;
  dev_t dev = 0;

  /*
   * Register the RAM Disk driver.
   */
  printf ("Register RAM Disk Driver: ");
  sc = rtems_io_register_driver(0, &ramdisk_ops, &major);
  if (sc != RTEMS_SUCCESSFUL) {
    return RTEMS_UNSATISFIED;
  }

  rd = ramdisk_allocate(NULL, block_size, block_count, false);
  if (rd == NULL) {
    rtems_io_unregister_driver(major);

    return RTEMS_UNSATISFIED;
  }

  dev = rtems_filesystem_make_dev_t(major, 0);

  sc = rtems_disk_create_phys(
    dev,
    block_size,
    block_count,
    ramdisk_ioctl,
    rd,
    disk_name_path
  );
  if (sc != RTEMS_SUCCESSFUL) {
    ramdisk_free(rd);
    rtems_io_unregister_driver(major);

    return RTEMS_UNSATISFIED;
  }

  return RTEMS_SUCCESSFUL;
}


/**
 * The RAM Disk configuration.
 */
rtems_ramdisk_config rtems_ramdisk_configuration[] =
{
  {
    512,  /* block size */
    4000, /* block_num */
    NULL  /* location */
  }
};

/**
 * The number of RAM Disk configurations.
 */
size_t rtems_ramdisk_configuration_size = 1;

}

#include "datCode.hh"
#include DAT_PUBLIC(service, logger, Logger.hh)
using service::logger::Logger;


extern "C" void rce_appmain(uintptr_t)
{
  Logger().info("Creating ramdisk at /ram");
  rtems_status_code rc = create_ramdisk("/ram", 512, 100);
  Logger().info("create_ramdisk returned %08x", rc);

  msdos_format_request_param_t rqdata;

  memset(&rqdata,0,sizeof(rqdata));
  rqdata.OEMName="Rtems";
  rqdata.VolLabel="MyRTEMSDisk";
  rqdata.fattype=MSDOS_FMT_FATANY;
  rqdata.quick_format=TRUE;

  msdos_format("/dev/ramdisk0",&rqdata);

#if 0
  fstab_t fs_mnt_table[] = {
    {
      "/dev/ramdisk0","/mnt/ramdisk",
      &msdos_ops, RTEMS_FILESYSTEM_READ_WRITE,
      FSMOUNT_MNT_OK | FSMOUNT_MNT_CRTERR | FSMOUNT_MNT_FAILED,
      0
    }
  };

  rtems_fsmount(fs_mnt_table,
                sizeof(fs_mnt_table)/sizeof(fs_mnt_table[0]),
                NULL);
#endif
}
