/**
* @file      mmc.c
*
* @brief     Multi-Media Card driver for RTEMS based on the U-Boot code:
*
*
 * Copyright 2008, Freescale Semiconductor, Inc
 * Andy Fleming
 *
 * Based vaguely on the Linux code
 *
 * SPDX-License-Identifier:     GPL-2.0+
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
//#define MMC_PERF 1
//#define MMC_TRACE 1

#if 0
#define DEBUG                           /* Enables things in status-checks.h */
#endif

/* FIXME: How to set these defines? */
#if !defined(CONFIG_MMC_TRACE)
  #define CONFIG_MMC_TRACE 0
#endif
#if !defined(MMC_TRACE)
  #define MMC_TRACE 0
#endif
#if !defined(MMC_PERF)
  #define MMC_PERF 0
#endif

#include <errno.h>

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <rtems.h>
#include <rtems/libio.h>
#include <rtems/endian.h>
#include <rtems/status-checks.h>
#include <rtems/blkdev.h>
#include "rtems/diskdevs.h"


#include "sd/mmc.h"
#include "mmcImpl.h"

#define __be32_to_cpu ntohl

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define ROUND(a,b)		(((a) + (b) - 1) & ~((b) - 1))

#define ALIGN(x,a)              __ALIGN_MASK((x),(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))

#define ARCH_DMA_MINALIGN       32  /* The L1 Cache line size */

