/**
 * @file
 *
 * @ingroup eth_emacps
 *
 * @brief Ethernet driver.
 */

/*
 *                              Copyright 2013
 *                                     by
 *                        The Board of Trustees of the
 *                      Leland Stanford Junior University.
 *
 * See the associated software notice in the accompanying zynq.h header file.
 */
/*
 * Copyright (c) 2010 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#define __INSIDE_RTEMS_BSD_TCPIP_STACK__ 1
#define __BSD_VISIBLE                    1

#include <assert.h>

//#include <libcpu/arm-cp15.h>

#include "ethernet.h"

#include "xstatus.h"

//#include "xil_mmu.h"
//#include "xil_exception.h"
//#include "xil_cache.h"
//#include "xil_cache_l.h"

#include "xemacps_bd.h"

#include "memory/mem.h"

/* Byte alignment of BDs */
#define BD_ALIGNMENT XEMACPS_DMABD_MINIMUM_ALIGNMENT


static void eth_emacps_update_tx_stats(EthEmacPs  *emacPs,
                                       XEmacPs_Bd *bd)
{
  uint32_t s = XEmacPs_BdGetStatus(bd);

  /* Update error counters */
  if (s & EMACPS_ETH_TX_STAT_MASK) {
    if (s & EMACPS_ETH_TX_RETRIES) {
      ++emacPs->stats.tx_stat_retries;
    }
    if (s & EMACPS_ETH_TX_AHB_ERROR) {
      ++emacPs->stats.tx_stat_ahb_errors;
    }
    if (s & EMACPS_ETH_TX_LATE_COLLISION) {
      ++emacPs->stats.tx_stat_late_collisions;
    }
    switch (s & EMACPS_ETH_TX_CKSUM_ERROR) {
      case EMACPS_ETH_TX_NO_ERROR:
        ++emacPs->stats.tx_stat_no_errors;
        break;
      case EMACPS_ETH_TX_VLAN_ERROR:
        ++emacPs->stats.tx_stat_vlan_errors;
        break;
      case EMACPS_ETH_TX_SNAP_ERROR:
        ++emacPs->stats.tx_stat_snap_errors;
        break;
      case EMACPS_ETH_TX_IP_ERROR:
        ++emacPs->stats.tx_stat_ip_errors;
        break;
      case EMACPS_ETH_TX_ID_ERROR:
        ++emacPs->stats.tx_stat_id_errors;
        break;
      case EMACPS_ETH_TX_BADFRAG_ERROR:
        ++emacPs->stats.tx_stat_badfrag_errors;
        break;
      case EMACPS_ETH_TX_TCPUDP_ERROR:
        ++emacPs->stats.tx_stat_tcpudp_errors;
        break;
      case EMACPS_ETH_TX_EOP_ERROR:
        ++emacPs->stats.tx_stat_eop_errors;
        break;
    }
  }
}

#ifdef RECORD
extern struct {
  struct mbuf *alloc;
  unsigned     ctrl;
  struct mbuf *free;
  unsigned     status;
}        mbufList[64];
extern unsigned mbufIdx;
extern unsigned mbufFreeIdx;
#endif

