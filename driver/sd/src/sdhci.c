/**
* @file      sdhci.cc
*
* @brief     SD Host Controller Interface driver based on the U-Boot code:
*
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
* @date      May 14, 2013 -- Created
*
* $Revision: 3121 $
*
* @verbatim                    Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
//#ifndef DEBUG
//#  define DEBUG
//#endif

#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include <malloc.h>
#include <stdlib.h>

#include <rtems.h>
#include <rtems/diskdevs.h>
#include <rtems/blkdev.h>
#include <rtems/irq-extension.h>
#include <rtems/status-checks.h>


#include "sd/mmc.h"
#include "mmcImpl.h"
#include "sd/sdhci.h"
#include "sdhciImpl.h"
#include "chart/chart.h"


/**
 * fls - find last (most-significant) bit set
 * @x: the word to search
 *
 * This is defined the same way as ffs.
 * Note fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32.
 */
static inline int fls(int x)
{
  int r = 32;

  if (!x) {
    return 0;
  }
  if (!(x & 0xffff0000u)) {
    x <<= 16;
    r -= 16;
  }
  if (!(x & 0xff000000u)) {
    x <<= 8;
    r -= 8;
  }
  if (!(x & 0xf0000000u)) {
    x <<= 4;
    r -= 4;
  }
  if (!(x & 0xc0000000u)) {
    x <<= 2;
    r -= 2;
  }
  if (!(x & 0x80000000u)) {
    x <<= 1;
    r -= 1;
  }
  return r;
}

static void *aligned_buffer;

static void sdhci_dumpregs(struct sdhci_host* host)
{
  printf("%s: =========== REGISTER DUMP (%s)===========\n",
         host->hw_name, host->mmc.name);

  printf("%s: IO addr:  0x%08x | Version:  0x%08x\n",
         host->hw_name,
         host->ioaddr,
         sdhci_readw(host, SDHCI_HOST_VERSION));
  printf("%s: Blk cnt:  0x%08x | Blk size: 0x%08x\n",
         host->hw_name,
         sdhci_readw(host, SDHCI_BLOCK_COUNT),
         sdhci_readw(host, SDHCI_BLOCK_SIZE));
  printf("%s: Cmd:      0x%08x | Arg:      0x%08lx\n",
         host->hw_name,
         sdhci_readw(host, SDHCI_COMMAND),
         sdhci_readl(host, SDHCI_ARGUMENT));
  printf("%s: Rsp 0:    0x%08lx | Rsp 1:    0x%08lx\n",
         host->hw_name,
         sdhci_readl(host, SDHCI_RESPONSE + 0),
         sdhci_readl(host, SDHCI_RESPONSE + 4));
  printf("%s: Rsp 2:    0x%08lx | Rsp 3:    0x%08lx\n",
         host->hw_name,
         sdhci_readl(host, SDHCI_RESPONSE + 8),
         sdhci_readl(host, SDHCI_RESPONSE + 12));
  printf("%s: Buf Data: 0x%08lx | Present:  0x%08lx\n",
         host->hw_name,
         sdhci_readl(host, SDHCI_BUFFER),
         sdhci_readl(host, SDHCI_PRESENT_STATE));
  printf("%s: Trn mode: 0x%08x | Host ctl: 0x%08x\n",
         host->hw_name,
         sdhci_readw(host, SDHCI_TRANSFER_MODE),
         sdhci_readb(host, SDHCI_HOST_CONTROL));
  printf("%s: Power:    0x%08x | Blk gap:  0x%08x\n",
         host->hw_name,
         sdhci_readb(host, SDHCI_POWER_CONTROL),
         sdhci_readb(host, SDHCI_BLOCK_GAP_CONTROL));
  printf("%s: Wake-up:  0x%08x | Clock:    0x%08x\n",
         host->hw_name,
         sdhci_readb(host, SDHCI_WAKE_UP_CONTROL),
         sdhci_readw(host, SDHCI_CLOCK_CONTROL));
  printf("%s: Timeout:  0x%08x | Int stat: 0x%08lx\n",
         host->hw_name,
         sdhci_readb(host, SDHCI_TIMEOUT_CONTROL),
         sdhci_readl(host, SDHCI_INT_STATUS));
  printf("%s: Int enab: 0x%08lx | Sig enab: 0x%08lx\n",
         host->hw_name,
         sdhci_readl(host, SDHCI_INT_ENABLE),
         sdhci_readl(host, SDHCI_SIGNAL_ENABLE));
  printf("%s: AC12 err: 0x%08x | Slot int: 0x%08x\n",
         host->hw_name,
         sdhci_readw(host, SDHCI_ACMD12_ERR),
         sdhci_readw(host, SDHCI_SLOT_INT_STATUS));
  printf("%s: Caps:     0x%08lx | Max curr: 0x%08lx\n",
         host->hw_name,
         sdhci_readl(host, SDHCI_CAPABILITIES),
         sdhci_readl(host, SDHCI_MAX_CURRENT));
  if (host->mmc.host_caps & SDHCI_USE_ADMA)
    printf("%s: ADMA Err: 0x%08lx | ADMA adx: 0x%08lx\n",
           host->hw_name,
           sdhci_readl(host, SDHCI_ADMA_ERROR),
           sdhci_readl(host, SDHCI_ADMA_ADDRESS));
#ifdef CONFIG_MMC_SDMA
  printf("%s: SDMA adx: 0x%08lx\n",
         host->hw_name,
         sdhci_readl(host, SDHCI_DMA_ADDRESS));
#endif

  printf("%s: ===========================================\n",
         host->hw_name);
}