#define ALLOC_ALIGN_BUFFER(type, name, size, align)                     \
        char __##name[ROUND(size * sizeof(type), align) + (align - 1)]; \
                                                                        \
        type *name = (type *) ALIGN((uintptr_t)__##name, align)
#define ALLOC_CACHE_ALIGN_BUFFER(type, name, size)                      \
        ALLOC_ALIGN_BUFFER(type, name, size, ARCH_DMA_MINALIGN)

/* Set block count limit because of 16 bit register limit on some hardware*/
#ifndef CONFIG_SYS_MMC_MAX_BLK_COUNT
#define CONFIG_SYS_MMC_MAX_BLK_COUNT 65535
#endif


/* Time units */
static const int tunit[] = {
  1,
  10,
  100,
  1000,
  10000,
  100000,
  1000000,
  10000000,
};

/* frequency bases */
static const int fbase[] = {
  100000,
  1000000,
  10000000,
  100000000,
  0,                                    /* reserved */
  0,                                    /* reserved */
  0,                                    /* reserved */
  0,                                    /* reserved */
};

/* Multiplier values for TAAC and TRAN_SPEED.  Multiplied by 10 to be nice
 * to platforms without floating point.
 */
static const int multiplier[] = {
  0,	/* reserved */
  10,
  12,
  13,
  15,
  20,
  25,
  30,
  35,
  40,
  45,
  50,
  55,
  60,
  70,
  80,
};

/**
 * @name Card Specific Data Functions
 * @{
 */

static inline uint32_t mmc_access_time(const unsigned *csd)
{
  uint32_t taac = (csd[0] >> 16) & 0xff;
  return (multiplier[(taac >> 3) & 0xf] * tunit[taac & 0x7]) / 10;
}

static inline uint32_t mmc_trans_speed(const unsigned *csd)
{
  uint32_t tran_speed = csd[0] & 0xff;
  return (multiplier[(tran_speed >> 3) & 0xf] * fbase[tran_speed & 0x7]) / 10;
}

static inline uint32_t mmc_max_access_time(const unsigned *csd, unsigned transfer_speed)
{
  uint64_t ac = mmc_access_time(csd);
  uint32_t n  = ((csd[0] >> 8) & 0xff) * 100;
  ac = (ac * transfer_speed) / 8000000000ULL;
  return n + (uint32_t)ac;
}

/** @} */

int __board_mmc_getwp(struct mmc *mmc)
{
  return -1;
}

int board_mmc_getwp(struct mmc *mmc)
__attribute__ ((weak, alias("__board_mmc_getwp")));

int mmc_getwp(struct mmc *mmc)
{
  int wp;

  wp = board_mmc_getwp(mmc);

  if (wp < 0) {
    if (mmc->getwp)
      wp = mmc->getwp(mmc);
    else
      wp = 0;
  }

  return wp;
}

int __board_mmc_getcd(struct mmc *mmc) {
  return -1;
}

int board_mmc_getcd(struct mmc *mmc)
__attribute__ ((weak, alias("__board_mmc_getcd")));


static int mmc_send_cmd(struct mmc*      mmc,
                        struct mmc_cmd*  cmd,
			struct mmc_data* data)
{
  int ret;

#if CONFIG_MMC_TRACE
  printf("CMD_SEND: %2d\tARG: 0x%08X    ", cmd->cmdidx, cmd->cmdarg);
  ret = mmc->send_cmd(mmc, cmd, data);
  switch (cmd->resp_type) {
    case MMC_RSP_NONE:
      printf("MMC_RSP_NONE\n");
      break;
    case MMC_RSP_R1:
      printf("MMC_RSP_R1,5,6,7: 0x%08X\n", cmd->response[0]);
      break;
    case MMC_RSP_R1b:
      printf("MMC_RSP_R1b:      0x%08X\n", cmd->response[0]);
      break;
    case MMC_RSP_R2:
      printf("MMC_RSP_R2:       0x%08X\n"
             "\t\t\t\t\t\t     0x%08x\n"
             "\t\t\t\t\t\t     0x%08x\n"
             "\t\t\t\t\t\t     0x%08x\n",
             cmd->response[0], cmd->response[1], cmd->response[2], cmd->response[3]);
      break;
    case MMC_RSP_R3:
      printf("MMC_RSP_R3,4:     0x%08X\n", cmd->response[0]);
      break;
    default:
      printf("ERROR MMC rsp not supported\n");
      break;
  }
#else
  ret = mmc->send_cmd(mmc, cmd, data);
#endif
  return ret;
}

static int mmc_send_status(struct mmc* mmc, int timeout)
{
  struct mmc_cmd cmd;
  int err, retries = 5;
#if CONFIG_MMC_TRACE
  int status;
#endif

  cmd.cmdidx    = MMC_CMD_SEND_STATUS;
  cmd.resp_type = MMC_RSP_R1;
  if (!mmc_host_is_spi(mmc))
    cmd.cmdarg  = mmc->rca << 16;

  do {
    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (!err) {
      if ((cmd.response[0] & MMC_STATUS_RDY_FOR_DATA) &&
          (cmd.response[0] & MMC_STATUS_CURR_STATE) != MMC_STATE_PRG)
        break;
      else if (cmd.response[0] & MMC_STATUS_MASK) {
        RTEMS_SYSLOG("Status Error: 0x%08x\n", cmd.response[0]);
        return COMM_ERR;
      }
    } else if (--retries < 0)
      return err;

    udelay(1000);

  } while (timeout--);

#if CONFIG_MMC_TRACE
  status = (cmd.response[0] & MMC_STATUS_CURR_STATE) >> 9;
  printf("CURR STATE:%d\n", status);
#endif
  if (timeout <= 0) {
    RTEMS_SYSLOG("Timeout waiting card ready\n");
    return TIMEOUT;
  }

  return 0;
}

static int mmc_set_blocklen(struct mmc* mmc, int len)
{
  struct mmc_cmd cmd;

  cmd.cmdidx    = MMC_CMD_SET_BLOCKLEN;
  cmd.resp_type = MMC_RSP_R1;
  cmd.cmdarg    = len;

  return mmc_send_cmd(mmc, &cmd, NULL);
}

uint32_t
mmc_erase_blocks(struct mmc *mmc,
                 uint32_t   start,
                 uint32_t   blkcnt)
{
  struct mmc_cmd cmd;
  uint32_t       end;
  int            err, start_cmd, end_cmd;

  if (mmc->high_capacity)
    end = start + blkcnt - 1;
  else {
    end = (start + blkcnt - 1) * mmc->write_bl_len;
    start *= mmc->write_bl_len;
  }

  if (IS_SD(mmc)) {
    start_cmd = SD_CMD_ERASE_WR_BLK_START;
    end_cmd   = SD_CMD_ERASE_WR_BLK_END;
  } else {
    start_cmd = MMC_CMD_ERASE_GROUP_START;
    end_cmd   = MMC_CMD_ERASE_GROUP_END;
  }

  cmd.cmdidx    = start_cmd;
  cmd.cmdarg    = start;
  cmd.resp_type = MMC_RSP_R1;

  if ( (err = mmc_send_cmd(mmc, &cmd, NULL)) ) {
    RTEMS_SYSLOG("mmc_send_cmd(%d) failed\n", cmd.cmdidx);
    return err;
  }

  cmd.cmdidx = end_cmd;
  cmd.cmdarg = end;

  if ( (err = mmc_send_cmd(mmc, &cmd, NULL)) ) {
    RTEMS_SYSLOG("mmc_send_cmd(%d) failed\n", cmd.cmdidx);
    return err;
  }

  cmd.cmdidx    = MMC_CMD_ERASE;
  cmd.cmdarg    = SECURE_ERASE;
  cmd.resp_type = MMC_RSP_R1b;

  if ( (err = mmc_send_cmd(mmc, &cmd, NULL)) ) {
    RTEMS_SYSLOG("mmc_send_cmd(%d) failed\n", cmd.cmdidx);
    return err;
  }

  return 0;
}

static int
mmc_erase(struct mmc* mmc,
          uint32_t    start,
          uint32_t    blkcnt)
{
  int err = 0;
  uint32_t blk = 0, blk_r = 0;
  int timeout = 1000;

  if ((start % mmc->erase_grp_size) || (blkcnt % mmc->erase_grp_size))
    RTEMS_SYSLOG("\n\nCaution! Your device's Erase group is 0x%x\n"
                 "The erase range would be changed to 0x%lx~0x%lx\n\n",
                 mmc->erase_grp_size, start & ~(mmc->erase_grp_size - 1),
                 ((start + blkcnt + mmc->erase_grp_size)
                  & ~(mmc->erase_grp_size - 1)) - 1);

  while (blk < blkcnt) {
    blk_r = ((blkcnt - blk) > mmc->erase_grp_size) ?
      mmc->erase_grp_size : (blkcnt - blk);
    err = mmc_erase_blocks(mmc, start + blk, blk_r);
    if (err)
      break;

    blk += blk_r;

    /* Waiting for the ready status */
    if (mmc_send_status(mmc, timeout))
      return 0;
  }

  return blk;
}

/**
 * This primitive copies contiguous blocks from the supplied buffer to the
 * device.
 *
 * @param mmc The mmc control structure.
 * @param start The block number from which to start writing
 * @param blkcnt The number of contiguous blocks to write
 * @param src The destination buffer to read from
 * @return The number of blocks read
 */
uint32_t
mmc_write_blocks(struct mmc*    mmc,
                 uint32_t       start,
                 uint32_t       blkcnt,
                 const uint8_t* src)
{
  struct mmc_cmd  cmd;
  struct mmc_data data;
  int timeout = 1000;

  cmd.cmdidx     = (blkcnt == 1) ? MMC_CMD_WRITE_SINGLE_BLOCK
                                 : MMC_CMD_WRITE_MULTIPLE_BLOCK;
  cmd.cmdarg     = mmc->high_capacity ? start : start * mmc->write_bl_len;
  cmd.resp_type  = MMC_RSP_R1;

  data.src       = src;
  data.blocks    = blkcnt;
  data.blocksize = mmc->write_bl_len;
  data.flags     = MMC_DATA_WRITE;

  if (mmc_send_cmd(mmc, &cmd, &data)) {
    RTEMS_SYSLOG("mmc_send_cmd(%d) failed\n", cmd.cmdidx);
    return 0;
  }

  /* SPI multiblock writes terminate using a special token,
   * not a STOP_TRANSMISSION request.
   */
  if (!mmc_host_is_spi(mmc) && blkcnt > 1) {
    udelay(1000);
    cmd.cmdidx    = MMC_CMD_STOP_TRANSMISSION;
    cmd.cmdarg    = 0;
    cmd.resp_type = MMC_RSP_R1b;
    if (mmc_send_cmd(mmc, &cmd, NULL)) {
      RTEMS_SYSLOG("Failed to send stop cmd\n");
      return 0;
    }
  }

  /* Waiting for the ready status */
  if (mmc_send_status(mmc, timeout)) {
    RTEMS_SYSLOG("send_status timed out\n");
    return 0;
  }

  return blkcnt;
}

/**
 * MMC WRITE request handler. This primitive copies data from supplied buffer
 * to the device.
 *
 * @param mmc The mmc control structure.
 * @param req Pointer to the WRITE block device request info.
 * @param lba The size of the device in blocks.
 * @return 0 No error.
 * @return EIO Attempt to write past the end of the device, invalid block size,
 *         communication failures.
 */
static int
mmc_write(rtems_disk_device*    dd,
          rtems_blkdev_request* req)
{
  struct mmc*             mmc         = rtems_disk_get_driver_data(dd);
  rtems_blkdev_bnum       lba         = rtems_disk_get_block_count(dd);
  rtems_blkdev_sg_buffer* bufs        = req->bufs;
  uint32_t                bufnum      = req->bufnum;
  uint32_t                start       = bufs->block;
  const uint8_t*          src         = bufs->buffer;
  uint32_t                blocks_todo = 0;

#if MMC_PERF
  uint32_t blktot = 0;
  uint64_t t0     = get_timer(0);
#endif

  if (!IS_SD(mmc))
    if (mmc_set_blocklen(mmc, mmc->write_bl_len))
      return EIO;

  /* Aggragate the write request into as few write commands as possible */
  do {
    uint32_t block  = bufs->block;
    uint8_t* buffer = bufs->buffer;
    uint32_t length = bufs->length;
    uint32_t blkcnt = (length + mmc->write_bl_len - 1) / mmc->write_bl_len;

#if MMC_TRACE
    printk("%s: blk = %08lx, len = %08lx, buf = %08x, cnt = %08lx, todo = %08x\n",
           __func__, block, length, (uintptr_t)buffer, blkcnt, blocks_todo);
#endif

    blocks_todo += blkcnt;
    bufs        += 1;

    if ((--bufnum == 0) || ((block  + blkcnt != bufs->block) ||
                            (buffer + length != bufs->buffer)))
    {
      if ((start + blocks_todo) > lba) {
        RTEMS_SYSLOG("Block number 0x%08lx exceeds max(0x%08lx)\n",
                     start + blocks_todo, lba);
        return EIO;
      }

#if MMC_PERF
      blktot += blocks_todo;
#endif

      do {
        uint32_t cnt = (blocks_todo > mmc->b_max) ?  mmc->b_max : blocks_todo;

        if (mmc_write_blocks(mmc, start, cnt, src) != cnt)
          return EIO;

        blocks_todo -= cnt;
        start       += cnt;
        src         += cnt * mmc->write_bl_len;
      } while (blocks_todo > 0);

      start = bufs->block;
      src   = bufs->buffer;
    }
  } while (bufnum);

#if MMC_PERF
  uint64_t dT    = get_timer(t0);
  uint64_t usecs = get_usecs(dT);
  uint64_t bytes = blktot * mmc->write_bl_len;
  uint32_t rate  = bytes / usecs;
  printf("%s: dT = 0x%llx ticks = %llu usecs, blocks = %lu, bytes = %llu, rate = %lu bytes/usec\n",
         __func__, dT, usecs, blktot, bytes, rate);
#endif

  return 0;
}

/**
 * This primitive copies contiguous blocks from the device to the supplied
 * buffer.
 *
 * @param mmc The mmc control structure.
 * @param start The block number from which to start reading
 * @param blkcnt The number of contiguous blocks to read
 * @param dst The destination buffer to fill
 * @return The number of blocks read
 */
uint32_t
mmc_read_blocks(struct mmc* mmc,
                uint32_t    start,
                uint32_t    blkcnt,
                uint8_t*    dst)
{
  struct mmc_cmd  cmd;
  struct mmc_data data;

  cmd.cmdidx     = (blkcnt == 1) ? MMC_CMD_READ_SINGLE_BLOCK
                                 : MMC_CMD_READ_MULTIPLE_BLOCK;
  cmd.cmdarg     = mmc->high_capacity ? start : start * mmc->read_bl_len;
  cmd.resp_type  = MMC_RSP_R1;

  data.dst       = dst;
  data.blocks    = blkcnt;
  data.blocksize = mmc->read_bl_len;
  data.flags     = MMC_DATA_READ;

  if (mmc_send_cmd(mmc, &cmd, &data)) {
    RTEMS_SYSLOG("mmc_send_cmd(%d) failed\n", cmd.cmdidx);
    return 0;
  }

  if (blkcnt > 1) {
    cmd.cmdidx    = MMC_CMD_STOP_TRANSMISSION;
    cmd.cmdarg    = 0;
    cmd.resp_type = MMC_RSP_R1b;
    if (mmc_send_cmd(mmc, &cmd, NULL)) {
      RTEMS_SYSLOG("Failed to send stop cmd\n");
      return 0;
    }
  }

  return blkcnt;
}

/**
 * MMC READ request handler. This primitive copies data from the device to the
 * supplied buffer.
 *
 * @param mmc The mmc control structure.
 * @param req Pointer to the READ block device request info.
 * @param lba The size of the device in blocks.
 * @return 0 No error.
 * @return EIO Attempt to write past the end of the device, invalid block size,
 *         communication failures.
 */
static int
mmc_read(rtems_disk_device*    dd,
         rtems_blkdev_request* req)
{
  struct mmc*             mmc         = rtems_disk_get_driver_data(dd);
  rtems_blkdev_bnum       lba         = rtems_disk_get_block_count(dd);
  rtems_blkdev_sg_buffer* bufs        = req->bufs;
  uint32_t                bufnum      = req->bufnum;
  uint32_t                start       = bufs->block;
  uint8_t*                dst         = bufs->buffer;
  uint32_t                blocks_todo = 0;

#if MMC_PERF
  uint32_t blktot = 0;
  uint64_t t0     = get_timer(0);
#endif

  if (!IS_SD(mmc))
    if (mmc_set_blocklen(mmc, mmc->read_bl_len))
      return EIO;

  /* Aggragate the read request into as few read commands as possible */
  do {
    uint32_t block  = bufs->block;
    uint8_t* buffer = bufs->buffer;
    uint32_t length = bufs->length;
    uint32_t blkcnt = (length + mmc->read_bl_len - 1) / mmc->read_bl_len;

#if MMC_TRACE
    printk("%s: bn = %08lx, blk = %08lx, len = %08lx, buf = %08x, cnt = %08lx, todo = %08x\n",
           __func__, bufnum, block, length, (uintptr_t)buffer, blkcnt, blocks_todo);
#endif

    blocks_todo += blkcnt;
    bufs        += 1;

    if ((--bufnum == 0) || ((block  + blkcnt != bufs->block) ||
                            (buffer + length != bufs->buffer))) {
      if ((start + blocks_todo) > lba) {
        RTEMS_SYSLOG("Block number 0x%08lx exceeds max(0x%08lx)\n",
                     start + blocks_todo, lba);
        return EIO;
      }

#if MMC_PERF
      blktot += blocks_todo;
#endif

      do {
        uint32_t cnt = (blocks_todo > mmc->b_max) ?  mmc->b_max : blocks_todo;

        if (mmc_read_blocks(mmc, start, cnt, dst) != cnt)
          return EIO;

        blocks_todo -= cnt;
        start       += cnt;
        dst         += cnt * mmc->read_bl_len;
      } while (blocks_todo > 0);

      start = bufs->block;
      dst   = bufs->buffer;
    }
  } while (bufnum);

#if MMC_PERF
  uint64_t dT    = get_timer(t0);
  uint64_t usecs = get_usecs(dT);
  uint64_t bytes = blktot * mmc->read_bl_len;
  uint32_t rate  = bytes / usecs;
  printf("%s: dT = 0x%llx ticks = %llu usecs, blocks = %lu, bytes = %llu, rate = %lu bytes/usec\n",
         __func__, dT, usecs, blktot, bytes, rate);
#endif

  return 0;
}

static int mmc_go_idle(struct mmc* mmc)
{
  struct mmc_cmd cmd;
  int err;

  udelay(1000);

  cmd.cmdidx    = MMC_CMD_GO_IDLE_STATE;
  cmd.cmdarg    = 0;
  cmd.resp_type = MMC_RSP_NONE;

  err = mmc_send_cmd(mmc, &cmd, NULL);

  if (err)
    return err;

  udelay(2000);

  return 0;
}

static int sd_send_op_cond(struct mmc* mmc)
{
  int timeout = 1000;
  int err;
  struct mmc_cmd cmd;

  do {
    cmd.cmdidx    = MMC_CMD_APP_CMD;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg    = 0;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err)
      return err;

    cmd.cmdidx    = SD_CMD_APP_SEND_OP_COND;
    cmd.resp_type = MMC_RSP_R3;

    /*
     * Most cards do not answer if some reserved bits
     * in the ocr are set. However, Some controllers
     * can set bit 7 (reserved for low voltages), but
     * how to manage low voltages SD card is not yet
     * specified.
     */
    cmd.cmdarg = mmc_host_is_spi(mmc) ? 0 : (mmc->voltages & 0xff8000);

    if (mmc->version == SD_VERSION_2)
      cmd.cmdarg |= OCR_HCS;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err)
      return err;

    udelay(1000);
  } while ((!(cmd.response[0] & OCR_BUSY)) && timeout--);

  if (timeout <= 0)
    return UNUSABLE_ERR;

  if (mmc->version != SD_VERSION_2)
    mmc->version = SD_VERSION_1_0;

  if (mmc_host_is_spi(mmc)) { /* read OCR for spi */
    cmd.cmdidx    = MMC_CMD_SPI_READ_OCR;
    cmd.resp_type = MMC_RSP_R3;
    cmd.cmdarg    = 0;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err)
      return err;
  }

  mmc->ocr = cmd.response[0];

  mmc->high_capacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);
  mmc->rca           = 0;

  return 0;
}