void eth_emacps_process_txd_bds(EthEmacPs      *emacPs,
                                XEmacPs_BdRing *txRing)
{
  unsigned      k;
  XEmacPs_Bd   *txBdSet;
  XEmacPs_Bd   *curBd;
  struct mbuf **mbufs = emacPs->txMbufTable;
  unsigned      nBds  = XEmacPs_BdRingFromHwTx(txRing,
                                               emacPs->txUnitCount,
                                               &txBdSet);

  PRINTK("%-40s: beg, numBds = %08x\n", __func__, nBds);

  /* txBdSet is located in non-cached memory, so no need to call invalidate */

  /* Free the processed BD's */
  for (k = 0, curBd = txBdSet; k < nBds; ++k) {
    unsigned     bdIdx  = XEMACPS_BD_TO_INDEX(txRing, curBd);
    struct mbuf *m      = mbufs[bdIdx];
    unsigned     status = XEMACPS_TXBUF_USED_MASK;

    /* Update statistics counters */
    eth_emacps_update_tx_stats(emacPs, curBd);

#ifdef RECORD
    mbufList[mbufFreeIdx   & 0x3ff].free   = m;
    mbufList[mbufFreeIdx++ & 0x3ff].status = ((unsigned*)curBd)[1] | (mbufIdx << 16);
#endif

    /* Release mbuf */
    PRINTK("%-40s: bdIdx = %08x, mbufs = %08x, mbuf = %08x, m_data = %08x\n", __func__,
           bdIdx, (uint)mbufs, (uint)m, (uint)m->m_data);
    if (m != NULL) {
      m_free(m);
    }
    mbufs[bdIdx] = NULL;

    /* Reset the BD - these are in uncached memory */
    if (bdIdx == (emacPs->txUnitCount - 1)) {
      status |= XEMACPS_TXBUF_WRAP_MASK;
    }
    ((unsigned*)curBd)[0] = 0xabadbabe; /* Avoid overwriting previous buffer */
    ((unsigned*)curBd)[1] = status;     /* Set USED on ALL BDs of a frame */

    /* Next one */
    curBd = XEmacPs_BdRingNext(txRing, curBd);
    dsb();
  }

  if (nBds) {
    int xs = XEmacPs_BdRingFree(txRing, nBds, txBdSet);
    if (xs != 0) {
      RTEMS_SYSLOG("Error %d from BdRingFree\n", xs);
    }
  }

  PRINTK("%-40s: end, cnt = %08x\n", __func__, nBds);
}


/*!
 * send_handler():
 *
 * @brief Handler called from the ISR at packet transmission DMA completion
 */
void eth_emacps_send_handler(void *arg)
{
  rtems_status_code sc;
  EthEmacPs        *emacPs  = (EthEmacPs *)arg;
  unsigned          baseAdx = emacPs->instance.Config.BaseAddress;
  unsigned          reg     = XEmacPs_ReadReg(baseAdx, XEMACPS_TXSR_OFFSET);

  PRINTK("%-40s: beg\n", __func__);

  /* Clear the tx_status register by setting tx_complete */
  XEmacPs_WriteReg(baseAdx, XEMACPS_TXSR_OFFSET, reg);

  ++emacPs->stats.tx_interrupts;        /* Count TX DMA completions */
  PRINTK("%-40s: tx_interrupts = %lu\n", __func__, emacPs->stats.tx_interrupts);

  /* Wake up the Transmit task to process the sent BDs */
  sc = rtems_bsdnet_event_send(gEthEmacPsTxTask, ETH_EMACPS_EVENT_INTERRUPT);
  assert(sc == RTEMS_SUCCESSFUL);

  PRINTK("%-40s: end\n", __func__);
}


/*!
 * sgsend():
 *
 * @brief Scatter/gather send of a frame, potentially composed of multiple
 *        fragments.
 *
 * This function should be called only with interrupts disabled.
 */
