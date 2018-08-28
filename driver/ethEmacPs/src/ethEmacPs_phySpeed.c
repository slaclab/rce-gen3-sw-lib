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
 * Copyright (c) 2007-2008, Advanced Micro Devices, Inc.
 *               All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in
 *      the documentation and/or other materials provided with the
 *      distribution.
 *    * Neither the name of Advanced Micro Devices, Inc. nor the names
 *      of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written
 *      permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Some portions copyright (c) 2010 Xilinx, Inc.  All rights reserved.
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

#include "ethernet.h"

/* Advertisement control register. */
#define ADVERTISE_10HALF      0x0020  /* Try for 10mbps half-duplex  */
#define ADVERTISE_10FULL      0x0040  /* Try for 10mbps full-duplex  */
#define ADVERTISE_100HALF     0x0080  /* Try for 100mbps half-duplex */
#define ADVERTISE_100FULL     0x0100  /* Try for 100mbps full-duplex */

#define ADVERTISE_100_AND_10  (ADVERTISE_10FULL  | ADVERTISE_100FULL | \
                               ADVERTISE_10HALF  | ADVERTISE_100HALF)
#define ADVERTISE_100         (ADVERTISE_100FULL | ADVERTISE_100HALF)
#define ADVERTISE_10          (ADVERTISE_10FULL  | ADVERTISE_10HALF)

#define ADVERTISE_1000        0x0300


#define IEEE_CONTROL_REG_OFFSET                  0
#define IEEE_STATUS_REG_OFFSET                   1
#define IEEE_AUTONEGO_ADVERTISE_REG              4
#define IEEE_PARTNER_ABILITIES_1_REG_OFFSET      5
#define IEEE_1000_ADVERTISE_REG_OFFSET           9
#define IEEE_PARTNER_ABILITIES_3_REG_OFFSET     10
#define IEEE_COPPER_SPECIFIC_CONTROL_REG        16
#define IEEE_SPECIFIC_STATUS_REG                17
#define IEEE_COPPER_SPECIFIC_STATUS_REG_2       19
#define IEEE_CONTROL_REG_MAC                    21
#define IEEE_PAGE_ADDRESS_REGISTER              22


#define IEEE_CTRL_1GBPS_LINKSPEED_MASK      0x2040
#define IEEE_CTRL_LINKSPEED_MASK            0x0040
#define IEEE_CTRL_LINKSPEED_1000M           0x0040
#define IEEE_CTRL_LINKSPEED_100M            0x2000
#define IEEE_CTRL_LINKSPEED_10M             0x0000
#define IEEE_CTRL_RESET_MASK                0x8000
#define IEEE_CTRL_AUTONEGOTIATE_ENABLE      0x1000
#define IEEE_STAT_AUTONEGOTIATE_CAPABLE     0x0008
#define IEEE_STAT_AUTONEGOTIATE_COMPLETE    0x0020
#define IEEE_STAT_AUTONEGOTIATE_RESTART     0x0200
#define IEEE_STAT_1GBPS_EXTENSIONS          0x0100
#define IEEE_AN1_ABILITY_MASK               0x1FE0
#define IEEE_AN3_ABILITY_MASK_1GBPS         0x0C00
#define IEEE_AN1_ABILITY_MASK_100MBPS       0x0380
#define IEEE_AN1_ABILITY_MASK_10MBPS        0x0060
#define IEEE_RGMII_TXRX_CLOCK_DELAYED_MASK  0x0030

#define IEEE_ASYMMETRIC_PAUSE_MASK          0x0800
#define IEEE_PAUSE_MASK                     0x0400
#define IEEE_AUTONEG_ERROR_MASK             0x8000

#define PHY_DETECT_REG                           1
#define PHY_DETECT_MASK                     0x1808

/* Frequency setting */
#define SLCR_LOCK_ADDR                      (XPS_SYS_CTRL_BASEADDR + 0x4)
#define SLCR_UNLOCK_ADDR                    (XPS_SYS_CTRL_BASEADDR + 0x8)
#define SLCR_GEM0_CLK_CTRL_ADDR             (XPS_SYS_CTRL_BASEADDR + 0x140)
#define SLCR_GEM1_CLK_CTRL_ADDR             (XPS_SYS_CTRL_BASEADDR + 0x144)
#define SLCR_LOCK_KEY_VALUE                 0x767B
#define SLCR_UNLOCK_KEY_VALUE               0xDF0D
#define SLCR_ADDR_GEM_RST_CTRL              (XPS_SYS_CTRL_BASEADDR + 0x214)
#define EMACPS_SLCR_DIV_MASK                0xFC0FC0FF