/* We pass in the cmd since otherwise the init seems to fail */
static int mmc_send_op_cond_iter(struct mmc*     mmc,
                                 struct mmc_cmd* cmd,
                                 int             use_arg)
{
  int err;

  cmd->cmdidx    = MMC_CMD_SEND_OP_COND;
  cmd->resp_type = MMC_RSP_R3;
  cmd->cmdarg    = 0;
  if (use_arg && !mmc_host_is_spi(mmc)) {
    cmd->cmdarg =
      (mmc->voltages &
       (mmc->op_cond_response & OCR_VOLTAGE_MASK)) |
      (mmc->op_cond_response & OCR_ACCESS_MODE);

    if (mmc->host_caps & MMC_MODE_HC)
      cmd->cmdarg |= OCR_HCS;
  }
  err = mmc_send_cmd(mmc, cmd, NULL);
  if (err)
    return err;
  mmc->op_cond_response = cmd->response[0];
  return 0;
}

int mmc_send_op_cond(struct mmc *mmc)
{
  struct mmc_cmd cmd;
  int err, i;

  /* Some cards seem to need this */
  mmc_go_idle(mmc);

  /* Asking to the card its capabilities */
  mmc->op_cond_pending = 1;
  for (i = 0; i < 2; i++) {
    err = mmc_send_op_cond_iter(mmc, &cmd, i != 0);
    if (err)
      return err;

    /* exit if not busy (flag seems to be inverted) */
    if (mmc->op_cond_response & OCR_BUSY)
      return 0;
  }
  return IN_PROGRESS;
}