rtems_status_code eth_emacps_sgsend(EthEmacPs *emacPs, struct mbuf *m)
{
  int             xs;
  uint32_t        ctrl;
  unsigned        bdIdx;
  XEmacPs_Bd     *curBd, *prvBd;
  XEmacPs_Bd     *txBdSet;
  XEmacPs_BdRing *txRing = &(XEmacPs_GetTxRing(&emacPs->instance));
  struct mbuf   **mbufs  = emacPs->txMbufTable;
  struct mbuf    *n;
  int             n_mbufs;

  PRINTK("%-40s: beg\n", __func__);

  /* First count the number of mbufs */
  for (n = m, n_mbufs = 0; n != NULL; n = n->m_next)
    n_mbufs++;

  PRINTK("%-40s: n_mbufs = %08x\n", __func__, n_mbufs);

  /* Obtain as many BD's */
  xs = XEmacPs_BdRingAlloc(txRing, n_mbufs, &txBdSet);
  if (xs != 0) {
    RTEMS_SYSLOG("Error %d allocating %d TxBDs\n", xs, n_mbufs);
    return RTEMS_NO_MEMORY;
  }

  for(n = m, curBd = txBdSet; n != NULL; n = n->m_next) {
    /* Send the data from the mbuf to the interface, one mbuf at a time.
       The size of the data in each mbuf is kept in the ->m_len variable. */
#if 1
    rtems_cache_flush_multiple_data_lines(mtod(n, const void *),
                                          (size_t) n->m_len);
#else
    Xil_DCacheFlushRange((unsigned)mtod(n, const void *), (size_t)n->m_len);
#endif

    bdIdx = XEMACPS_BD_TO_INDEX(txRing, curBd);
    if (mbufs[bdIdx] != NULL) {
      RTEMS_SYSLOG("mbuf not available for index %d\n", bdIdx);
      return RTEMS_RESOURCE_IN_USE;
    }
    mbufs[bdIdx] = n;

    ctrl = XEMACPS_TXBUF_USED_MASK; /* s/w must continue to own the fragment until we're ready to transmit */
    if (n->m_len > (XEMACPS_MAX_FRAME_SIZE - XEMACPS_HDR_SIZE - XEMACPS_TRL_SIZE))
      ctrl |= (XEMACPS_MAX_FRAME_SIZE - XEMACPS_HDR_SIZE - XEMACPS_TRL_SIZE) & XEMACPS_TXBUF_LEN_MASK;
    else
      ctrl |= n->m_len & XEMACPS_TXBUF_LEN_MASK;
    if (bdIdx == (emacPs->txUnitCount - 1)) {
      ctrl |= XEMACPS_TXBUF_WRAP_MASK;
    }
    if (n->m_next == NULL) {
      ctrl |= XEMACPS_TXBUF_LAST_MASK;
    }

    XEmacPs_BdSetAddressTx(curBd, mtod(n, unsigned));
    /*XEmacPs_BdSetStatus(curBd, ctrl);*/ /* Only ORs */
    XEmacPs_BdWrite(curBd, XEMACPS_BD_STAT_OFFSET, ctrl);
    dsb();

    PRINTK("%-40s: %02" PRIu32 ": %u %s\n", __func__,
           bdIdx, n->m_len, (n->m_next == NULL) ? "L" : "");

    DUMP4("mb:", __func__, false, n, sizeof(*n));
    DUMP2("md:", __func__, true,  n->m_data, n->m_len);

    prvBd = curBd;
    curBd = XEmacPs_BdRingNext(txRing, curBd);
  }

  /* Quickly hand ownership of all fragments in the frame to h/w.
     Do this in reverse order to prevent the DMA engine from advancing
     to the next fragment before s/w gets a chance to hand over ownership */
  for(n = m, curBd = prvBd; n != NULL; n = n->m_next) {
    XEmacPs_BdClearTxUsed(curBd);

    curBd = XEmacPs_BdRingPrev(txRing, curBd);
  }
  dsb();

  /* Flush the set of BDs to hardware */
  xs = XEmacPs_BdRingToHw(txRing, n_mbufs, txBdSet);
  if (xs != 0) {
    RTEMS_SYSLOG("Error %d submitting TxBD to hardware\n", xs);
    return RTEMS_IO_ERROR;
  }
  dsb();

  /* Start transmit - no harm to "start" it again if it is still running */
  XEmacPs_Transmit( (&emacPs->instance) );
  dsb();

  /* Increment transmitted frames counter */
  ++emacPs->stats.txd_frames;
  PRINTK("%-40s: txd_frames = %08x\n", __func__, emacPs->stats.txd_frames);

  PRINTK("%-40s: end, status = %d\n", __func__, xs);
  return RTEMS_SUCCESSFUL;
}