static void sdhci_reset(struct sdhci_host* host, uint8_t mask)
{
  uint32_t timeout;

  /* Wait max 100 ms */
  timeout = 100;
  sdhci_writeb(host, mask, SDHCI_SOFTWARE_RESET);
  while (sdhci_readb(host, SDHCI_SOFTWARE_RESET) & mask) {
    if (timeout == 0) {
      RTEMS_SYSLOG("Reset 0x%x never completed\n", (int)mask);
      return;
    }
    timeout--;
    udelay(1000);
  }
}

static void sdhci_cmd_done(struct sdhci_host* host, struct mmc_cmd* cmd)
{
  int i;
  if (cmd->resp_type & MMC_RSP_136) {
    /* CRC is stripped so we need to do some shifting. */
    for (i = 0; i < 4; i++) {
      cmd->response[i] = sdhci_readl(host, SDHCI_RESPONSE + (3-i)*4) << 8;
      if (i != 3)
        cmd->response[i] |= sdhci_readb(host, SDHCI_RESPONSE + (3-i)*4-1);
    }
  } else {
    cmd->response[0] = sdhci_readl(host, SDHCI_RESPONSE);
  }
}

static void sdhci_transfer_pio(struct sdhci_host* host, struct mmc_data* data)
{
  int i;

  if (data->flags == MMC_DATA_READ) {
    uint32_t* dst = (uint32_t*)data->dst;

    for (i = 0; i < data->blocksize; i += 4)
      *dst++ = sdhci_readl(host, SDHCI_BUFFER);

  } else {
    const uint32_t* src = (const uint32_t*)data->src;

    for (i = 0; i < data->blocksize; i += 4)
      sdhci_writel(host, *src++, SDHCI_BUFFER);

    udelay(20);                       // RiC: Or we get command timeouts
  }
}

static int sdhci_transfer_data(struct sdhci_host* host,
                               struct mmc_data*   data,
                               unsigned           start_addr)
{
  rtems_status_code sc;
  unsigned stat, rdy, mask, block = 0;
  const unsigned intMask = (SDHCI_INT_SPACE_AVAIL | SDHCI_INT_DATA_AVAIL |
                            SDHCI_INT_DMA_END     | SDHCI_INT_DATA_END   |
                            SDHCI_INT_ERROR);

#ifdef CONFIG_MMC_SDMA
  uint8_t ctrl;
  ctrl  = sdhci_readb(host, SDHCI_HOST_CONTROL);
  ctrl &= ~SDHCI_CTRL_DMA_MASK;
  sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);