int mmc_complete_op_cond(struct mmc *mmc)
{
  struct mmc_cmd cmd;
  int      timeout = 1000;
  uint32_t start;
  int      err;

  mmc->op_cond_pending = 0;
  start = get_timer(0);
  do {
    err = mmc_send_op_cond_iter(mmc, &cmd, 1);
    if (err)
      return err;
    if (get_timer(start) > timeout)
      return UNUSABLE_ERR;
    udelay(100);
  } while (!(mmc->op_cond_response & OCR_BUSY));

  if (mmc_host_is_spi(mmc)) { /* read OCR for spi */
    cmd.cmdidx    = MMC_CMD_SPI_READ_OCR;
    cmd.resp_type = MMC_RSP_R3;
    cmd.cmdarg    = 0;

    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err)
      return err;
  }

  mmc->version = MMC_VERSION_UNKNOWN;
  mmc->ocr     = cmd.response[0];

  mmc->high_capacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);
  mmc->rca           = 0;

  return 0;
}

static int mmc_send_ext_csd(struct mmc* mmc, uint8_t* ext_csd)
{
  struct mmc_cmd cmd;
  struct mmc_data data;
  int err;

  /* Get the Card Status Register */
  cmd.cmdidx     = MMC_CMD_SEND_EXT_CSD;
  cmd.resp_type  = MMC_RSP_R1;
  cmd.cmdarg     = 0;

  data.dst       = (uint8_t *)ext_csd;
  data.blocks    = 1;
  data.blocksize = MMC_MAX_BLOCK_LEN;
  data.flags     = MMC_DATA_READ;

  err = mmc_send_cmd(mmc, &cmd, &data);

  return err;
}


static int mmc_switch(struct mmc* mmc, uint8_t set, uint8_t index, uint8_t value)
{
  struct mmc_cmd cmd;
  int timeout = 1000;
  int ret;

  cmd.cmdidx    = MMC_CMD_SWITCH;
  cmd.resp_type = MMC_RSP_R1b;
  cmd.cmdarg    = (MMC_SWITCH_MODE_WRITE_BYTE << 24) | (index << 16) | (value << 8);

  ret = mmc_send_cmd(mmc, &cmd, NULL);

  /* Waiting for the ready status */
  if (!ret)
    ret = mmc_send_status(mmc, timeout);

  return ret;

}

static int mmc_change_freq(struct mmc* mmc)
{
  ALLOC_CACHE_ALIGN_BUFFER(uint8_t, ext_csd, MMC_MAX_BLOCK_LEN);
  char cardtype;
  int err;

  mmc->card_caps = 0;

  if (mmc_host_is_spi(mmc))
    return 0;

  /* Only version 4 supports high-speed */
  if (mmc->version < MMC_VERSION_4)
    return 0;

  err = mmc_send_ext_csd(mmc, ext_csd);

  if (err)
    return err;

  cardtype = ext_csd[EXT_CSD_CARD_TYPE] & 0xf;

  err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING, 1);

  if (err)
    return err;

  /* Now check to see that it worked */
  err = mmc_send_ext_csd(mmc, ext_csd);

  if (err)
    return err;

  /* No high-speed support */
  if (!ext_csd[EXT_CSD_HS_TIMING])
    return 0;

  /* High Speed is set, there are two types: 52MHz and 26MHz */
  if (cardtype & MMC_HS_52MHZ)
    mmc->card_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;
  else
    mmc->card_caps |= MMC_MODE_HS;

  return 0;
}

static int mmc_set_capacity(struct mmc *mmc, int part_num)
{
  switch (part_num) {
    case 0:
      mmc->capacity = mmc->capacity_user;
      break;
    case 1:
    case 2:
      mmc->capacity = mmc->capacity_boot;
      break;
    case 3:
      mmc->capacity = mmc->capacity_rpmb;
      break;
    case 4:
    case 5:
    case 6:
    case 7:
      mmc->capacity = mmc->capacity_gp[part_num - 4];
      break;
    default:
      return -1;
  }

  return 0;
}

int mmc_getcd(struct mmc* mmc)
{
  int cd;

  cd = board_mmc_getcd(mmc);

  if (cd < 0) {
    if (mmc->getcd)
      cd = mmc->getcd(mmc);
    else
      cd = 1;
  }

  return cd;
}

static int sd_switch(struct mmc* mmc, int mode, int group, uint8_t value, uint8_t* resp)
{
  struct mmc_cmd  cmd;
  struct mmc_data data;

  /* Switch the frequency */
  cmd.cmdidx     = SD_CMD_SWITCH_FUNC;
  cmd.resp_type  = MMC_RSP_R1;
  cmd.cmdarg     = (mode << 31) | 0xffffff;
  cmd.cmdarg    &= ~(0xf << (group * 4));
  cmd.cmdarg    |= value << (group * 4);

  data.dst       = (uint8_t *)resp;
  data.blocksize = 64;
  data.blocks    = 1;
  data.flags     = MMC_DATA_READ;

  return mmc_send_cmd(mmc, &cmd, &data);
}