static void eth_emacps_update_rx_stats(EthEmacPs  *emacPs,
                                       XEmacPs_Bd *bd)
{
  uint32_t s = XEmacPs_BdGetStatus(bd);

  /* Update error counters */
  if (s & EMACPS_ETH_RX_STAT_MASK) {
    if (s & EMACPS_ETH_RX_BCAST) {
      ++emacPs->stats.rx_stat_bcast;
    }
    if (s & EMACPS_ETH_RX_MCAST) {
      ++emacPs->stats.rx_stat_mcast;
    }
    if (s & EMACPS_ETH_RX_UCAST) {
      ++emacPs->stats.rx_stat_ucast;
    }
    switch (s & EMACPS_ETH_RX_EXTERNAL) {
      case EMACPS_ETH_RX_ADX1:
        ++emacPs->stats.rx_stat_adx1;
        break;
      case EMACPS_ETH_RX_ADX2:
        ++emacPs->stats.rx_stat_adx2;
        break;
      case EMACPS_ETH_RX_ADX3:
        ++emacPs->stats.rx_stat_adx3;
        break;
      case EMACPS_ETH_RX_ADX4:
        ++emacPs->stats.rx_stat_adx4;
        break;
    }
    if (s & EMACPS_ETH_RX_BIT_24) {
      ++emacPs->stats.rx_stat_bit_24;
    }
    switch (s & EMACPS_ETH_RX_BIT_22_23) {
      case EMACPS_ETH_RX_REG1:
        ++emacPs->stats.rx_stat_reg1;
        break;
      case EMACPS_ETH_RX_REG2:
        ++emacPs->stats.rx_stat_reg2;
        break;
      case EMACPS_ETH_RX_REG3:
        ++emacPs->stats.rx_stat_reg3;
        break;
      case EMACPS_ETH_RX_REG4:
        ++emacPs->stats.rx_stat_reg4;
        break;
    }
    if (s & EMACPS_ETH_RX_VLAN) {
      ++emacPs->stats.rx_stat_vlan;
    }
    if (s & EMACPS_ETH_RX_PRIORITY) {
      ++emacPs->stats.rx_stat_priority;
    }
    if (s & EMACPS_ETH_RX_CFI) {
      ++emacPs->stats.rx_stat_cfi;
    }
    if (s & EMACPS_ETH_RX_EOF) {
      ++emacPs->stats.rx_stat_eof;
    }
    if (s & EMACPS_ETH_RX_SOF) {
      ++emacPs->stats.rx_stat_sof;
    }
    if (s & EMACPS_ETH_RX_FCS) {
      ++emacPs->stats.rx_stat_fcs;
    }
  }
}


void eth_emacps_process_rxd_bds(EthEmacPs      *emacPs,
                                XEmacPs_BdRing *rxRing,
                                int             wait)
{
  struct mbuf **mbufs   = emacPs->rxMbufTable;
  unsigned      freeBds = XEmacPs_BdRingGetFreeCnt(rxRing);
  unsigned      k;

  PRINTK("%-40s: beg, nBDs = %08x\n", __func__, freeBds);

  for (k = 0; k < freeBds; k++) {
    int          xs;
    struct mbuf *m;
    XEmacPs_Bd  *curBd;
    unsigned     bdIdx;

    /* Allocate BDs one at a time instead of all at once to improve odds */
    xs = XEmacPs_BdRingAlloc(rxRing, 1, &curBd);
    if (xs != 0) {
      RTEMS_SYSLOG("Error %d allocating RxBD\n", xs);
      break;
    }

    /* curBd is located in non-cached memory, so no need to cache invalidate */

    /* Update statistics counters from curBd's previous use */
    eth_emacps_update_rx_stats(emacPs, curBd);

    /* Associate an mbuf with the BD */
    m = eth_emacps_add_new_mbuf(emacPs, mbufs, wait);
    if (m == NULL) {
      ++emacPs->stats.rx_memErr;
      ++emacPs->stats.rx_drop;
      RTEMS_SYSLOG("Unable to allocate mbuf\n");
      XEmacPs_BdRingUnAlloc(rxRing, 1, curBd);
      dsb();
      break;
    }

    bdIdx = XEMACPS_BD_TO_INDEX(rxRing, curBd);

    /* Add mbuf to ring; The lower two bits of m->m_data must be shaved off
       since DMA uses them for ownership and wrap!  Use  bits 14 and 15 of
       the Network Configuration register to set up finer alignment.  This
       is done in eth_emacps_setup_rx_dscs(). */
    XEmacPs_BdSetAddressRx(curBd, mtod(m, uint32_t) & ~0x3);

    /* Clear ownership and, if necessary, set wrap bit */
    XEmacPs_BdClearRxNew(curBd);
    if (bdIdx == (emacPs->rxUnitCount - 1)) {
      XEmacPs_BdSetRxWrap(curBd);
    }
    dsb();

    /* Enqueue to HW, flushing descriptor from cache */
    xs = XEmacPs_BdRingToHw(rxRing, 1, curBd);
    if (xs != 0) {
      RTEMS_SYSLOG("Error %d committing RxBD to hardware: \n", xs);
      if (xs == XST_DMA_SG_LIST_ERROR)
        RTEMS_SYSLOG("  XST_DMA_SG_LIST_ERROR: This function was called out of \n"
                     "  sequence with XEmacPs_BdRingAlloc()\n");
      else
        RTEMS_SYSLOG("  Set of BDs was rejected because: \n"
                     "  - the first BD did not have its start-of-packet bit set, or\n"
                     "  - the last  BD did not have its end-of-packet   bit set, or\n"
                     "  - any one of the BD set has 0 as length value\n");
      m_free(m);
      return;
    }

    /* Add mbuf to table */
    mbufs[bdIdx] = m;
    PRINTK("%-40s: bdIdx = %08x, mBuf = %08x, md = %08x\n", __func__,
           bdIdx, (uint)m, (uint)m->m_data);
  }

  PRINTK("%-40s: end, cnt = %08x\n", __func__, k);
}


