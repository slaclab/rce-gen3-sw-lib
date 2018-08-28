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
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
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

#include <stdio.h>
#include <string.h>

#include "xparameters.h"
#include <xemacps.h>

#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <rtems.h>
#include <rtems/rtems_bsdnet.h>
#include <rtems/rtems_mii_ioctl.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/mbuf.h>

#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

#include <bsp.h>
#include <bsp/irq.h>
#include <bsp/utility.h>

#include "ethernet.h"               /* This must be included early */


// RiC: This is not used, but save it so it can be resurrected
//#if LWIP_IGMP
//static uint8_t eth_emacps_mcast_entry_mask = 0;
//
//
//static int eth_emacps_mac_filter_update (struct netif   *netif,
//                                         struct ip_addr *group,
//                                         uint8_t         action)
//{
//  return 0;
//}
//#endif


/*
 * low_level_output():
 *
 * Should do the actual transmission of the packet. The packet is
 * contained in the mbuf that is passed to the function. This mbuf
 * might be chained.
 *
 */
static rtems_status_code low_level_output(EthEmacPs *emacPs, struct mbuf *m)
{
  /* TX space is available as long as there are valid BDs */
  XEmacPs_BdRing   *txRing = &(XEmacPs_GetTxRing(&emacPs->instance));
  int               cnt    = XEmacPs_BdRingGetFreeCnt(txRing);
  rtems_status_code sc;

  PRINTK("%-40s: beg, cnt = %08x\n", __func__, cnt);

  /* Check if space is available to send */
  if (cnt > 0) {
    sc = eth_emacps_sgsend(emacPs, m);
    if ((sc == RTEMS_SUCCESSFUL) || (sc == RTEMS_NO_MEMORY)) {
      ++emacPs->stats.tx_xmit;
      sc = RTEMS_SUCCESSFUL;
    } else {
      ++emacPs->stats.tx_drop;
      PRINTK("%-40s: sc = %08x, drop = %08x\n", __func__,
             sc, emacPs->stats.tx_drop);
      m_freem(m);
    }
  } else {
    ++emacPs->stats.tx_full_queues;
    sc = RTEMS_UNSATISFIED;
  }

  PRINTK("%-40s: end, cnt = %08x\n", __func__, cnt);
  return sc;
}


/*
 * eth_emacps_output():
 *
 * This function is called by the TCP/IP stack when an IP packet
 * should be sent. It calls the function called low_level_output() to
 * do the actual transmission of the packet.
 *
 */
rtems_status_code eth_emacps_output(EthEmacPs   *emacPs,
                                    struct mbuf *m)
{
  rtems_status_code     sc;
  rtems_interrupt_level level;

  PRINTK("%-40s: beg\n", __func__);

  rtems_interrupt_disable(level);

  /* Resolve hardware address, then send (or queue) packet */
  sc = low_level_output(emacPs, m);

  rtems_interrupt_enable(level);

  PRINTK("%-40s: end, sc = %08x\n", __func__, sc);
  return sc;
}


/*
 * low_level_input():
 *
 * Should allocate an mbuf and transfer the bytes of the incoming
 * packet from the interface into the mbuf.
 *
 * Returns the number of packets read
 *
 */