static int sd_change_freq(struct mmc* mmc)
{
  int err;
  struct mmc_cmd cmd;
  ALLOC_CACHE_ALIGN_BUFFER(unsigned, scr, 2);
  ALLOC_CACHE_ALIGN_BUFFER(unsigned, switch_status, 16);
  struct mmc_data data;
  int timeout;
  int retries = 5;

  mmc->card_caps = 0;

  if (mmc_host_is_spi(mmc))
    return 0;

  /* Read the SCR to find out if this card supports higher speeds */
retry_scr:
  cmd.cmdidx    = MMC_CMD_APP_CMD;
  cmd.resp_type = MMC_RSP_R1;
  cmd.cmdarg    = mmc->rca << 16;

  err = mmc_send_cmd(mmc, &cmd, NULL);
  if (err) {
    RTEMS_SYSLOG("mmc_send_cmd(%d) failed\n", cmd.cmdidx);
    return err;
  }

  cmd.cmdidx     = SD_CMD_APP_SEND_SCR;
  cmd.resp_type  = MMC_RSP_R1;
  cmd.cmdarg     = 0;

  timeout = 3;

  //retry_scr:
  data.dst       = (uint8_t *)scr;
  data.blocksize = 8;
  data.blocks    = 1;
  data.flags     = MMC_DATA_READ;

  err = mmc_send_cmd(mmc, &cmd, &data);
  if (err) {
    if (timeout--)
      goto retry_scr;

    RTEMS_SYSLOG("mmc_send_cmd(%d) timed out\n", cmd.cmdidx);
    return err;
  }

  mmc->scr[0] = __be32_to_cpu(scr[0]);
  mmc->scr[1] = __be32_to_cpu(scr[1]);

  if ((mmc->scr[0] >> 28) != 0) {
    if (--retries)
      goto retry_scr;
    RTEMS_SYSLOG("Suspicious SCR structure version #: %08x %08x, rsp = %08x",
                 mmc->scr[0], mmc->scr[1], cmd.response[0]);
  }

  switch ((mmc->scr[0] >> 24) & 0xf) {
    case 0:
      mmc->version = SD_VERSION_1_0;
      break;
    case 1:
      mmc->version = SD_VERSION_1_10;
      break;
    case 2:
      mmc->version = SD_VERSION_2;
      if ((mmc->scr[0] >> 15) & 0x1)
        mmc->version = SD_VERSION_3;
      break;
    default:
      mmc->version = SD_VERSION_1_0;
      break;
  }

  if (mmc->scr[0] & SD_DATA_4BIT)
    mmc->card_caps |= MMC_MODE_4BIT;

  /* Version 1.0 doesn't support switching */
  if (mmc->version == SD_VERSION_1_0)
    return 0;

  udelay(100);

  timeout = 4;
  while (timeout--) {
    err = sd_switch(mmc, SD_SWITCH_CHECK, 0, 1, (uint8_t *)switch_status);
    if (err) {
      RTEMS_SYSLOG("1st sd_switch() failed\n");
      return err;
    }

    /* The high-speed function is busy.  Try again */
    if (!(__be32_to_cpu(switch_status[7]) & SD_HIGHSPEED_BUSY))
      break;
  }

  /* If high-speed isn't supported, we return */
  if (!(__be32_to_cpu(switch_status[3]) & SD_HIGHSPEED_SUPPORTED))
    return 0;

  /*
   * If the host doesn't support SD_HIGHSPEED, do not switch card to
   * HIGHSPEED mode even if the card support SD_HIGHSPEED.
   * This can avoid further problems when the card runs in different
   * mode between the host.
   */
  if (!((mmc->host_caps & MMC_MODE_HS_52MHz) && (mmc->host_caps & MMC_MODE_HS)))
    return 0;

  udelay(100);

  err = sd_switch(mmc, SD_SWITCH_SWITCH, 0, 1, (uint8_t *)switch_status);
  if (err) {
    RTEMS_SYSLOG("2nd sd_switch() failed\n");
    return err;
  }

  if ((__be32_to_cpu(switch_status[4]) & 0x0f000000) == 0x01000000)
    mmc->card_caps |= MMC_MODE_HS;

  return 0;
}

static void mmc_set_ios(struct mmc* mmc)
{
  mmc->set_ios(mmc);
}

void mmc_set_clock(struct mmc* mmc, unsigned clock)
{
  if (clock > mmc->f_max)
    clock = mmc->f_max;

  if (clock < mmc->f_min)
    clock = mmc->f_min;

  mmc->clock = clock;

  mmc_set_ios(mmc);
}

static void mmc_set_bus_width(struct mmc* mmc, unsigned width)
{
  mmc->bus_width = width;

  mmc_set_ios(mmc);
}

static int mmc_startup(struct mmc* mmc)
{
  int err, i;
  uint64_t cmult, csize, capacity;
  struct mmc_cmd cmd;
  ALLOC_CACHE_ALIGN_BUFFER(uint8_t, ext_csd, MMC_MAX_BLOCK_LEN);
  ALLOC_CACHE_ALIGN_BUFFER(uint8_t, test_csd, MMC_MAX_BLOCK_LEN);
  int timeout = 1000;

#ifdef CONFIG_MMC_SPI_CRC_ON
  if (mmc_host_is_spi(mmc)) { /* enable CRC check for spi */
    cmd.cmdidx    = MMC_CMD_SPI_CRC_ON_OFF;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg    = 1;

    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err)
      return err;
  }
