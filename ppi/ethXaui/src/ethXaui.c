/**
 * @file
 *
 * @ingroup eth_xaui
 *
 * @brief Xaui Ethernet driver.
 */

/*
 *                              Copyright 2015
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


/*
 * low_level_output():
 *
 * Should do the actual transmission of the packet. The packet is
 * contained in the mbuf that is passed to the function. This mbuf
 * might be chained.
 *
 */
static rtems_status_code low_level_output(EthXaui *xaui, struct mbuf *m)
{
  /* space is available as long as there are valid TXs */
  int               cnt    = xaui->txfree;
  rtems_status_code sc;
  struct mbuf    *n;
  int             n_mbufs;

  PRINTK("%-40s: beg, cnt = %08x\n", __func__, cnt);

  /* First count the number of mbufs */
  for (n = m, n_mbufs = 0; n != NULL; n = n->m_next)
    n_mbufs++;

  /* Check if space is available to send */
  if (n_mbufs <= cnt) {
    sc = eth_xaui_sgsend(xaui, m);
    if (sc != RTEMS_SUCCESSFUL) {
      ++xaui->stats.tx_drop;
      PRINTK("%-40s: sc = %08x, drop = %08x\n", __func__,
             sc, xaui->stats.tx_drop);
      m_freem(m);
    }
  } else {
    ++xaui->stats.tx_full_queues;
      PRINTK("%-40s: unsatisfied tx free = %08x, mbufs = %08x\n", __func__,
             xaui->txfree,n_mbufs);
    sc = RTEMS_UNSATISFIED;
  }

  PRINTK("%-40s: end, cnt = %08x\n", __func__, cnt);
  return sc;
}


/*
 * eth_xaui_output():
 *
 * This function is called by the TCP/IP stack when an IP packet
 * should be sent. It calls the function called low_level_output() to
 * do the actual transmission of the packet.
 *
 */
rtems_status_code eth_xaui_output(EthXaui   *xaui,
                                    struct mbuf *m)
{
  rtems_status_code     sc;
  rtems_interrupt_level level;

  PRINTK("%-40s: beg\n", __func__);

  rtems_interrupt_disable(level);

  /* Resolve hardware address, then send (or queue) packet */
  sc = low_level_output(xaui, m);

  rtems_interrupt_enable(level);

  PRINTK("%-40s: end, sc = %08x\n", __func__, sc);
  return sc;
}


void eth_xaui_low_level_init(void *arg)
{
  EthXaui    *xaui  = (EthXaui *)arg;
  struct ifnet *ifp = &xaui->arpcom.ac_if;
  int           xs;

  /* Initialize the MAC */
  xs = eth_xaui_init_mac(xaui);
  if (xs != 0) {
    RTEMS_SYSLOG("Xaui initialization for device ID %d failed with status %d\n",
           ifp->if_unit, xs);
    rtems_fatal_error_occurred(0xdead0000);
  }

  PRINTK("%-40s: end\n", __func__);
}