#endif

  rdy  = SDHCI_INT_SPACE_AVAIL | SDHCI_INT_DATA_AVAIL;
  mask = SDHCI_DATA_AVAILABLE  | SDHCI_SPACE_AVAILABLE;
  do {
    /* Process any already signalled interrupts first */
    stat = sdhci_readl(host, SDHCI_INT_STATUS);
    if ((stat & intMask) == 0) {
      const unsigned timeout = rtems_clock_get_ticks_per_second();

      /* Enable interrupt signals */
      sdhci_writel(host, intMask, SDHCI_SIGNAL_ENABLE);

      /* Wait for a signal */
      sc = rtems_semaphore_obtain(host->sem_intr, RTEMS_WAIT, timeout);
      if (sc != RTEMS_SUCCESSFUL)
        break;

      stat = sdhci_readl(host, SDHCI_INT_STATUS);
    }
    if (stat & SDHCI_INT_ERROR)
      break;

    if (stat & rdy) {
      if (!(sdhci_readl(host, SDHCI_PRESENT_STATE) & mask))
        continue;
      sdhci_writel(host, rdy, SDHCI_INT_STATUS);
      sdhci_transfer_pio(host, data);
      data->dst += data->blocksize;
      if (++block >= data->blocks)
        break;
    }
#ifdef CONFIG_MMC_SDMA
    if (stat & SDHCI_INT_DMA_END) {
      sdhci_writel(host, SDHCI_INT_DMA_END, SDHCI_INT_STATUS);
      start_addr &= ~(SDHCI_DEFAULT_BOUNDARY_SIZE - 1);
      start_addr += SDHCI_DEFAULT_BOUNDARY_SIZE;
      sdhci_writel(host, start_addr, SDHCI_DMA_ADDRESS);
    }
#endif
  } while (!(stat & SDHCI_INT_DATA_END));

  if (sc != RTEMS_SUCCESSFUL) {
    if (sc == RTEMS_TIMEOUT)
      RTEMS_SYSLOG("Transfer data timeout\n");
    else
      RTEMS_SYSLOG("Semaphore obtain failed\n");
    return -1;
  }

  if (stat & SDHCI_INT_ERROR) {
    RTEMS_SYSLOG("Error bit set in normal interrupt status: 0x%08x\n", stat);
    return -1;
  }

  return 0;
}

static int sdhci_send_command(struct mmc*      mmc,
                              struct mmc_cmd*  cmd,
                              struct mmc_data* data)
{
  struct sdhci_host* host = (struct sdhci_host*)mmc->priv;
  unsigned stat = 0;
  int      ret  = 0;
  int      trans_bytes = 0, is_aligned = 1;
  unsigned mask, flags, mode;
  unsigned timeout = 0;
  uintptr_t start_addr = 0;
  rtems_status_code sc = RTEMS_SUCCESSFUL;

  /* Clear all pending interrupts */
  sdhci_writel(host, SDHCI_INT_ALL_MASK, SDHCI_INT_STATUS);

  mask = SDHCI_CMD_INHIBIT;
  if (data || (cmd->resp_type & MMC_RSP_BUSY))
    mask |= SDHCI_DATA_INHIBIT;

  /* We shouldn't wait for data inihibit with stop commands, even
     though they might use busy signaling */
  if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION)
    mask &= ~SDHCI_DATA_INHIBIT;

  /* Wait max 10 ms for inhibits to be released */
  timeout = 10;
  while (sdhci_readl(host, SDHCI_PRESENT_STATE) & mask) {
    if (timeout == 0) {
      RTEMS_SYSLOG("Controller never released inhibit bit(s)\n");
      sdhci_dumpregs(host);
      sdhci_reset(host, SDHCI_RESET_CMD);
      sdhci_reset(host, SDHCI_RESET_DATA);
      return COMM_ERR;
    }
    timeout--;
    udelay(1000);
  }

  mask = SDHCI_INT_RESPONSE;
  if (!(cmd->resp_type & MMC_RSP_PRESENT))
    flags = SDHCI_CMD_RESP_NONE;
  else if (cmd->resp_type & MMC_RSP_136)
    flags = SDHCI_CMD_RESP_LONG;
  else if (cmd->resp_type & MMC_RSP_BUSY) {
    flags = SDHCI_CMD_RESP_SHORT_BUSY;
    mask |= SDHCI_INT_DATA_END;
  } else
    flags = SDHCI_CMD_RESP_SHORT;

  if (cmd->resp_type & MMC_RSP_CRC)
    flags |= SDHCI_CMD_CRC;
  if (cmd->resp_type & MMC_RSP_OPCODE)
    flags |= SDHCI_CMD_INDEX;
  if (data)
    flags |= SDHCI_CMD_DATA;
  // RiC: I think the following is wrong.  It came from FreeBSD.
  //if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION)
  //  flags |= SDHCI_CMD_ABORTCMD;

  /* Set Transfer mode regarding to data flag */
  if (data != 0) {
    sdhci_writeb(host, 0xe, SDHCI_TIMEOUT_CONTROL);
    mode = SDHCI_TRNS_BLK_CNT_EN;
    trans_bytes = data->blocks * data->blocksize;
    if (data->blocks > 1)
      mode |= SDHCI_TRNS_MULTI;

    if (data->flags == MMC_DATA_READ)
      mode |= SDHCI_TRNS_READ;

#ifdef CONFIG_MMC_SDMA
    if (data->flags == MMC_DATA_READ)
      start_addr = (unsigned)data->dst;
    else
      start_addr = (unsigned)data->src;
    if ((host->quirks & SDHCI_QUIRK_32BIT_DMA_ADDR) &&
        (start_addr & 0x7) != 0x0) {
      is_aligned = 0;
      start_addr = (unsigned)aligned_buffer;
      if (data->flags != MMC_DATA_READ)
        memcpy(aligned_buffer, data->src, trans_bytes);
    }

    sdhci_writel(host, start_addr, SDHCI_DMA_ADDRESS);
    mode |= SDHCI_TRNS_DMA;
#endif

    /* Set up Block Size, Block Count, Argument, Transfer Mode and Command
       registers in specified order */
    sdhci_writew(host, SDHCI_MAKE_BLKSZ(SDHCI_DEFAULT_BOUNDARY_ARG,
                                        data->blocksize),
                                     SDHCI_BLOCK_SIZE);
    sdhci_writew(host, data->blocks, SDHCI_BLOCK_COUNT);
    sdhci_writew(host, mode,         SDHCI_TRANSFER_MODE);
  }

  sdhci_writel(host, cmd->cmdarg,  SDHCI_ARGUMENT);