#endif

  /* Put the Card in Identify Mode */
  cmd.cmdidx    = mmc_host_is_spi(mmc) ? MMC_CMD_SEND_CID
                                       : MMC_CMD_ALL_SEND_CID; /* unsupported in spi */
  cmd.resp_type = MMC_RSP_R2;
  cmd.cmdarg    = 0;

  err = mmc_send_cmd(mmc, &cmd, NULL);
  if (err) {
    RTEMS_SYSLOG("mmc_send_cmd(%d) failed\n", cmd.cmdidx);
    return err;
  }

  memcpy(mmc->cid, cmd.response, 16);

  /*
   * For MMC cards, set the Relative Address.
   * For SD cards, get the Relatvie Address.
   * This also puts the cards into Standby State
   */
  if (!mmc_host_is_spi(mmc)) { /* cmd not supported in spi */
    cmd.cmdidx    = SD_CMD_SEND_RELATIVE_ADDR;
    cmd.cmdarg    = mmc->rca << 16;
    cmd.resp_type = MMC_RSP_R6;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err) {
      RTEMS_SYSLOG("mmc_send_cmd(%d) failed\n", cmd.cmdidx);
      return err;
    }

    if (IS_SD(mmc))
      mmc->rca = (cmd.response[0] >> 16) & 0xffff;
  }

  /* Get the Card-Specific Data */
  cmd.cmdidx    = MMC_CMD_SEND_CSD;
  cmd.resp_type = MMC_RSP_R2;
  cmd.cmdarg    = mmc->rca << 16;

  err = mmc_send_cmd(mmc, &cmd, NULL);

  /* Waiting for the ready status */
  mmc_send_status(mmc, timeout);

  if (err) {
    RTEMS_SYSLOG("mmc_send_cmd(%d) failed\n", cmd.cmdidx);
    return err;
  }

  mmc->csd[0] = cmd.response[0];
  mmc->csd[1] = cmd.response[1];
  mmc->csd[2] = cmd.response[2];
  mmc->csd[3] = cmd.response[3];

  if (mmc->version == MMC_VERSION_UNKNOWN) {
    int version = (cmd.response[0] >> 26) & 0xf;

    switch (version) {
      case 0:
        mmc->version = MMC_VERSION_1_2;
        break;
      case 1:
        mmc->version = MMC_VERSION_1_4;
        break;
      case 2:
        mmc->version = MMC_VERSION_2_2;
        break;
      case 3:
        mmc->version = MMC_VERSION_3;
        break;
      case 4:
        mmc->version = MMC_VERSION_4;
        break;
      default:
        mmc->version = MMC_VERSION_1_2;
        break;
    }
  }

  mmc->tran_speed   = mmc_trans_speed(cmd.response);
  //printk("%s: tran_speed = %08x\n", __func__, mmc->tran_speed);

  mmc->read_bl_len  = 1 << ((cmd.response[1] >> 16) & 0xf);
  mmc->write_bl_len = IS_SD(mmc) ? mmc->read_bl_len
                                 : (1 << ((cmd.response[3] >> 22) & 0xf));

  if (mmc->high_capacity) {
    csize = (mmc->csd[1] & 0x3f) << 16 | (mmc->csd[2] & 0xffff0000) >> 16;
    cmult = 8;
  } else {
    csize = (mmc->csd[1] & 0x3ff) << 2 | (mmc->csd[2] & 0xc0000000) >> 30;
    cmult = (mmc->csd[2] & 0x00038000) >> 15;
  }

  mmc->capacity_user = ((csize + 1) << (cmult + 2)) * mmc->read_bl_len;
  mmc->capacity_boot = 0;
  mmc->capacity_rpmb = 0;
  for (i = 0; i < 4; i++)
    mmc->capacity_gp[i] = 0;

  if (mmc->read_bl_len > MMC_MAX_BLOCK_LEN)
    mmc->read_bl_len = MMC_MAX_BLOCK_LEN;

  if (mmc->write_bl_len > MMC_MAX_BLOCK_LEN)
    mmc->write_bl_len = MMC_MAX_BLOCK_LEN;

  /* Select the card, and put it into Transfer Mode */
  if (!mmc_host_is_spi(mmc)) { /* cmd not supported in spi */
    cmd.cmdidx    = MMC_CMD_SELECT_CARD;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg    = mmc->rca << 16;

    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err) {
      RTEMS_SYSLOG("mmc_send_cmd(%d) failed\n", cmd.cmdidx);
      return err;
    }
  }

  /*
   * For SD, its erase group is always one sector
   */
  mmc->erase_grp_size = 1;
  mmc->part_config = MMCPART_NOAVAILABLE;
  if (!IS_SD(mmc) && (mmc->version >= MMC_VERSION_4)) {
    /* check  ext_csd version and capacity */
    err = mmc_send_ext_csd(mmc, ext_csd);
    if (!err && (ext_csd[EXT_CSD_REV] >= 2)) {
      /*
       * According to the JEDEC Standard, the value of
       * ext_csd's capacity is valid if the value is more
       * than 2GB
       */
      capacity = (ext_csd[EXT_CSD_SEC_CNT    ] <<  0 |
                  ext_csd[EXT_CSD_SEC_CNT + 1] <<  8 |
                  ext_csd[EXT_CSD_SEC_CNT + 2] << 16 |
                  ext_csd[EXT_CSD_SEC_CNT + 3] << 24) * MMC_MAX_BLOCK_LEN;
      if ((capacity >> 20) > 2 * 1024)
        mmc->capacity_user = capacity;
    }

    switch (ext_csd[EXT_CSD_REV]) {
      case 1:
        mmc->version = MMC_VERSION_4_1;
        break;
      case 2:
        mmc->version = MMC_VERSION_4_2;
        break;
      case 3:
        mmc->version = MMC_VERSION_4_3;
        break;
      case 5:
        mmc->version = MMC_VERSION_4_41;
        break;
      case 6:
        mmc->version = MMC_VERSION_4_5;
        break;
    }

    /*
     * Check whether GROUP_DEF is set, if yes, read out
     * group size from ext_csd directly, or calculate
     * the group size from the csd value.
     */
    if (ext_csd[EXT_CSD_ERASE_GROUP_DEF])
      mmc->erase_grp_size =
        ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * MMC_MAX_BLOCK_LEN * 1024;
    else {
      int erase_gsz, erase_gmul;
      erase_gsz  = (mmc->csd[2] & 0x00007c00) >> 10;
      erase_gmul = (mmc->csd[2] & 0x000003e0) >>  5;
      mmc->erase_grp_size = (erase_gsz + 1) * (erase_gmul + 1);
    }

    /* store the partition info of emmc */
    if ((ext_csd[EXT_CSD_PARTITIONING_SUPPORT] & PART_SUPPORT) ||
        ext_csd[EXT_CSD_BOOT_MULT])
      mmc->part_config = ext_csd[EXT_CSD_PART_CONF];

    mmc->capacity_boot = ext_csd[EXT_CSD_BOOT_MULT] << 17;

    mmc->capacity_rpmb = ext_csd[EXT_CSD_RPMB_MULT] << 17;

    for (i = 0; i < 4; i++) {
      int idx = EXT_CSD_GP_SIZE_MULT + i * 3;
      mmc->capacity_gp[i] = ((ext_csd[idx + 2] << 16) +
                             (ext_csd[idx + 1] << 8)  +
                             (ext_csd[idx    ]     ));
      mmc->capacity_gp[i] *= ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE];
      mmc->capacity_gp[i] *= ext_csd[EXT_CSD_HC_WP_GRP_SIZE];
    }
  }

  err = mmc_set_capacity(mmc, mmc->part_num);
  if (err) {
    RTEMS_SYSLOG("mmc_set_capacity() failed\n");
    return err;
  }

  err = IS_SD(mmc) ? sd_change_freq(mmc) : mmc_change_freq(mmc);
  if (err) {
    RTEMS_SYSLOG("%s_change_freq() failed\n", IS_SD(mmc) ? "sd" : "mmc");
    return err;
  }

  /* Restrict card's capabilities by what the host can do */
  //printk("%s: card_caps = %08x, host_caps = %08x\n", __func__, mmc->card_caps, mmc->host_caps);
  mmc->card_caps &= mmc->host_caps;

  if (IS_SD(mmc)) {
    if (mmc->card_caps & MMC_MODE_4BIT) {
      cmd.cmdidx    = MMC_CMD_APP_CMD;
      cmd.resp_type = MMC_RSP_R1;
      cmd.cmdarg    = mmc->rca << 16;

      err = mmc_send_cmd(mmc, &cmd, NULL);
      if (err) {
        RTEMS_SYSLOG("mmc_send_cmd(%d) failed\n", cmd.cmdidx);
        return err;
      }

      cmd.cmdidx    = SD_CMD_APP_SET_BUS_WIDTH;
      cmd.resp_type = MMC_RSP_R1;
      cmd.cmdarg    = 2;

      err = mmc_send_cmd(mmc, &cmd, NULL);
      if (err) {
        RTEMS_SYSLOG("mmc_send_cmd(%d) failed\n", cmd.cmdidx);
        return err;
      }

      mmc_set_bus_width(mmc, 4);
    }

    //printk("%s: SD: HS = %u, 52 MHz = %u\n", __func__, mmc->card_caps & MMC_MODE_HS, mmc->card_caps & MMC_MODE_HS_52MHz);
    mmc->tran_speed = (mmc->card_caps & MMC_MODE_HS) ? 50000000 : 25000000;

  } else {
    uint32_t idx;

    /* An array of possible bus widths in order of preference */
    static unsigned ext_csd_bits[] = {
      EXT_CSD_BUS_WIDTH_8,
      EXT_CSD_BUS_WIDTH_4,
      EXT_CSD_BUS_WIDTH_1,
    };

    /* An array to map CSD bus widths to host cap bits */
    static unsigned ext_to_hostcaps[] = {
      [EXT_CSD_BUS_WIDTH_4] = MMC_MODE_4BIT,
      [EXT_CSD_BUS_WIDTH_8] = MMC_MODE_8BIT,
    };

    /* An array to map chosen bus width to an integer */
    static unsigned widths[] = {
      8, 4, 1,
    };

    for (idx=0; idx < ARRAY_SIZE(ext_csd_bits); idx++) {
      unsigned extw = ext_csd_bits[idx];

      /*
       * Check to make sure the controller supports
       * this bus width, if it's more than 1
       */
      if (extw != EXT_CSD_BUS_WIDTH_1 &&
          !(mmc->host_caps & ext_to_hostcaps[extw]))
        continue;

      err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_BUS_WIDTH, extw);
      if (err)
        continue;

      mmc_set_bus_width(mmc, widths[idx]);

      err = mmc_send_ext_csd(mmc, test_csd);
      if (!err && ext_csd[EXT_CSD_PARTITIONING_SUPPORT] \
          == test_csd[EXT_CSD_PARTITIONING_SUPPORT]
          && ext_csd[EXT_CSD_ERASE_GROUP_DEF] \
          == test_csd[EXT_CSD_ERASE_GROUP_DEF] \
          && ext_csd[EXT_CSD_REV] \
          == test_csd[EXT_CSD_REV]
          && ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] \
          == test_csd[EXT_CSD_HC_ERASE_GRP_SIZE]
          && memcmp(&ext_csd[EXT_CSD_SEC_CNT], \
                    &test_csd[EXT_CSD_SEC_CNT], 4) == 0) {

        mmc->card_caps |= ext_to_hostcaps[extw];
        break;
      }
    }

    //printk("%s: !SD: HS = %u, 52 MHz = %u\n", __func__, mmc->card_caps & MMC_MODE_HS, mmc->card_caps & MMC_MODE_HS_52MHz);
    if (mmc->card_caps & MMC_MODE_HS) {
      mmc->tran_speed = (mmc->card_caps & MMC_MODE_HS_52MHz) ? 52000000
                                                             : 26000000;
    }
  }

  mmc_set_clock(mmc, mmc->tran_speed);

  if (IS_SD(mmc)) {
    if (mmc->read_bl_len != mmc->write_bl_len) {
      RTEMS_SYSLOG("Read block length (%08x) != Write block length (%08x)\n",
                   mmc->read_bl_len, mmc->write_bl_len);
      return -1;
    }
    err = mmc_set_blocklen(mmc, mmc->read_bl_len);
    if (err) {
      RTEMS_SYSLOG("mmc_send_cmd(%d) failed\n", cmd.cmdidx);
      return err;
    }
  }

  return 0;
}

