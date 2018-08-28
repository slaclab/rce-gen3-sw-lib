/*!@file     fs.h
*
* @brief     File system parameters
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      August 25, 2012 -- Created
*
* $Revision: 2469 $
*
* @verbatim                    Copyright 2012
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef CONFIGURATION_SYSTEM_FS_H
#define CONFIGURATION_SYSTEM_FS_H

#   define SD_DEVICE      "sdA"
#   define SD_DEVICE_FILE "/dev/" SD_DEVICE

#   define SD_PARTITION_BOOT      SD_DEVICE_FILE "1"
#   define SD_PARTITION_SCRATCH   SD_DEVICE_FILE "2"
#   define SD_PARTITION_RTEMS     SD_DEVICE_FILE "4"
#   define SD_PARTITION_RTEMSAPP  SD_DEVICE_FILE "5"

#   define SD_MOUNT_POINT "/mnt"
#   define SD_MOUNT_POINT_BOOT      SD_MOUNT_POINT "/boot"
#   define SD_MOUNT_POINT_SCRATCH   SD_MOUNT_POINT "/scratch"
#   define SD_MOUNT_POINT_RTEMS     SD_MOUNT_POINT "/rtems"
#   define SD_MOUNT_POINT_RTEMSAPP  SD_MOUNT_POINT "/rtemsapp"

#   define SD_PARTITION_TYPE_BOOT      "dosfs"
#   define SD_PARTITION_TYPE_SCRATCH   "dosfs"
#   define SD_PARTITION_TYPE_RTEMS     "dosfs"
#   define SD_PARTITION_TYPE_RTEMSAPP  "dosfs"

#endif