#ifdef CONFIG_MMC_SDMA
  if (data && (data->flags != MMC_DATA_READ))
    rtems_cache_flush_multiple_data_lines(data->src, trans_bytes);
#endif

  /* This issues the SD command */
  sdhci_writew(host, SDHCI_MAKE_CMD(cmd->cmdidx, flags), SDHCI_COMMAND);

  /* Wait for completion */
  do {
    const unsigned intMask = mask | SDHCI_INT_ERROR;

    /* Process any already signalled interrupts first */
    stat = sdhci_readl(host, SDHCI_INT_STATUS);
    if ((stat & mask) != mask) {
      const unsigned timeout = rtems_clock_get_ticks_per_second();

      /* Enable interrupt signalling */
      sdhci_writel(host, intMask, SDHCI_SIGNAL_ENABLE);

      /* Wait for a signal */
      sc = rtems_semaphore_obtain(host->sem_intr, RTEMS_WAIT, timeout);
      if (sc != RTEMS_SUCCESSFUL)
        break;

      stat = sdhci_readl(host, SDHCI_INT_STATUS);
    }
    if (stat & SDHCI_INT_ERROR)
      break;
  } while ((stat & mask) != mask);

  if (sc != RTEMS_SUCCESSFUL) {
    if (sc == RTEMS_TIMEOUT) {
      if (!(host->quirks & SDHCI_QUIRK_BROKEN_R1B)) {
        RTEMS_SYSLOG("Timeout waiting for status update\n");
        ret = TIMEOUT;
      }
    } else {
      RTEMS_SYSLOG("Semaphore obtain failed\n");
      ret = -1;
    }
  }

  if ((stat & (SDHCI_INT_ERROR | mask)) == mask) {
    sdhci_cmd_done(host, cmd);
    sdhci_writel(host, mask, SDHCI_INT_STATUS);
  } else {
    RTEMS_SYSLOG("Error bit set in normal interrupt status: 0x%08x\n", stat);
    ret = -1;
  }

  if (!ret && data) {
    ret = sdhci_transfer_data(host, data, start_addr);

#ifdef CONFIG_MMC_SDMA
    if (data->flags == MMC_DATA_READ)
      rtems_cache_invalidate_multiple_data_lines(data->dst, trans_bytes);
#endif
  }

  if (host->quirks & SDHCI_QUIRK_WAIT_SEND_CMD)
    udelay(1000);

  stat = sdhci_readl(host, SDHCI_INT_STATUS);
  sdhci_writel(host, SDHCI_INT_ALL_MASK, SDHCI_INT_STATUS);
  if (!ret) {
    if ((host->quirks & SDHCI_QUIRK_32BIT_DMA_ADDR) && data &&
        !is_aligned && (data->flags == MMC_DATA_READ))
      memcpy(data->dst, aligned_buffer, trans_bytes);
    return 0;
  }

  sdhci_dumpregs(host);

  /*
   * The controller needs a reset of internal state machines
   * upon error conditions.
   */
  sdhci_reset(host, SDHCI_RESET_CMD);
  sdhci_reset(host, SDHCI_RESET_DATA);
  if ((stat & SDHCI_INT_TIMEOUT) || (ret == TIMEOUT))
    return TIMEOUT;
  else
    return COMM_ERR;
}