/*
 * recv_handler():
 *
 * @brief Handler called from the ISR to wake up the receive task.
 */
void eth_emacps_recv_handler(void *arg)
{
  rtems_status_code sc;
  EthEmacPs        *emacPs  = (EthEmacPs *)arg;
  unsigned          baseAdx = emacPs->instance.Config.BaseAddress;
  unsigned          reg     = XEmacPs_ReadReg(baseAdx, XEMACPS_RXSR_OFFSET);

  PRINTK("%-40s: beg\n", __func__);

  /* Clear the rx_status register by setting frame_recd */
  XEmacPs_WriteReg(baseAdx, XEMACPS_RXSR_OFFSET, reg);

  ++emacPs->stats.rx_interrupts;
  PRINTK("%-40s: rx_interrupts = %lu\n", __func__, emacPs->stats.rx_interrupts);

  /* If Reception-Done interrupt is asserted, wake the receive task */
  sc = rtems_bsdnet_event_send(gEthEmacPsRxTask, ETH_EMACPS_EVENT_INTERRUPT);
  assert(sc == RTEMS_SUCCESSFUL);

  PRINTK("%-40s: end\n", __func__);
}


int eth_emacps_setup_rx_dscs(EthEmacPs *emacPs, XEmacPs_BdRing *rxRing)
{
  int        xs;
  XEmacPs_Bd bdTemplate;
  unsigned   netCfg;

  PRINTK("%-40s: beg\n", __func__);

  /* Setup a BD template for the Rx channel */
  XEmacPs_BdClear(&bdTemplate);

  /* Create the RxBD ring */
  xs = XEmacPs_BdRingCreate(rxRing,
                            (uint32_t)emacPs->rxBdSpace,
                            (uint32_t)emacPs->rxBdSpace,
                            BD_ALIGNMENT,
                            emacPs->rxUnitCount);
  if (xs != 0) {
    RTEMS_SYSLOG("Error %d setting up RxBD space, BdRingCreate", xs);
    return xs;
  }

  /*
   * The template will be copied to every RxBD.
   * We will not have to explicitly set these again.
   */
  xs = XEmacPs_BdRingClone(rxRing, &bdTemplate, XEMACPS_RECV);
  if (xs != 0) {
    RTEMS_SYSLOG("Error %d setting up RxBD space, BdRingClone", xs);
    return xs;
  }

  /* Offset buffer alignment by 16 bits to give 32 bit alignment of struct ip */
  netCfg = XEmacPs_ReadReg(emacPs->instance.Config.BaseAddress,
                           XEMACPS_NWCFG_OFFSET);
  netCfg |= (ETH_EMACPS_RX_DATA_OFFSET & 0x3) << 14;
  XEmacPs_WriteReg(emacPs->instance.Config.BaseAddress,
                   XEMACPS_NWCFG_OFFSET, netCfg);

  PRINTK("%-40s: end, rxUnitCount = %08x\n", __func__, emacPs->rxUnitCount);
  return xs;
}

