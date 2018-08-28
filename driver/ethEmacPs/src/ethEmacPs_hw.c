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

#include "ethernet.h"


// RiC: This is not used, but save it so it can be resurrected
//XEmacPs_Config *eth_emacps_lookup_config(unsigned mac_base)
//{
//  extern XEmacPs_Config  XEmacPs_ConfigTable[];
//  XEmacPs_Config        *cfg = NULL;
//  int i;
//
//  for (i = 0; i < XPAR_XEMACPS_NUM_INSTANCES; i++) {
//    if (XEmacPs_ConfigTable[i].BaseAddress == mac_base) {
//      cfg = &XEmacPs_ConfigTable[i];
//      break;
//    }
//  }
//
//  return cfg;
//}


int eth_emacps_init_mac(EthEmacPs* emacPs)
{
  XEmacPs *instance = &emacPs->instance;
  int      xs;

  PRINTK("%-40s: beg\n", __func__);

  PRINTK("%-40s: MAC address = %02x:%02x:%02x:%02x:%02x:%02x\n", __func__,
         ((char*)emacPs->arpcom.ac_enaddr)[0],
         ((char*)emacPs->arpcom.ac_enaddr)[1],
         ((char*)emacPs->arpcom.ac_enaddr)[2],
         ((char*)emacPs->arpcom.ac_enaddr)[3],
         ((char*)emacPs->arpcom.ac_enaddr)[4],
         ((char*)emacPs->arpcom.ac_enaddr)[5]);

  /* Set the MAC address */
  xs = XEmacPs_SetMacAddress(instance, emacPs->arpcom.ac_enaddr, 1);
  if (xs != 0) {
    struct ifnet *ifp = &emacPs->arpcom.ac_if;
    RTEMS_SYSLOG("Setting MAC address for device ID %d failed with status %d\n",
                 ifp->if_unit, xs);
    return xs;
  }

  /* Set up the hardware */
  XEmacPs_SetMdioDivisor(instance, MDC_DIV_224);
  emacPs->linkSpeed = eth_emacps_phy_setup(emacPs);
  XEmacPs_SetOperatingSpeed(instance, emacPs->linkSpeed);
  sleep(1);         /* Setting the operating speed of the MAC needs a delay. */

  PRINTK("%-40s: end\n", __func__);
  return xs;
}


void eth_emacps_init_on_error (EthEmacPs *emacPs)
{
  XEmacPs *instance = &emacPs->instance;
  int      xs;

  PRINTK("%-40s: beg\n", __func__);

  PRINTK("%-40s: MAC address = %02x:%02x:%02x:%02x:%02x:%02x\n", __func__,
         ((char*)emacPs->arpcom.ac_enaddr)[0],
         ((char*)emacPs->arpcom.ac_enaddr)[1],
         ((char*)emacPs->arpcom.ac_enaddr)[2],
         ((char*)emacPs->arpcom.ac_enaddr)[3],
         ((char*)emacPs->arpcom.ac_enaddr)[4],
         ((char*)emacPs->arpcom.ac_enaddr)[5]);

  /* Set mac address */
  xs = XEmacPs_SetMacAddress(instance, emacPs->arpcom.ac_enaddr, 1);
  if (xs != 0) {
    RTEMS_SYSLOG("Setting MAC address failed with error %d\n", xs);
  }

  XEmacPs_SetOperatingSpeed(instance, emacPs->linkSpeed);
  sleep(1);         /* Setting the operating speed of the MAC needs a delay. */
  PRINTK("%-40s: end\n", __func__);
}


int eth_emacps_setup_isr(EthEmacPs* emacPs)
{
  int xs;

  PRINTK("%-40s: beg\n", __func__);

  /* Setup callbacks */
  xs  = XEmacPs_SetHandler(&emacPs->instance,
                           XEMACPS_HANDLER_DMASEND,
                           (void *) eth_emacps_send_handler,
                           &emacPs->instance);
  xs |= XEmacPs_SetHandler(&emacPs->instance,
                           XEMACPS_HANDLER_DMARECV,
                           (void *) eth_emacps_recv_handler,
                           &emacPs->instance);
  xs |= XEmacPs_SetHandler(&emacPs->instance,
                           XEMACPS_HANDLER_ERROR,
                           (void *) eth_emacps_error_handler,
                           &emacPs->instance);
  PRINTK("%-40s: end\n", __func__);

  return xs;
}


void eth_emacps_start(EthEmacPs *emacPs)
{
  PRINTK("%-40s: beg\n", __func__);

  /* Start the emacPs */
  XEmacPs_Start(&emacPs->instance);

  PRINTK("%-40s: end\n", __func__);
}

void eth_emacps_stop(EthEmacPs *emacPs)
{
  PRINTK("%-40s: beg\n", __func__);

  /* Stop the emacPs */
  XEmacPs_Stop(&emacPs->instance);

  PRINTK("%-40s: end\n", __func__);
}

void eth_emacps_reset(EthEmacPs *emacPs)
{
  PRINTK("%-40s: beg\n", __func__);

  /* Reset the emacPs */
  XEmacPs_Reset(&emacPs->instance);

  PRINTK("%-40s: end\n", __func__);
}