static int sdhci_set_clock(struct mmc* mmc, unsigned clock)
{
  struct sdhci_host* host = (struct sdhci_host* )mmc->priv;
  unsigned div, clk, timeout;

  //uint32_t ctrl = sdhci_readw(host, SDHCI_CLOCK_CONTROL);

  //printf("%s: Initial Host Clock Control = %08lx\n", __func__, ctrl);
  //printf("%s: max = %u, min = %u, clock = %u\n", __func__, mmc->f_max, mmc->f_min, clock);

  //printf("%s: clock = %08x = %u\n", __func__, clock, clock);

  sdhci_writew(host, 0, SDHCI_CLOCK_CONTROL);

  if (clock == 0)
    return 0;

  if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300) {
    /* Version 3.00 divisors must be a multiple of 2. */
    if (mmc->f_max <= clock)
      div = 1;
    else {
      for (div = 2; div < SDHCI_MAX_DIV_SPEC_300; div += 2) {
        if ((mmc->f_max / div) <= clock)
          break;
      }
    }
  } else {
    /* Version 2.00 divisors must be a power of 2. */
    for (div = 1; div < SDHCI_MAX_DIV_SPEC_200; div *= 2) {
      //printf("%s: f_max = %u, clock = %u, div = %u\n", __func__, mmc->f_max / div, clock, div);
      if ((mmc->f_max / div) <= clock)
        break;
    }
  }
  div >>= 1;
  //printf("%s: Final div = %u\n", __func__, div);

  if (host->set_clock)
    host->set_clock(host->index, div);

  clk  =  (div & SDHCI_DIV_MASK)    << SDHCI_DIVIDER_SHIFT;
  clk |= ((div & SDHCI_DIV_HI_MASK) >> SDHCI_DIV_MASK_LEN) << SDHCI_DIVIDER_HI_SHIFT;
  clk |= SDHCI_CLOCK_INT_EN;
  sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

  /* Wait max 20 ms */
  timeout = 20;
  while (!((clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL)) & SDHCI_CLOCK_INT_STABLE)) {
    if (timeout == 0) {
      RTEMS_SYSLOG("Internal clock never stabilized\n");
      return -1;
    }
    timeout--;
    udelay(1000);
  }

  clk |= SDHCI_CLOCK_CARD_EN;
  sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

  host->clock = clk;

  if (mmc->verbose) {
    static const char* const no  = "no";
    static const char* const yes = "yes";

    printf("\n*** %s ***\n", __func__);
    printf("  SDCLK Divider Select:    %u\n", (clk >> SDHCI_DIVIDER_SHIFT) & SDHCI_DIV_MASK);
    if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300)
      printf("  SDCLK Divider Hi Select: %u\n", (clk & SDHCI_DIV_HI_MASK) >> SDHCI_DIVIDER_HI_SHIFT);
    printf("  SDCLK Enable:            %s\n", (clk & SDHCI_CLOCK_CARD_EN)    ? yes : no);
    printf("  Internal Clock Stable:   %s\n", (clk & SDHCI_CLOCK_INT_STABLE) ? yes : no);
    printf("  Internal Clock Enable:   %s\n", (clk & SDHCI_CLOCK_INT_EN)     ? yes : no);
    printf("  Final SDCLK frequency:   %u\n", mmc->f_max / (div + 1));
  }

  return 0;
}