static int mmc_send_if_cond(struct mmc* mmc)
{
  struct mmc_cmd cmd;
  int err;

  cmd.cmdidx    = SD_CMD_SEND_IF_COND;
  /* We set the bit if the host supports voltages between 2.7 and 3.6 V */
  cmd.cmdarg    = ((mmc->voltages & 0xff8000) != 0) << 8 | 0xaa;
  cmd.resp_type = MMC_RSP_R7;

  err = mmc_send_cmd(mmc, &cmd, NULL);
  if (err)
    return err;

  if ((cmd.response[0] & 0xff) != 0xaa)
    return UNUSABLE_ERR;
  else
    mmc->version = SD_VERSION_2;

  return 0;
}


static void mmc_print_cid(struct mmc* mmc)
{
  /* Card Identification */
  printf("\n*** Card Identification ***\n");
  printf("  Mfg ID:                   %02x\n",        mmc->cid[0] >> 24  & 0xff);
  printf("  OEM:                      %c%c\n",       (mmc->cid[0] >> 16) & 0xff,
                                                     (mmc->cid[0] >>  8) & 0xff);
  printf("  Name:                     %c%c%c%c%c\n",  mmc->cid[0]        & 0xff,
                                                     (mmc->cid[1] >> 24) & 0xff,
                                                     (mmc->cid[1] >> 16) & 0xff,
                                                     (mmc->cid[1] >>  8) & 0xff,
                                                      mmc->cid[1]        & 0xff);
  printf("  Revision:                 %d.%d\n",      (mmc->cid[2] >> 28) & 0x0f,
                                                     (mmc->cid[2] >> 24) & 0x0f);
  printf("  Ser no:                   %06x%02x\n",    mmc->cid[2]        & 0xffffff,
                                                     (mmc->cid[3] >> 24) & 0xff);
  printf("  Mfg date:                 %d/%d\n",      (mmc->cid[3] >>  8) & 0x0f,
                                                    ((mmc->cid[3] >> 12) & 0xff) + 2000);

  printf("  Version:                  %s %d.%d\n",  IS_SD(mmc) ? "SD" : "MMC",
                                                     (mmc->version >> 8) & 0xf,
                                                      mmc->version       & 0xff);
}

static void mmc_print_csd(struct mmc* mmc)
{
  /* CSD information */
  printf("\n*** Card Specific Data ***\n");
  printf("  CSD structure version:    %" PRIu8  "\n", 1 + ((mmc->csd[0] >> 30) & 0x03));
  printf("  Spec version:             %" PRIu8  "\n", (mmc->csd[0] >> 24) & 0x3f);
  printf("  Access time [ns]:         %" PRIu32 "\n",  mmc_access_time(mmc->csd));
  printf("  Max access time [ticks]:  %" PRIu32 "\n",  mmc_max_access_time(mmc->csd, mmc->tran_speed));
  printf("  Max transfer speed [B/s]: %"   "u"  "\n",  mmc->tran_speed);
  printf("  Max read  block size [B]: %"   "u"  "\n",  mmc->read_bl_len);
  printf("  Max write block size [B]: %"   "u"  "\n",  mmc->write_bl_len);
  printf("  Block count:              %" PRIu32 "\n", (uint32_t)(mmc->capacity /  mmc->read_bl_len));
  printf("  Capacity [B]:             %" PRIu64 "\n",  mmc->capacity);
  printf("  Copy:                     %" PRIu8  "\n", (mmc->csd[3] >> 14) & 0x01);
  printf("  Permanent write protect:  %" PRIu8  "\n", (mmc->csd[3] >> 13) & 0x01);
  printf("  Temporary write protect:  %" PRIu8  "\n", (mmc->csd[3] >> 12) & 0x01);
}

static void mmc_print_host_caps(struct mmc* mmc)
{
  static const char* const no  = "no";
  static const char* const yes = "yes";

  /* Print host capabilities */
  printf("\n*** Host Capabilities ***\n");
  printf("  High speed:               %s\n", (mmc->host_caps & MMC_MODE_HS)       ? yes : no);
  printf("  High speed 52 MHz:        %s\n", (mmc->host_caps & MMC_MODE_HS_52MHz) ? yes : no);
  printf("  4-Bit:                    %s\n", (mmc->host_caps & MMC_MODE_4BIT)     ? yes : no);
  printf("  8-Bit:                    %s\n", (mmc->host_caps & MMC_MODE_8BIT)     ? yes : no);
  printf("  SPI mode:                 %s\n", (mmc->host_caps & MMC_MODE_SPI)      ? yes : no);
  printf("  High capacity:            %s\n", (mmc->host_caps & MMC_MODE_HC)       ? yes : no);
}

