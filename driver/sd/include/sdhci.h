/* -*-Mode: C;-*- */
/**
* @file      sdhci.hh
*
* @brief     SD Host Controller Interface driver.
*
*            Patterned after:
*
 *  SD Card LibI2C driver.
 *
 * Copyright (c) 2008
 * Embedded Brains GmbH
 * Obere Lagerstr. 30
 * D-82178 Puchheim
 * Germany
 * rtems@embedded-brains.de
 *
 * The license and distribution terms for this file may be found in the file
 * LICENSE in this distribution or at http://www.rtems.com/license/LICENSE.
*
* and:
*
 * Copyright 2011, Marvell Semiconductor Inc.
 * Lei Wen <leiwen@marvell.com>
 *
 * SPDX-License-Identifier:    GPL-2.0+
 *
 * Back ported to the 8xx platform (from the 8260 platform) by
 * Murray.Jensen@cmst.csiro.au, 27-Jan-01.
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      August 21, 2012 -- Created
*
* $Revision: 2961 $
*
* @verbatim                    Copyright 2012
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

#ifndef _DRIVER_SD_SDHCI_HH_
#define _DRIVER_SD_SDHCI_HH_

#include <rtems.h>
#include <rtems/blkdev.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @defgroup rtems_sdhci SD Host Device
 *
 * @ingroup rtems_blkdev
 *
 */
/**@{**/

/**
 * @name Quirks
 */
/**@{**/

#define SDHCI_QUIRK_32BIT_DMA_ADDR	(1 << 0)
#define SDHCI_QUIRK_REG32_RW		(1 << 1)
#define SDHCI_QUIRK_BROKEN_R1B		(1 << 2)
#define SDHCI_QUIRK_NO_HISPD_BIT	(1 << 3)
#define SDHCI_QUIRK_BROKEN_VOLTAGE	(1 << 4)
#define SDHCI_QUIRK_NO_CD		(1 << 5)
#define SDHCI_QUIRK_WAIT_SEND_CMD	(1 << 6)
#define SDHCI_QUIRK_NO_SIMULT_VDD_AND_POWER (1 << 7)
#define SDHCI_QUIRK_USE_WIDE8           (1 << 8)

/** @} */

/*
 * @name Host attributes
 */
/**@{**/

#define SDHCI_USE_SDMA                  (1 << 0) /* Host is SDMA capable */
#define SDHCI_USE_ADMA                  (1 << 1) /* Host is ADMA capable */

/** @} */

/**
 * @name Static Configuration
 */
/**@{**/

/**
 * @brief SDHCI host configuration table entry.
 */
typedef struct rtems_sdhci_config {
  uintptr_t ioaddr;              /**< Base address of the device. */
  unsigned  irqno;               /**< The controller's interrupt request number. */
  unsigned  flags;               /**< Host attributes. */
  unsigned  quirks;              /**< Bit list of host controller "quirks" affecting the driver's behavior. */
  unsigned  max_clk;             /**< Maximum host clock frequency supported */
  unsigned  min_clk;             /**< Minimum host clock frequency supported */
  bool      verbose;             /**< Verbosity flag. */
} rtems_sdhci_config;

/**
 * @brief External reference to the SD host configuration table describing
 * each SD disk in the system.
 *
 * The configuration table is provided by the application.
 */
extern const rtems_sdhci_config rtems_sdhci_configuration[];

/**
 * @brief External reference the size of the SD disk configuration table
 * @ref rtems_sdhci_configuration.
 *
 * The configuration table size is provided by the application.
 */
extern size_t rtems_sdhci_configuration_size;

/**
 * @brief SD disk driver initialization entry point.
 */
rtems_device_driver
rtems_sdhci_initialize( rtems_device_major_number major,
                        rtems_device_minor_number minor,
                        void*                     arg );

/**
 * SD disk driver table entry.
 */
#define SDHCI_DRIVER_TABLE_ENTRY \
  { \
    .initialization_entry = rtems_sdhci_initialize, \
    RTEMS_GENERIC_BLOCK_DEVICE_DRIVER_ENTRIES \
  }

/**
 * The base name of the SD disks.
 */
#define SDHCI_DISK_DEVICE_BASE_NAME "/dev/sd"

/**
 * @brief Initializes and registers a SD disk.
 *
 * @retval RTEMS_SUCCESSFUL Successful operation.
 * @retval RTEMS_UNSATISFIED Something is wrong.
 */
rtems_status_code sdhci_register();

/** @} */

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DRIVER_SD_SDHCI_HH_ */