static void sdhci_set_power(struct sdhci_host* host, unsigned short power)
{
  uint8_t pwr = 0;

  if (power != (unsigned short)-1) {
    switch (1 << power) {
      case MMC_VDD_165_195:
        pwr = SDHCI_POWER_180;
        break;
      case MMC_VDD_29_30:
      case MMC_VDD_30_31:
        pwr = SDHCI_POWER_300;
        break;
      case MMC_VDD_32_33:
      case MMC_VDD_33_34:
        pwr = SDHCI_POWER_330;
        break;
    }
  }

  if (pwr == 0) {
    sdhci_writeb(host, 0, SDHCI_POWER_CONTROL);
    return;
  }

  if (host->quirks & SDHCI_QUIRK_NO_SIMULT_VDD_AND_POWER)
    sdhci_writeb(host, pwr, SDHCI_POWER_CONTROL);

  pwr |= SDHCI_POWER_ON;

  sdhci_writeb(host, pwr, SDHCI_POWER_CONTROL);
}

static void sdhci_set_ios(struct mmc* mmc)
{
  uint32_t           ctrl;
  struct sdhci_host* host = (struct sdhci_host*)mmc->priv;

  if (host->set_control_reg)
    host->set_control_reg(host);

  if (mmc->clock != host->clock)
    sdhci_set_clock(mmc, mmc->clock);

  /* Set bus width */
  ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);
  if (mmc->bus_width == 8) {
    ctrl &= ~SDHCI_CTRL_4BITBUS;
    if ((SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300) ||
        (host->quirks & SDHCI_QUIRK_USE_WIDE8))
      ctrl |= SDHCI_CTRL_8BITBUS;
  } else {
    if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300)
      ctrl &= ~SDHCI_CTRL_8BITBUS;
    if (mmc->bus_width == 4)
      ctrl |= SDHCI_CTRL_4BITBUS;
    else
      ctrl &= ~SDHCI_CTRL_4BITBUS;
  }

  if (mmc->clock > 26000000)
    ctrl |= SDHCI_CTRL_HISPD;
  else
    ctrl &= ~SDHCI_CTRL_HISPD;

  if (host->quirks & SDHCI_QUIRK_NO_HISPD_BIT)
    ctrl &= ~SDHCI_CTRL_HISPD;

  sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);
}

static void sdhci_intr_handler(void* arg)
{
  struct sdhci_host* host = (sdhci_host*)arg;
  rtems_status_code  sc;

  sdhci_writel(host, ~SDHCI_INT_ALL_MASK, SDHCI_SIGNAL_ENABLE);

  sc = rtems_semaphore_release(host->sem_intr);
  if (sc != RTEMS_SUCCESSFUL)
    RTEMS_SYSLOG("Semaphore release failed");
}

static rtems_status_code sdhci_init(struct mmc* mmc)
{
  struct sdhci_host* host = (struct sdhci_host*)mmc->priv;
  rtems_status_code  sc;

  if ((host->quirks & SDHCI_QUIRK_32BIT_DMA_ADDR) && !aligned_buffer) {
    int rc = posix_memalign(&aligned_buffer, 8, 512*1024);
    if (rc || !aligned_buffer) {
      RTEMS_SYSLOG("Aligned buffer alloc failed\n");
      return RTEMS_IO_ERROR;
    }
  }

  sdhci_set_power(host, fls(mmc->voltages) - 1);

  if (host->quirks & SDHCI_QUIRK_NO_CD) {
    unsigned status;

    sdhci_writel(host, SDHCI_CTRL_CD_TEST_INS | SDHCI_CTRL_CD_TEST,
                 SDHCI_HOST_CONTROL);

    status = sdhci_readl(host, SDHCI_PRESENT_STATE);
    while ((!(status & SDHCI_CARD_PRESENT)) ||
           (!(status & SDHCI_CARD_STATE_STABLE)) ||
           (!(status & SDHCI_CARD_DETECT_PIN_LEVEL)))
      status = sdhci_readl(host, SDHCI_PRESENT_STATE);
  }

  /* Clear all interrupts */
  sdhci_writel(host, SDHCI_INT_ALL_MASK, SDHCI_INT_STATUS);

  /* Create a signalling semaphore to notify the task of interrupts */
  sc = rtems_semaphore_create(rtems_build_name('S', 'D', 'I', 'N'), 0,
                              RTEMS_SIMPLE_BINARY_SEMAPHORE , 0,
                              &host->sem_intr);
  if (sc != RTEMS_SUCCESSFUL) {
    RTEMS_SYSLOG("Interrupt semaphore creation failed\n");
    return sc;
  }

  /* Install interrupt handler and enable the vector */
  sc = rtems_interrupt_handler_install( host->irqno,
                                        "sdhci",
                                        RTEMS_INTERRUPT_UNIQUE,
                                        sdhci_intr_handler,
                                        host );
  if (sc != RTEMS_SUCCESSFUL) {
    RTEMS_SYSLOG("Interrupt handler installation failed\n");
    return sc;
  }

  /* Enable only interrupts served by the SD controller */
  sdhci_writel(host, SDHCI_INT_DATA_MASK | SDHCI_INT_CMD_MASK, SDHCI_INT_ENABLE);

  /* Mask all sdhci interrupt sources */
  sdhci_writel(host, ~SDHCI_INT_ALL_MASK, SDHCI_SIGNAL_ENABLE);

  return RTEMS_SUCCESSFUL;
}

