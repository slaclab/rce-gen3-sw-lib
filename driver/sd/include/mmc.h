/* -*-Mode: C;-*- */
/**
* @file      mmc.c
*
* @brief     Multi-Media Card driver for RTEMS based on the U-Boot code:
*
 * Copyright 2008,2010 Freescale Semiconductor, Inc
 * Andy Fleming
 *
 * Based (loosely) on the Linux code
 *
 * SPDX-License-Identifier:     GPL-2.0+
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      May 14, 2013 -- Created
*
* $Revision: 2961 $
*
* @verbatim                    Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

#ifndef _MMC_H_
#define _MMC_H_

#include <rtems.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @defgroup rtems_mmc Multi Media Card
 *
 * @ingroup rtems_blkdev
 *
 */
/**@{**/

/**
 * @name Host capabilities
 */
/**@{**/

#define MMC_MODE_HS		0x001
#define MMC_MODE_HS_52MHz	0x010
#define MMC_MODE_4BIT		0x100
#define MMC_MODE_8BIT		0x200
#define MMC_MODE_SPI		0x400
#define MMC_MODE_HC		0x800

/** @} */

/* Enable SDMA use */
#define CONFIG_MMC_SDMA

/* Forward declaration */
struct mmc;

/**
 * @name MMC disk specific ioctl request types.
 *
 * To use open the device and issue the ioctl call.
 *
 * @code
 *  int fd = open ("/dev/mmc0", O_WRONLY, 0);
 *  if (fd < 0)
 *  {
 *    printf ("driver open failed: %s\n", strerror (errno));
 *    exit (1);
 *  }
 *  struct {
 *    uint32_t start;
 *    uint32_t count;
 *  } arg;
 *  if (ioctl (fd, RTEMS_MMC_IOCTL_ERASE_BLOCKS, &arg) < 0)
 *  {
 *    printf ("driver erase failed: %s\n", strerror (errno));
 *    exit (1);
 *  }
 *  close (fd);
 * @endcode
 */
/**@{**/
#define RTEMS_MMC_IOCTL_ERASE_BLOCKS _IO('B', 128)
#define RTEMS_MMC_IOCTL_PRINT_STATUS _IO('B', 131)

/** @} */

rtems_status_code rtems_mmc_attach(const rtems_driver_address_table* ops);

uint32_t          mmc_erase_blocks(struct mmc* mmc,
                                   uint32_t    start,
                                   uint32_t    blkcnt);

uint32_t          mmc_read_blocks(struct mmc* mmc,
                                  uint32_t    start,
                                  uint32_t    blkcnt,
                                  uint8_t*    dst);

uint32_t          mmc_write_blocks(struct mmc*    mmc,
                                   uint32_t       start,
                                   uint32_t       blkcnt,
                                   const uint8_t* src);
/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MMC_H_ */
