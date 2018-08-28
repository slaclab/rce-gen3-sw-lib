// -*-Mode: C++;-*-
/**
 * @file      sd-card.hh
 *
 * @brief     SD Card driver.
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

#ifndef _DRIVER_SD_SD_CARD_HH_
#define _DRIVER_SD_SD_CARD_HH_

#include <stdint.h>
#include <stdbool.h>

#define SD_CARD_N_AC_MAX_DEFAULT 8

#define SD_CARD_CMD_FIFO_DEPTH  512
#define SD_CARD_RSP_FIFO_DEPTH 1024
#define SD_CARD_TX_FIFO_DEPTH   512
#define SD_CARD_RX_FIFO_DEPTH   512

typedef struct {
  const char* device_name;
  uint32_t    n_ac_max;
  uint32_t    block_number;
  uint32_t    block_size;
  uint32_t    block_size_shift;
  uint32_t    cmdFifoDepth;
  uint32_t    txFifoDepth;
  uint8_t     tid;
  bool        verbose;
} sd_card_driver_entry;

namespace configuration
{
  namespace system
  {
    extern sd_card_driver_entry sd_card_driver_table[];

    extern size_t sd_card_driver_table_size;
  }
}

namespace driver
{
  namespace sd
  {
    rtems_status_code sd_card_register(void);
  }
}

#endif /* _DRIVER_SD_SD_CARD_HH_ */