static int low_level_input(EthEmacPs *emacPs, XEmacPs_BdRing *rxRing)
{
  unsigned              k;
  XEmacPs_Bd           *rxBdSet;
  XEmacPs_Bd           *curBd;
  struct ifnet *const   ifp   = &emacPs->arpcom.ac_if;
  struct mbuf         **mbufs = emacPs->rxMbufTable;
  unsigned              nBds  = XEmacPs_BdRingFromHwRx(rxRing,
                                                       emacPs->rxUnitCount,
                                                       &rxBdSet);

  PRINTK("%-40s: beg, numBds = %08x\n", __func__, nBds);

  /* See if there is data to process */
  for (k = 0, curBd = rxBdSet; k < nBds; k++) {
    unsigned             bdIdx = XEMACPS_BD_TO_INDEX(rxRing, curBd);
    struct mbuf         *m     = mbufs[bdIdx];
    struct ether_header *eh    = mtod(m, struct ether_header*);
    int                  sz    = XEmacPs_BdGetLength(curBd);

    PRINTK("%-40s: bdIdx = %08x, mBuf = %08x, m_data = %08x, sz = %08x\n", __func__,
           bdIdx, (uint)m, (uint)m->m_data, sz);

    /* Update mbuf */
    m->m_len        = sz              - ETHER_HDR_LEN;
    m->m_pkthdr.len = sz              - ETHER_HDR_LEN;
    m->m_data       = mtod(m, char *) + ETHER_HDR_LEN;

    PRINTK("%-40s: %02u: %u\n", __func__, bdIdx, sz);

    PRINTK("%-40s: status = %08x, hdr = %08x, hlen = %04x, data = %08x, dlen = %04x\n", __func__,
           XEmacPs_BdGetStatus(curBd), eh, ETHER_HDR_LEN, m->m_data, m->m_len);

    DUMP4("mb:", __func__, false, m, sizeof(*m));
    DUMP2("bd:", __func__, true,  eh, sz);

    ++emacPs->stats.rx_recv;

    /* Hand it over */
    ether_input(ifp, eh, m);

    /* Next one */
    curBd = XEmacPs_BdRingNext(rxRing, curBd);
  }

  /* Free up the BD's */
  if (nBds) {
    XEmacPs_BdRingFree(rxRing, nBds, rxBdSet);
  }
  dsb();

  PRINTK("%-40s: end, %u\n", __func__, nBds);
  return nBds;
}


/*
 * eth_emacps_input():
 *
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface.
 *
 * Returns the number of packets read
 *
 */
int eth_emacps_input(EthEmacPs *emacPs, XEmacPs_BdRing *rxRing)
{
  unsigned              n;
  rtems_interrupt_level level;

  PRINTK("%-40s: beg\n", __func__);

  /* Move received packet into a new mbuf */
  rtems_interrupt_disable(level);
  n = low_level_input(emacPs, rxRing);
  rtems_interrupt_enable(level);

  PRINTK("%-40s: end, %u\n", __func__, n);
  return n;
}


void eth_emacps_low_level_init(void *arg)
{
  EthEmacPs    *emacPs = (EthEmacPs *)arg;
  struct ifnet *ifp    = &emacPs->arpcom.ac_if;
  int           xs;

  /* Initialize instance */
  XEmacPs_Config* cfg = XEmacPs_LookupConfig(ifp->if_unit);
  if (cfg == NULL) {
    RTEMS_SYSLOG("XEmacPs configuration for device ID %d not found\n",
           ifp->if_unit);
    rtems_fatal_error_occurred(0xdead0000);
  }

  xs = XEmacPs_CfgInitialize(&emacPs->instance, cfg, cfg->BaseAddress);
  if (xs != 0) {
    RTEMS_SYSLOG("XEmacPs CfgInitialize for device ID %d failed with status %d\n",
           ifp->if_unit, xs);
    rtems_fatal_error_occurred(0xdead0001);
  }

  /* Initialize the MAC */
  xs = eth_emacps_init_mac(emacPs);
  if (xs != 0) {
    RTEMS_SYSLOG("XEmacPs initialization for device ID %d failed with status %d\n",
           ifp->if_unit, xs);
    rtems_fatal_error_occurred(0xdead0002);
  }

  /* Setup burst length */
  XEmacPs_DMABLengthUpdate(&emacPs->instance, XEMACPS_16BYTE_BURST);

  /* Set up ISR */
  xs = eth_emacps_setup_isr(emacPs);
  if (xs != 0) {
    RTEMS_SYSLOG("Setting up callbacks for device ID %d failed with status %d\n",
           ifp->if_unit, xs);
    rtems_fatal_error_occurred(0xdead0003);
  }

  /* Initialize DMA */
  xs = eth_emacps_init_dma(emacPs);
  if (xs != 0) {
    RTEMS_SYSLOG("Initializing DMAs for device ID %d failed with status %d\n",
           ifp->if_unit, xs);
    rtems_fatal_error_occurred(0xdead0004);
  }

  PRINTK("%-40s: end\n", __func__);
}