static rtems_status_code add_sdhci(struct sdhci_host*        host,
                                   const rtems_sdhci_config* cfg,
                                   dev_t                     dev)
{
  rtems_device_minor_number minor = rtems_filesystem_dev_minor_t(dev);
  struct mmc*               mmc;
  unsigned                  caps;

  host->hw_name   = "sdhci";
  host->irqno     = cfg->irqno;
  host->ioaddr    = cfg->ioaddr;
  host->host_caps = cfg->flags;
  host->quirks    = cfg->quirks;
  host->verbose   = cfg->verbose;

  mmc = &host->mmc;
  mmc->priv = host;

  mmc->verbose  = host->verbose;

  host->version = sdhci_readw(host, SDHCI_HOST_VERSION);
  if (mmc->verbose) {
    printf("*** Host Controller Version Information ***\n");
    printf("  Name:           %s\n",      host->hw_name);
    printf("  Version:        %08x\n",    host->version);
    printf("  Vendor version: %u\n",     (host->version & SDHCI_VENDOR_VER_MASK) >> SDHCI_VENDOR_VER_SHIFT);
    printf("  Spec version:   %u.00\n", ((host->version & SDHCI_SPEC_VER_MASK)   >> SDHCI_SPEC_VER_SHIFT) + 1);
  }

  sprintf(mmc->name, "%s", SDHCI_DISK_DEVICE_BASE_NAME "A");
  mmc->name[sizeof(SDHCI_DISK_DEVICE_BASE_NAME)] += minor;
  mmc->send_cmd = sdhci_send_command;
  mmc->set_ios  = sdhci_set_ios;
  mmc->init     = sdhci_init;
  mmc->getcd    = NULL;
  mmc->getwp    = NULL;

  caps = sdhci_readl(host, SDHCI_CAPABILITIES);
  if (mmc->verbose) {
    static const char* const no  = "no";
    static const char* const yes = "yes";
    static const char* const KHz = "KHz";
    static const char* const MHz = "MHz";
    unsigned mask;

    printf("\n*** Host Controller Capabilities ***\n");
    printf("  Mask for %-17s "          "0x%08x\n", mmc->name, caps);
    printf("  Timeout Clock Frequency:   %u %s\n", (caps & SDHCI_TIMEOUT_CLK_MASK) >> SDHCI_TIMEOUT_CLK_SHIFT, (caps & SDHCI_TIMEOUT_CLK_UNIT) ? MHz : KHz);
    mask = (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300) ? SDHCI_CLOCK_V3_BASE_MASK : SDHCI_CLOCK_BASE_MASK;
    printf("  Base SD Clock Frequency:   %u %s\n", (caps & mask) >> SDHCI_CLOCK_BASE_SHIFT, MHz);
    printf("  Max Block Length:          %u\n", 1U << (9 + ((caps & SDHCI_MAX_BLOCK_MASK) >> SDHCI_MAX_BLOCK_SHIFT)));
    printf("  SDMA Support:              %s\n", (caps & SDHCI_CAN_DO_SDMA)  ? yes : no);
    printf("  ADMA1 Support:             %s\n", (caps & SDHCI_CAN_DO_ADMA1) ? yes : no);
    printf("  ADMA2 Support:             %s\n", (caps & SDHCI_CAN_DO_ADMA2) ? yes : no);
    printf("  8-bit Support:             %s\n", (caps & SDHCI_CAN_DO_8BIT)  ? yes : no);
    printf("  High Speed Support:        %s\n", (caps & SDHCI_CAN_DO_HISPD) ? yes : no);
    printf("  Suspend/Resume Support:    %s\n", (caps & SDHCI_CAN_DO_SUSP)  ? yes : no);
    printf("  3.3V Support:              %s\n", (caps & SDHCI_CAN_VDD_330)  ? yes : no);
    printf("  3.0V Support:              %s\n", (caps & SDHCI_CAN_VDD_300)  ? yes : no);
    printf("  1.8V Support:              %s\n", (caps & SDHCI_CAN_VDD_180)  ? yes : no);
    printf("  64-bit System Bus Support: %s\n", (caps & SDHCI_CAN_64BIT)    ? yes : no);
  }
#ifdef CONFIG_MMC_SDMA
  if (!(caps & SDHCI_CAN_DO_SDMA)) {
    RTEMS_SYSLOG("Your controller don't support sdma\n");
    return RTEMS_IO_ERROR;
  }
#endif

  if (cfg->max_clk)
    mmc->f_max = cfg->max_clk;
  else {
    if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300)
      mmc->f_max = (caps & SDHCI_CLOCK_V3_BASE_MASK) >> SDHCI_CLOCK_BASE_SHIFT;
    else
      mmc->f_max = (caps & SDHCI_CLOCK_BASE_MASK)    >> SDHCI_CLOCK_BASE_SHIFT;
    mmc->f_max *= 1000000;
  }
  if (mmc->f_max == 0) {
    RTEMS_SYSLOG("Hardware doesn't specify base clock frequency\n");
    return RTEMS_IO_ERROR;
  }

  if (cfg->min_clk)
    mmc->f_min = cfg->min_clk;
  else {
    if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300)
      mmc->f_min = mmc->f_max / SDHCI_MAX_DIV_SPEC_300;
    else
      mmc->f_min = mmc->f_max / SDHCI_MAX_DIV_SPEC_200;
  }
  if (host->verbose)
    printf("  Clock range:               %u - %u Hz\n",
           mmc->f_min, mmc->f_max);

  mmc->voltages = 0;
  if (caps & SDHCI_CAN_VDD_330)
    mmc->voltages |= MMC_VDD_32_33 | MMC_VDD_33_34;
  if (caps & SDHCI_CAN_VDD_300)
    mmc->voltages |= MMC_VDD_29_30 | MMC_VDD_30_31;
  if (caps & SDHCI_CAN_VDD_180)
    mmc->voltages |= MMC_VDD_165_195;

  if (host->quirks & SDHCI_QUIRK_BROKEN_VOLTAGE)
    mmc->voltages |= host->voltages;
  if (host->verbose)
    printf("  Voltage selection:         0x%08x\n", mmc->voltages);

  mmc->host_caps = MMC_MODE_HS | MMC_MODE_HS_52MHz | MMC_MODE_4BIT;
  if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300) {
    if (caps & SDHCI_CAN_DO_8BIT)
      mmc->host_caps |= MMC_MODE_8BIT;
  }
  if (host->host_caps)
    mmc->host_caps |= host->host_caps;
  if (host->verbose)
    printf("  Host capabilities:         0x%08x\n", mmc->host_caps);

  sdhci_reset(host, SDHCI_RESET_ALL);

  mmc_register(mmc, dev);

  return 0;
}
/** @} */