#define EMAC0_BASE_ADDRESS                  0xE000B000
#define EMAC1_BASE_ADDRESS                  0xE000C000


static int eth_emacps_detect_phy(EthEmacPs *emacPs)
{
  uint16_t phy_reg;
  unsigned phy_addr;

  for (phy_addr = 31; phy_addr > 0; phy_addr--) {
    XEmacPs_PhyRead(&emacPs->instance, phy_addr, PHY_DETECT_REG, &phy_reg);

    if ((phy_reg != 0xFFFF) &&
        ((phy_reg & PHY_DETECT_MASK) == PHY_DETECT_MASK)) {

      /* Found a valid PHY address */
      PRINTK("%-40s: PHY detected at address %d\n", __func__, phy_addr);
      PRINTK("%-40s: PHY detected\n", __func__);

      return phy_addr;
    }
  }

  RTEMS_SYSLOG("No PHY detected.  Assuming a PHY at address 0\n");

  /* default to zero */
  return 0;
}


unsigned eth_emacps_get_IEEE_phy_speed(EthEmacPs *emacPs)
{
  uint16_t tmp;
  uint16_t control;
  uint16_t status;
  uint16_t partner_capabilities;
  unsigned phy_addr = eth_emacps_detect_phy(emacPs);
  XEmacPs* instance = &emacPs->instance;

  PRINTK("Start PHY autonegotiation\n");

  XEmacPs_PhyWrite(instance, phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 2);
  XEmacPs_PhyRead(instance, phy_addr, IEEE_CONTROL_REG_MAC, &control);
  control |= IEEE_RGMII_TXRX_CLOCK_DELAYED_MASK;
  XEmacPs_PhyWrite(instance, phy_addr, IEEE_CONTROL_REG_MAC, control);

  XEmacPs_PhyWrite(instance, phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 0);

  XEmacPs_PhyRead(instance, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, &control);
  control |= IEEE_ASYMMETRIC_PAUSE_MASK;
  control |= IEEE_PAUSE_MASK;
  control |= ADVERTISE_100;
  control |= ADVERTISE_10;
  XEmacPs_PhyWrite(instance, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, control);

  XEmacPs_PhyRead(instance, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET, &control);
  control |= ADVERTISE_1000;
  XEmacPs_PhyWrite(instance, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET, control);

  XEmacPs_PhyWrite(instance, phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 0);
  XEmacPs_PhyRead(instance, phy_addr, IEEE_COPPER_SPECIFIC_CONTROL_REG, &control);
  control |= (7 << 12);                 /* Max number of gigabit attempts */
  control |= (1 << 11);                 /* Enable downshift */
  XEmacPs_PhyWrite(instance, phy_addr, IEEE_COPPER_SPECIFIC_CONTROL_REG, control);

  XEmacPs_PhyRead(instance, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
  control |= IEEE_CTRL_AUTONEGOTIATE_ENABLE;
  control |= IEEE_STAT_AUTONEGOTIATE_RESTART;
  XEmacPs_PhyWrite(instance, phy_addr, IEEE_CONTROL_REG_OFFSET, control);

  XEmacPs_PhyRead(instance, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
  control |= IEEE_CTRL_RESET_MASK;
  XEmacPs_PhyWrite(instance, phy_addr, IEEE_CONTROL_REG_OFFSET, control);

  for (;;) {
    XEmacPs_PhyRead(instance, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
    if (control & IEEE_CTRL_RESET_MASK)
      continue;
    else
      break;
  }

  RTEMS_SYSLOG("Waiting for PHY to complete autonegotiation\n");

  XEmacPs_PhyRead(instance, phy_addr, IEEE_STATUS_REG_OFFSET, &status);
  while ( !(status & IEEE_STAT_AUTONEGOTIATE_COMPLETE) ) {
    sleep(1);
    XEmacPs_PhyRead(instance, phy_addr, IEEE_COPPER_SPECIFIC_STATUS_REG_2, &tmp);
    if (tmp & IEEE_AUTONEG_ERROR_MASK) {
      RTEMS_SYSLOG("Autonegotiation error\n");
    }
    XEmacPs_PhyRead(instance, phy_addr, IEEE_STATUS_REG_OFFSET, &status);
  }

  RTEMS_SYSLOG("Autonegotiation complete\n");

  XEmacPs_PhyRead(instance, phy_addr, IEEE_SPECIFIC_STATUS_REG, &partner_capabilities);
  if      ( ((partner_capabilities >> 14) & 3) == 2)  /* 1000Mbps */
    return 1000;
  else if ( ((partner_capabilities >> 14) & 3) == 1)  /* 100Mbps */
    return 100;
  else                                                /* 10Mbps */
    return 10;
}


unsigned eth_emacps_configure_IEEE_phy_speed(EthEmacPs *emacPs, unsigned speed)
{
  uint16_t control;
  XEmacPs *instance = &emacPs->instance;
  unsigned phy_addr = eth_emacps_detect_phy(emacPs);

  XEmacPs_PhyWrite(instance, phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 2);
  XEmacPs_PhyRead(instance, phy_addr, IEEE_CONTROL_REG_MAC, &control);
  control |= IEEE_RGMII_TXRX_CLOCK_DELAYED_MASK;
  XEmacPs_PhyWrite(instance, phy_addr, IEEE_CONTROL_REG_MAC, control);

  XEmacPs_PhyWrite(instance, phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 0);

  XEmacPs_PhyRead(instance, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, &control);
  control |= IEEE_ASYMMETRIC_PAUSE_MASK;
  control |= IEEE_PAUSE_MASK;
  XEmacPs_PhyWrite(instance, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, control);

  XEmacPs_PhyRead(instance, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
  control &= ~IEEE_CTRL_LINKSPEED_1000M;
  control &= ~IEEE_CTRL_LINKSPEED_100M;
  control &= ~IEEE_CTRL_LINKSPEED_10M;

  if (speed == 1000) {
    control |= IEEE_CTRL_LINKSPEED_1000M;
  }

  else if (speed == 100) {
    control |= IEEE_CTRL_LINKSPEED_100M;
    /* Dont advertise PHY speed of 1000 Mbps */
    XEmacPs_PhyWrite(instance, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET, 0);
    /* Dont advertise PHY speed of 10 Mbps */
    XEmacPs_PhyWrite(instance, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, ADVERTISE_100);
  }

  else if (speed == 10) {
    control |= IEEE_CTRL_LINKSPEED_10M;
    /* Dont advertise PHY speed of 1000 Mbps */
    XEmacPs_PhyWrite(instance, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET, 0);
    /* Dont advertise PHY speed of 100 Mbps */
    XEmacPs_PhyWrite(instance, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, ADVERTISE_10);
  }

  XEmacPs_PhyWrite(instance, phy_addr, IEEE_CONTROL_REG_OFFSET,
                   control | IEEE_CTRL_RESET_MASK);
  sleep(1);

  return 0;
}


static void eth_emacps_setup_slcr_divisors(int mac_baseaddr, int speed)
{
  volatile unsigned slcrBaseAddress;
  unsigned          SlcrDiv0 = 0;       /* Stop compiler warnings */
  unsigned          SlcrDiv1 = 0;       /* Stop compiler warnings */
  unsigned          SlcrTxClkCntrl;

  *(volatile unsigned *)(SLCR_UNLOCK_ADDR) = SLCR_UNLOCK_KEY_VALUE;

  if (mac_baseaddr == EMAC0_BASE_ADDRESS) {
    slcrBaseAddress = SLCR_GEM0_CLK_CTRL_ADDR;
  } else {
    slcrBaseAddress = SLCR_GEM1_CLK_CTRL_ADDR;
  }
  if (speed == 1000) {
    if (mac_baseaddr == EMAC0_BASE_ADDRESS) {
#ifdef XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV0
      SlcrDiv0 = XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV0;
      SlcrDiv1 = XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV1;
#endif
    } else {
#ifdef XPAR_PS7_ETHERNET_1_ENET_SLCR_1000MBPS_DIV0
      SlcrDiv0 = XPAR_PS7_ETHERNET_1_ENET_SLCR_1000MBPS_DIV0;
      SlcrDiv1 = XPAR_PS7_ETHERNET_1_ENET_SLCR_1000MBPS_DIV1;
#endif
    }
  } else if (speed == 100) {
    if (mac_baseaddr == EMAC0_BASE_ADDRESS) {
#ifdef XPAR_PS7_ETHERNET_0_ENET_SLCR_100MBPS_DIV0
      SlcrDiv0 = XPAR_PS7_ETHERNET_0_ENET_SLCR_100MBPS_DIV0;
      SlcrDiv1 = XPAR_PS7_ETHERNET_0_ENET_SLCR_100MBPS_DIV1;
#endif
    } else {
#ifdef XPAR_PS7_ETHERNET_1_ENET_SLCR_100MBPS_DIV0
      SlcrDiv0 = XPAR_PS7_ETHERNET_1_ENET_SLCR_100MBPS_DIV0;
      SlcrDiv1 = XPAR_PS7_ETHERNET_1_ENET_SLCR_100MBPS_DIV1;
#endif
    }
  } else {
    if (mac_baseaddr == EMAC0_BASE_ADDRESS) {
#ifdef XPAR_PS7_ETHERNET_0_ENET_SLCR_10MBPS_DIV0
      SlcrDiv0 = XPAR_PS7_ETHERNET_0_ENET_SLCR_10MBPS_DIV0;
      SlcrDiv1 = XPAR_PS7_ETHERNET_0_ENET_SLCR_10MBPS_DIV1;
#endif
    } else {
#ifdef XPAR_PS7_ETHERNET_1_ENET_SLCR_10MBPS_DIV0
      SlcrDiv0 = XPAR_PS7_ETHERNET_1_ENET_SLCR_10MBPS_DIV0;
      SlcrDiv1 = XPAR_PS7_ETHERNET_1_ENET_SLCR_10MBPS_DIV1;
#endif
    }
  }
  SlcrTxClkCntrl  = *(volatile unsigned *)(slcrBaseAddress);
  SlcrTxClkCntrl &= EMACPS_SLCR_DIV_MASK;
  SlcrTxClkCntrl |= (SlcrDiv1 << 20);
  SlcrTxClkCntrl |= (SlcrDiv0 << 8);
  *(volatile unsigned *)(slcrBaseAddress) = SlcrTxClkCntrl;
  *(volatile unsigned *)(SLCR_LOCK_ADDR)  = SLCR_LOCK_KEY_VALUE;
}


unsigned eth_emacps_phy_setup(EthEmacPs *emacPs)
{
  unsigned baseAdx = emacPs->instance.Config.BaseAddress;
  unsigned linkSpeed;

#if defined(ETH_EMACPS_CONFIG_LINKSPEED)
#  if   (ETH_EMACPS_CONFIG_LINKSPEED == ETH_EMACPS_LINKSPEED_AUTODETECT)
  linkSpeed = eth_emacps_get_IEEE_phy_speed(emacPs);
  if        (linkSpeed == 1000) {
    eth_emacps_setup_slcr_divisors(baseAdx, 1000);
  } else if (linkSpeed == 100) {
    eth_emacps_setup_slcr_divisors(baseAdx, 100);
  } else {
    eth_emacps_setup_slcr_divisors(baseAdx, 10);
  }
#  elif (ETH_EMACPS_CONFIG_LINKSPEED == ETH_EMACPS_LINKSPEED_1000)
  eth_emacps_setup_slcr_divisors(baseAdx, 1000);
  linkSpeed = 1000;
  eth_emacps_configure_IEEE_phy_speed(emacPs, linkSpeed);
  sleep(1);
#  elif (ETH_EMACPS_CONFIG_LINKSPEED == ETH_EMACPS_LINKSPEED_100)
  eth_emacps_setup_slcr_divisors(baseAdx, 100);
  linkSpeed = 100;
  eth_emacps_configure_IEEE_phy_speed(emacPs, linkSpeed);
  sleep(1);
#  elif (ETH_EMACPS_CONFIG_LINKSPEED == ETH_EMACPS_LINKSPEED_10)
  eth_emacps_setup_slcr_divisors(baseAdx, 10);
  linkSpeed = 10;
  eth_emacps_configure_IEEE_phy_speed(emacPs, linkSpeed);
  sleep(1);
#  endif
#else
#  error ETH_EMACPS_CONFIG_LINKSPEED must be defined.
#endif

  RTEMS_SYSLOG("Link speed: %d\n", linkSpeed);
  return linkSpeed;
}