void eth_emacps_handle_error(EthEmacPs *emacPs)
{
  rtems_status_code sc;

  sc = rtems_bsdnet_event_send(gEthEmacPsErTask, ETH_EMACPS_EVENT_INTERRUPT);
  assert(sc == RTEMS_SUCCESSFUL);
}


void eth_emacps_error_task(void *arg)
{
  rtems_status_code  sc     = RTEMS_SUCCESSFUL;
  rtems_event_set    events = 0;
  EthEmacPs         *emacPs = (EthEmacPs *) arg;

  PRINTK("%-40s: beg\n", __func__);

  /* Main event loop */
  for (;;) {
    int                   xs = 0;
    rtems_interrupt_level level;

    PRINTK("%-40s: Waiting...\n", __func__);
    /* Wait for events */
    sc = rtems_bsdnet_event_receive( ETH_EMACPS_EVENT_INTERRUPT,
                                     RTEMS_EVENT_ANY | RTEMS_WAIT,
                                     RTEMS_NO_TIMEOUT,
                                     &events );
    assert(sc == RTEMS_SUCCESSFUL);

    PRINTK("%-40s: WAKE up: 0x%08" PRIx32 "\n", __func__, events);

    rtems_interrupt_disable(level);

    eth_emacps_free_tx_rx_mbufs(emacPs);

    xs = XEmacPs_CfgInitialize(&emacPs->instance, &emacPs->instance.Config,
                                   emacPs->instance.Config.BaseAddress);
    if (xs != 0) {
      RTEMS_SYSLOG("XEmacPs Configuration failed with status %d\n", xs);
    }

    /* Initialize the mac */
    eth_emacps_init_on_error(emacPs);

    /* Setup burst length */
    XEmacPs_DMABLengthUpdate(&emacPs->instance, XEMACPS_16BYTE_BURST);

    eth_emacps_setup_isr(emacPs);
    eth_emacps_init_dma(emacPs);
    eth_emacps_start(emacPs);

    rtems_interrupt_enable(level);
  }

  PRINTK("%-40s: end\n", __func__);
}


void eth_emacps_handle_tx_errors(EthEmacPs* emacPs)
{
  unsigned baseAdx = emacPs->instance.Config.BaseAddress;
  uint32_t netCtrlReg;
  PRINTK("%-40s: beg\n", __func__);

  rtems_interrupt_level level;
  rtems_interrupt_disable(level);

  /* Disable transmitter */
  netCtrlReg  = XEmacPs_ReadReg(baseAdx, XEMACPS_NWCTRL_OFFSET);
  netCtrlReg &= ~XEMACPS_NWCTRL_TXEN_MASK;
  XEmacPs_WriteReg(baseAdx, XEMACPS_NWCTRL_OFFSET, netCtrlReg);

  eth_emacps_free_only_tx_mbufs(emacPs);

  eth_emacps_setup_tx_dscs(emacPs, &XEmacPs_GetTxRing(&emacPs->instance));

  /* Enable transmitter */
  netCtrlReg  = XEmacPs_ReadReg(baseAdx, XEMACPS_NWCTRL_OFFSET);
  netCtrlReg |= XEMACPS_NWCTRL_TXEN_MASK;
  XEmacPs_WriteReg(baseAdx, XEMACPS_NWCTRL_OFFSET, netCtrlReg);

  rtems_interrupt_enable(level);
  PRINTK("%-40s: end\n", __func__);
}