/**
 * SDHCI disk device driver initialization.
 *
 * @todo Memory clean up on error is really badly handled.
 *
 * @param major SDHCI disk major device number.
 * @param minor Minor device number, not applicable.
 * @param arg Initialization argument, not applicable.
 */
rtems_device_driver
rtems_sdhci_initialize(rtems_device_major_number major,
                       rtems_device_minor_number minor __attribute__((unused)),
                       void*                     arg   __attribute__((unused)))
{
  rtems_device_minor_number i;
  const rtems_sdhci_config* cfg   = rtems_sdhci_configuration;
  size_t                    cfgSz = rtems_sdhci_configuration_size;
  sdhci_host*               host;

  host = calloc(cfgSz, sizeof(*host));
  if (!host) {
    RTEMS_SYSLOG("sdhci_host malloc failed\n");
    return RTEMS_NO_MEMORY;
  }

  for (i = 0; i < cfgSz; ++i, ++cfg, ++host) {
    dev_t             dev = rtems_filesystem_make_dev_t(major, i);
    rtems_status_code sc  = add_sdhci(host, cfg, dev);

    if (sc != RTEMS_SUCCESSFUL) {
      RTEMS_SYSLOG("Initialize SDHCI failed\n");
      return sc;
    }
  }

  return RTEMS_SUCCESSFUL;
}

/** @} */