int eth_emacps_setup_tx_dscs(EthEmacPs *emacPs, XEmacPs_BdRing *txRing)
{
  int             xs;
  XEmacPs_Bd      bdTemplate;

  PRINTK("%-40s: beg\n", __func__);

  /* Setup a BD template for the Tx channel */
  XEmacPs_BdClear(&bdTemplate);
  XEmacPs_BdSetStatus(&bdTemplate, XEMACPS_TXBUF_USED_MASK);

  /* Create the TxBD ring */
  xs = XEmacPs_BdRingCreate(txRing,
                            (uint32_t)emacPs->txBdSpace,
                            (uint32_t)emacPs->txBdSpace,
                            BD_ALIGNMENT,
                            emacPs->txUnitCount);
  if (xs != 0) {
    RTEMS_SYSLOG("Error %d setting up TxBD space, BdRingCreate", xs);
    return xs;
  }

  /* Copy the template into every TxBd */
  xs = XEmacPs_BdRingClone(txRing, &bdTemplate, XEMACPS_SEND);
  if (xs != 0) {
    RTEMS_SYSLOG("Error %d setting up TxBD space, BdRingClone", xs);
    return xs;
  }

  PRINTK("%-40s: end, txUnitCount = %08x\n", __func__, emacPs->txUnitCount);
  return xs;
}


int eth_emacps_init_dma(EthEmacPs* emacPs)
{
  XEmacPs_BdRing *rxRing;
  XEmacPs_BdRing *txRing;
  int             xs;

  PRINTK("%-40s: beg\n", __func__);

  /* Allocate a 1 MB MMU page aligned to 1 MB boundary for the BDs. */
  size_t    size      = 0x100000;
  uintptr_t alignment = (uintptr_t)0x100000;
  uintptr_t boundary  = (uintptr_t)0;
  uint8_t*  bdSpace   = (uint8_t*)mem_Region_alloc(MEM_REGION_UNCACHED,size);

  /* Make sure we got the space */
  if (bdSpace == NULL) {
    RTEMS_SYSLOG("BD space allocation failed\n");
    return -1;
  }

  /*
   * The BDs need to be allocated in uncached memory. 
   */

  rxRing = &XEmacPs_GetRxRing(&emacPs->instance);
  txRing = &XEmacPs_GetTxRing(&emacPs->instance);
  PRINTK("%-40s: rxRing = 0x%08x\n", __func__, rxRing);
  PRINTK("%-40s: txRing = 0x%08x\n", __func__, txRing);

  /*
   * We allocate 65536 bytes for both Rx and Tx BDs, which can each accomodate
   * a max of 8192 BDs.  That is much more than any application will ever need.
   * The rest of the 1 MB page is unused.
   */
  emacPs->rxBdSpace = (void *)bdSpace;
  emacPs->txBdSpace = (void *)(bdSpace + 0x10000);

  PRINTK("%-40s: rxBdSpace = 0x%08x\n", __func__, emacPs->rxBdSpace);
  PRINTK("%-40s: txBdSpace = 0x%08x\n", __func__, emacPs->txBdSpace);

  if (!emacPs->rxBdSpace || !emacPs->txBdSpace) {
    RTEMS_SYSLOG("Unable to allocate memory for TX/RX buffer descriptors");
    return XST_FAILURE;
  }

  /* Set up RX descriptors */
  xs = eth_emacps_setup_rx_dscs(emacPs, rxRing);
  if (xs != 0) {
    RTEMS_SYSLOG("Error %d setting up RX descriptors\n", xs);
    return xs;
  }

  /* Set up TX descriptors */
  xs = eth_emacps_setup_tx_dscs(emacPs, txRing);
  if (xs != 0) {
    RTEMS_SYSLOG("Error %d setting up TX descriptors\n", xs);
    return xs;
  }

  PRINTK("%-40s: end\n", __func__);

  return 0;
}


void eth_emacps_free_tx_rx_mbufs(EthEmacPs* emacPs)
{
  int           i;
  struct mbuf **mbufs = emacPs->rxMbufTable;

  PRINTK("%-40s: beg\n", __func__);
  eth_emacps_free_only_tx_mbufs(emacPs);

  for (i = 0; i < emacPs->rxUnitCount; ++i) {
    m_free(mbufs[i]);
  }
  PRINTK("%-40s: end\n", __func__);
}


void eth_emacps_free_only_tx_mbufs(EthEmacPs* emacPs)
{
  int           i;
  struct mbuf **mbufs = emacPs->txMbufTable;

  PRINTK("%-40s: beg\n", __func__);
  for (i = 0; i < emacPs->txUnitCount; ++i) {
    if (mbufs[i] != 0) {
      m_free(mbufs[i]);
      mbufs[i] = 0;
    }
  }
  PRINTK("%-40s: end\n", __func__);
}