static void mmc_print_info(struct mmc* mmc, bool verbose)
{
  printf("Device %lu/%lu: %s\n", rtems_filesystem_dev_major_t(mmc->dev),
                                 rtems_filesystem_dev_minor_t(mmc->dev),
                                 mmc->name);
  if (verbose) {
    mmc_print_cid(mmc);
    mmc_print_csd(mmc);
    mmc_print_host_caps(mmc);
  }
}

/**
 * MMC disk IOCTL handler.
 *
 * @param dd Disk device.
 * @param req IOCTL request code.
 * @param argp IOCTL argument.
 * @retval The IOCTL return value
 */
static int
mmc_ioctl(rtems_disk_device *dd, uint32_t req, void *arg)
{
  rtems_status_code sc;
  struct mmc*       mmc = rtems_disk_get_driver_data(dd);

  sc = rtems_semaphore_obtain(mmc->lock, RTEMS_WAIT, 0);
  if (sc != RTEMS_SUCCESSFUL)
    errno = EIO;
  else {
    errno = 0;
    switch (req) {
      case RTEMS_BLKIO_REQUEST: {
        rtems_blkdev_request* rqst = arg;

        switch (rqst->req) {
          case RTEMS_BLKDEV_REQ_READ:
            errno = mmc_read(dd, rqst);
            rtems_blkdev_request_done(rqst, errno ? RTEMS_IO_ERROR
                                                  : RTEMS_SUCCESSFUL);
            break;

          case RTEMS_BLKDEV_REQ_WRITE:
            errno = mmc_write(dd, rqst);
            rtems_blkdev_request_done(rqst, errno ? RTEMS_IO_ERROR
                                                  : RTEMS_SUCCESSFUL);
            break;

          default:
            RTEMS_SYSLOG("req %d invalid\n", rqst->req);
            errno = EINVAL;
            break;
        }
        break;
      }

      case RTEMS_MMC_IOCTL_ERASE_BLOCKS: {
        struct {
          uint32_t start;
          uint32_t count;
        } *prm = arg;
        mmc_erase(mmc, prm->start, prm->count);
        break;
      }
      case RTEMS_MMC_IOCTL_PRINT_STATUS:
        mmc_print_info(mmc, true);
        break;

      default:
        errno = rtems_blkdev_ioctl(dd, req, arg);
        break;
    }

    sc = rtems_semaphore_release(mmc->lock);
    if (sc != RTEMS_SUCCESSFUL)
      errno = EIO;
  }

  return errno == 0 ? 0 : -1;
}

int mmc_register(struct mmc* mmc, dev_t dev)
{
  rtems_status_code sc;

  /* Initialize the device */
  sc = mmc_init(mmc);
  if (sc != RTEMS_SUCCESSFUL) {
    RTEMS_SYSLOG("mmc_init() failed\n");
    return RTEMS_UNSATISFIED;
  }

  if (!mmc->b_max)
    mmc->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

  mmc->dev = dev;

  /* Setup the universal parts of the block interface just once */
  sc = rtems_disk_create_phys(dev,
                              mmc->read_bl_len,
                              mmc->capacity /  mmc->read_bl_len,
                              mmc_ioctl,
                              mmc,
                              mmc->name);
  if (sc != RTEMS_SUCCESSFUL) {
    rtems_disk_delete(dev);

    RTEMS_SYSLOG("Disk create phy failed\n");
    return RTEMS_UNSATISFIED;
  }

  /* Mutex for avoiding concurrent access to the device */
  sc = rtems_semaphore_create(rtems_build_name('M', 'C', 'C', 'K'), 1,
                              RTEMS_PRIORITY | RTEMS_BINARY_SEMAPHORE |
                              RTEMS_INHERIT_PRIORITY, 0, &mmc->lock);
  if (sc != RTEMS_SUCCESSFUL)
  {
    rtems_disk_delete(dev);

    RTEMS_SYSLOG("Disk lock create failed\n");
    return RTEMS_UNSATISFIED;
  }

  return 0;
}

int mmc_start_init(struct mmc* mmc)
{
  int err;

  if (mmc_getcd(mmc) == 0) {
    mmc->has_init = 0;
    RTEMS_SYSLOG("No card present\n");
    return NO_CARD_ERR;
  }

  /* If card has initialized, we're done */
  if (mmc->has_init)
    return 0;

  err = mmc->init(mmc);
  if (err)
    return err;

  mmc_set_bus_width(mmc, 1);
  mmc_set_clock(mmc, 1);

  /* Reset the Card */
  err = mmc_go_idle(mmc);
  if (err)
    return err;

  /* The internal partition reset to user partition(0) at every CMD0*/
  mmc->part_num = 0;

  /* Test for SD version 2 */
  err = mmc_send_if_cond(mmc);

  /* Now try to get the SD card's operating condition */
  err = sd_send_op_cond(mmc);

  /* If the command timed out, we check for an MMC card */
  if (err == TIMEOUT) {
    err = mmc_send_op_cond(mmc);
    if (err && err != IN_PROGRESS) {
      RTEMS_SYSLOG("Card did not respond to voltage select\n");
      return UNUSABLE_ERR;
    }
  }

  if (err == IN_PROGRESS)
    mmc->init_in_progress = 1;

  return err;
}

static int mmc_complete_init(struct mmc* mmc)
{
  int err = 0;

  if (mmc->op_cond_pending)
    err = mmc_complete_op_cond(mmc);

  if (!err)
    err = mmc_startup(mmc);

  mmc->has_init = err ? 0 : 1;

  mmc->init_in_progress = 0;

  return err;
}

int mmc_init(struct mmc* mmc)
{
  int      err   = IN_PROGRESS;
#if MMC_PERF
  uint64_t start = get_timer(0);
#endif

  if (mmc->has_init)
    return 0;

  if (!mmc->init_in_progress)
    err = mmc_start_init(mmc);

  if (!err || err == IN_PROGRESS)
    err = mmc_complete_init(mmc);

#if MMC_PERF
  printf("%s: err = %d, time = %llu ticks\n", __func__, err, get_timer(start));
#endif

  return err;
}

void print_mmc_devices()
{
  rtems_status_code  sc  = RTEMS_SUCCESSFUL;
  dev_t              dev = (dev_t)-1;
  rtems_disk_device *dd;

  printf("\nMMC devices:\n");

  /* Loop over all known disk devices */
  while (sc == RTEMS_SUCCESSFUL && (dd = rtems_disk_next(dev)) != NULL) {
    struct mmc* mmc = rtems_disk_get_driver_data(dd);

    dev = rtems_disk_get_device_identifier(dd);

    /* Handle only disk devices we recognize, i.e., MMC ones */
    if (dev == mmc->dev)
      mmc_print_info(mmc, mmc->verbose);

    sc = rtems_disk_release(dd);
  }

  printf("\n");
}


rtems_status_code rtems_mmc_attach(const rtems_driver_address_table* ops)
{
  rtems_status_code         sc;
  rtems_device_major_number major = 0;

  sc = rtems_disk_io_initialize();
  if (sc != RTEMS_SUCCESSFUL)
    return sc;

  sc = rtems_io_register_driver(0, ops, &major);
  if (sc != RTEMS_SUCCESSFUL)
    return RTEMS_UNSATISFIED;

  print_mmc_devices();

  return sc;
}