// RiC: This is not used, but save it so it can be resurrected
//void eth_emacps_restart_transmitter(EthEmacPs *emacPs)
//{
//  unsigned baseAdx = emacPs->instance.Config.BaseAddress;
//  uint32_t reg;
//
//  reg  = XEmacPs_ReadReg(baseAdx, XEMACPS_NWCTRL_OFFSET);
//  reg &= ~XEMACPS_NWCTRL_TXEN_MASK;
//  XEmacPs_WriteReg(baseAdx, XEMACPS_NWCTRL_OFFSET, reg);
//
//  reg  = XEmacPs_ReadReg(baseAdx, XEMACPS_NWCTRL_OFFSET);
//  reg |= XEMACPS_NWCTRL_TXEN_MASK;
//  XEmacPs_WriteReg(baseAdx, XEMACPS_NWCTRL_OFFSET, reg);
//}


void eth_emacps_error_handler(void *arg, uint8_t direction, uint32_t errorWord)
{
  rtems_status_code  sc       = RTEMS_SUCCESSFUL;
  EthEmacPs         *emacPs   = (EthEmacPs*) arg;
  //XEmacPs           *instance = &emacPs->instance;

  PRINTK("%-40s: beg, dir = %u, err = %lu\n", __func__, direction, errorWord);
  if (errorWord != 0) {
    switch (direction) {
      case XEMACPS_RECV: {
        if (errorWord & XEMACPS_RXSR_HRESPNOK_MASK) {
          RTEMS_SYSLOG("Receive DMA error\n");
          ++emacPs->stats.rx_dma_errors;
          eth_emacps_handle_error(emacPs);
        }
        if (errorWord & XEMACPS_RXSR_RXOVR_MASK) {
          RTEMS_SYSLOG("Receive overrun\n");
          ++emacPs->stats.rx_overruns;
          sc = rtems_bsdnet_event_send(gEthEmacPsRxTask, ETH_EMACPS_EVENT_INTERRUPT);
          assert(sc == RTEMS_SUCCESSFUL);
        }
        if (errorWord & XEMACPS_RXSR_BUFFNA_MASK) {
          RTEMS_SYSLOG("Receive buffer not available\n");
          ++emacPs->stats.rx_no_buffers;
          sc = rtems_bsdnet_event_send(gEthEmacPsRxTask, ETH_EMACPS_EVENT_INTERRUPT);
          assert(sc == RTEMS_SUCCESSFUL);
        }
        if (errorWord & ~XEMACPS_RXSR_ERROR_MASK) {
          RTEMS_SYSLOG("Unknown receive error\n");
          ++emacPs->stats.rx_unknowns;
        }
        break;
      }
      case XEMACPS_SEND: {
        if (errorWord & XEMACPS_TXSR_HRESPNOK_MASK) {
          RTEMS_SYSLOG("Transmit DMA error\n");
          ++emacPs->stats.tx_dma_errors;
          eth_emacps_handle_error(emacPs);
        }
        if (errorWord & XEMACPS_TXSR_URUN_MASK) {
          RTEMS_SYSLOG("Transmit underrun\n");
          ++emacPs->stats.tx_underruns;
          eth_emacps_handle_tx_errors(emacPs);
        }
        if (errorWord & XEMACPS_TXSR_BUFEXH_MASK) {
          RTEMS_SYSLOG("Transmit buffer exhausted\n");
          ++emacPs->stats.tx_no_buffers;
          eth_emacps_handle_tx_errors(emacPs);
        }
        if (errorWord & XEMACPS_TXSR_RXOVR_MASK) {
          RTEMS_SYSLOG("Transmit retry limit exceeded\n");
          ++emacPs->stats.tx_excessive_retries;
          eth_emacps_handle_tx_errors(emacPs);
        }
        if (errorWord & XEMACPS_TXSR_FRAMERX_MASK) {
          RTEMS_SYSLOG("Transmit collision\n");
          ++emacPs->stats.tx_collisions;
          sc = rtems_bsdnet_event_send(gEthEmacPsTxTask, ETH_EMACPS_EVENT_INTERRUPT);
          assert(sc == RTEMS_SUCCESSFUL);
        }
        if (errorWord & XEMACPS_TXSR_USEDREAD_MASK) {
          RTEMS_SYSLOG("Transmit buffer used bit set\n");
          ++emacPs->stats.tx_usedread;
          sc = rtems_bsdnet_event_send(gEthEmacPsTxTask, ETH_EMACPS_EVENT_INTERRUPT);
          assert(sc == RTEMS_SUCCESSFUL);
        }
        if (errorWord & ~XEMACPS_TXSR_ERROR_MASK) {
          RTEMS_SYSLOG("Unknown transmit error\n");
          ++emacPs->stats.tx_unknowns;
        }
        break;
      }
      default:
        RTEMS_SYSLOG("%-40s: Unknown error word value %lu\n", __func__, errorWord);
        break;
    }
  }
  PRINTK("%-40s: end\n", __func__);
}
