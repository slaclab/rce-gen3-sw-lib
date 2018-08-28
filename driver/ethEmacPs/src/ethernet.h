/**
 * @file
 *
 * @ingroup zynq_eth
 *
 * @brief Ethernet driver configuration.
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
 * Copyright (c) 2009
 * embedded brains GmbH
 * Obere Lagerstr. 30
 * D-82178 Puchheim
 * Germany
 * <rtems@embedded-brains.de>
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rtems.com/license/LICENSE.
 */

#ifndef LIBBSP_ARM_ZYNQ_ETHERNET_H
#define LIBBSP_ARM_ZYNQ_ETHERNET_H

#include <stdlib.h>
#include <limits.h>

#include <rtems.h>
#include <rtems/malloc.h>
#include <rtems/status-checks.h>
#include <rtems/rtems_bsdnet.h>
#include <rtems/rtems_mii_ioctl.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/mbuf.h>

#include <net/if.h>
#include <net/if_arp.h>

#include "xparameters.h"
#include "xemacps.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @defgroup zynq_eth Ethernet Support
 *
 * @ingroup zynq
 *
 * @brief Ethernet support.
 *
 * @{
 */

#define RTEMS_BSP_NETWORK_DRIVER_NAME "eth0"

#define ZYNQ_ETH_0                XPAR_XEMACPS_0

#define ZYNQ_IRQ_ETH_0            XPAR_XEMACPS_0_INTR

#define ZYNQ_ETH_DEVICE_ID        BSP_DEVICE_ID(ZYNQ_ETH_0)

#define ZYNQ_ETH_CONFIG_REG_BASE  BSP_BASEADDR(ZYNQ_ETH_0)

#define ZYNQ_ETH_CONFIG_INTERRUPT ZYNQ_IRQ_ETH_0

/*#define DEBUG 1*/
/*#define RECORD 1*/

/* Debug */
#ifdef DEBUG
  #include <stdio.h>
  #define PRINTF(...) printf(__VA_ARGS__)
  #define PRINTK(...) printk(__VA_ARGS__)
  #define DUMP2(...)  dump2(__VA_ARGS__)
  #define DUMP4(...)  dump4(__VA_ARGS__)
#else
  #define PRINTF(...)
  #define PRINTK(...)
  #define DUMP2(...)
  #define DUMP4(...)
#endif

#ifdef DEBUG
static void dump2(char* pfx, const char* f, bool swap, void* p, int l);
static void dump2(char* pfx, const char* f, bool swap, void* p, int l)
{
  int   i;
  uint16_t* q = (uint16_t*)p;

  printk("%-40s: %s ptr = %08x, len = %08x\n", f, pfx, (uint)p, l);

  l = (l + 1) >> 1;                     /* Convert to number of words */

  while (l > 0) {
    printk("%08x  ", (uint)q);
    for (i = 0; l > 0 && i < 8; ++q, --l, ++i) {
      printk("%04x ", (swap ? ntohs(*q) : *q));
    }
    printk("\n");
  }
}


static void dump4(char* pfx, const char* f, bool swap, void* p, int l);
static void dump4(char* pfx, const char* f, bool swap, void* p, int l)
{
  int   i;
  uint* q = (uint*)p;

  printk("%-40s: %s ptr = %08x, len = %08x\n", f, pfx, (uint)p, l);

  l = (l + 3) >> 2;                     /* Convert to number of words */

  while (l > 0) {
    printk("%08x  ", (uint)q);
    for (i = 0; l > 0 && i < 4; ++q, --l, ++i) {
      printk("%08x ", (swap ? ntohl(*q) : *q));
    }
    printk("\n");
  }
}
#endif


#define XEMACPS_BD_TO_INDEX(ring, bd)				\
  (((unsigned)(bd) - (unsigned)(ring)->BaseBdAddr) / (ring)->Separation)


#define RXBD_CNT 64
#define TXBD_CNT 256

#define ETH_EMACPS_LINKSPEED_AUTODETECT 0
#define ETH_EMACPS_LINKSPEED_1000       3
#define ETH_EMACPS_LINKSPEED_100        2
#define ETH_EMACPS_LINKSPEED_10         1

#define ETH_EMACPS_CONFIG_RX_UNIT_COUNT_DEFAULT 16
#define ETH_EMACPS_CONFIG_RX_UNIT_COUNT_MAX INT_MAX

#define ETH_EMACPS_CONFIG_TX_UNIT_COUNT_DEFAULT 32
#define ETH_EMACPS_CONFIG_TX_UNIT_COUNT_MAX INT_MAX

#define ETH_EMACPS_CONFIG_UNIT_MULTIPLE 8U

#define ETH_EMACPS_CONFIG_INTERRUPT ZYNQ_ETH_CONFIG_INTERRUPT
#define ETH_EMACPS_CONFIG_LINKSPEED ETH_EMACPS_LINKSPEED_1000 /*ETH_EMACPS_LINKSPEED_AUTODETECT */

#define ETH_EMACPS_RX_DATA_OFFSET 2

/* Events */
#define ETH_EMACPS_EVENT_INITIALIZE RTEMS_EVENT_1
#define ETH_EMACPS_EVENT_TXSTART    RTEMS_EVENT_2
#define ETH_EMACPS_EVENT_INTERRUPT  RTEMS_EVENT_3
#define ETH_EMACPS_EVENT_STOP       RTEMS_EVENT_4


typedef enum {
  ETH_EMACPS_STATE_NOT_INITIALIZED = 0,
  ETH_EMACPS_STATE_DOWN,
  ETH_EMACPS_STATE_UP
} EthEmacPsState;

/* TX Buffer Descriptor (BD) Status fields (UG585) */
#define EMACPS_ETH_TX_RETRIES           (0x1 << 29)
#define EMACPS_ETH_TX_AHB_ERROR         (0x1 << 27)
#define EMACPS_ETH_TX_LATE_COLLISION    (0x1 << 26)
#define EMACPS_ETH_TX_CKSUM_ERROR       (0x7 << 20)
#define EMACPS_ETH_TX_NO_ERROR          (0x0 << 20)
#define EMACPS_ETH_TX_VLAN_ERROR        (0x1 << 20)
#define EMACPS_ETH_TX_SNAP_ERROR        (0x2 << 20)
#define EMACPS_ETH_TX_IP_ERROR          (0x3 << 20)
#define EMACPS_ETH_TX_ID_ERROR          (0x4 << 20)
#define EMACPS_ETH_TX_BADFRAG_ERROR     (0x5 << 20)
#define EMACPS_ETH_TX_TCPUDP_ERROR      (0x6 << 20)
#define EMACPS_ETH_TX_EOP_ERROR         (0x7 << 20)

/* RX Buffer Descriptor (BD) Status fields (UG585) */
#define EMACPS_ETH_RX_BCAST             (0x1 << 30)
#define EMACPS_ETH_RX_MCAST             (0x1 << 29)
#define EMACPS_ETH_RX_UCAST             (0x1 << 28)
#define EMACPS_ETH_RX_EXTERNAL          (0x3 << 25)
#define EMACPS_ETH_RX_ADX1              (0x0 << 25)
#define EMACPS_ETH_RX_ADX2              (0x1 << 25)
#define EMACPS_ETH_RX_ADX3              (0x2 << 25)
#define EMACPS_ETH_RX_ADX4              (0x3 << 25)
#define EMACPS_ETH_RX_BIT_24            (0x1 << 24)
#define EMACPS_ETH_RX_BIT_22_23         (0x3 << 22)
#define EMACPS_ETH_RX_REG1              (0x0 << 22)
#define EMACPS_ETH_RX_REG2              (0x1 << 22)
#define EMACPS_ETH_RX_REG3              (0x2 << 22)
#define EMACPS_ETH_RX_REG4              (0x3 << 22)
#define EMACPS_ETH_RX_VLAN              (0x1 << 21)
#define EMACPS_ETH_RX_PRIORITY          (0x1 << 20)
#define EMACPS_ETH_RX_CFI               (0x1 << 16)
#define EMACPS_ETH_RX_EOF               (0x1 << 15)
#define EMACPS_ETH_RX_SOF               (0x1 << 14)
#define EMACPS_ETH_RX_FCS               (0x1 << 13)

#define EMACPS_ETH_TX_STAT_MASK         (EMACPS_ETH_TX_RETRIES        | \
                                         EMACPS_ETH_TX_AHB_ERROR      | \
                                         EMACPS_ETH_TX_LATE_COLLISION | \
                                         EMACPS_ETH_TX_CKSUM_ERROR    )

#define EMACPS_ETH_RX_STAT_MASK         (EMACPS_ETH_RX_BCAST          | \
                                         EMACPS_ETH_RX_MCAST          | \
                                         EMACPS_ETH_RX_UCAST          | \
                                         EMACPS_ETH_RX_EXTERNAL       | \
                                         EMACPS_ETH_RX_BIT_24         | \
                                         EMACPS_ETH_RX_BIT_22_23      | \
                                         EMACPS_ETH_RX_VLAN           | \
                                         EMACPS_ETH_RX_PRIORITY       | \
                                         EMACPS_ETH_RX_CFI            | \
                                         EMACPS_ETH_RX_EOF            | \
                                         EMACPS_ETH_RX_SOF            | \
                                         EMACPS_ETH_RX_FCS            )

struct EthEmacPsStats
{
  uint32_t rxd_frames;
  uint32_t rx_interrupts;

  uint32_t txd_frames;
  uint32_t tx_interrupts;

  uint32_t rx_drop;
  uint32_t rx_recv;
  uint32_t rx_memErr;
  uint32_t rx_dma_errors;
  uint32_t rx_overruns;
  uint32_t rx_no_buffers;
  uint32_t rx_unknowns;

  uint32_t rx_stat_bcast;
  uint32_t rx_stat_mcast;
  uint32_t rx_stat_ucast;
  uint32_t rx_stat_adx1;
  uint32_t rx_stat_adx2;
  uint32_t rx_stat_adx3;
  uint32_t rx_stat_adx4;
  uint32_t rx_stat_bit_24;
  uint32_t rx_stat_reg1;
  uint32_t rx_stat_reg2;
  uint32_t rx_stat_reg3;
  uint32_t rx_stat_reg4;
  uint32_t rx_stat_vlan;
  uint32_t rx_stat_priority;
  uint32_t rx_stat_cfi;
  uint32_t rx_stat_eof;
  uint32_t rx_stat_sof;
  uint32_t rx_stat_fcs;

  uint32_t tx_drop;
  uint32_t tx_xmit;
  uint32_t tx_dma_errors;
  uint32_t tx_underruns;
  uint32_t tx_no_buffers;
  uint32_t tx_excessive_retries;
  uint32_t tx_collisions;
  uint32_t tx_usedread;
  uint32_t tx_unknowns;
  uint32_t tx_overflows;
  uint32_t tx_full_queues;

  uint32_t tx_stat_retries;
  uint32_t tx_stat_ahb_errors;
  uint32_t tx_stat_late_collisions;
  uint32_t tx_stat_no_errors;
  uint32_t tx_stat_vlan_errors;
  uint32_t tx_stat_snap_errors;
  uint32_t tx_stat_ip_errors;
  uint32_t tx_stat_id_errors;
  uint32_t tx_stat_badfrag_errors;
  uint32_t tx_stat_tcpudp_errors;
  uint32_t tx_stat_eop_errors;
};

typedef struct {
  XEmacPs                  instance;
  struct arpcom            arpcom;
  EthEmacPsState           state;
  struct rtems_mdio_info   mdio;
  unsigned                 rxUnitCount;
  unsigned                 txUnitCount;
  struct mbuf            **rxMbufTable;
  struct mbuf            **txMbufTable;
  rtems_vector_number      irqNumber;
  rtems_id                 controlTask;

  /* Pointers to memory holding buffer descriptors (used only with SDMA) */
  void                    *rxBdSpace;
  void                    *txBdSpace;

  unsigned                 linkSpeed;

  struct EthEmacPsStats    stats;
} EthEmacPs;

static inline char *eth_emacps_config_alloc_table_area(size_t size)
{
  char* tableArea;

  PRINTK("%-40s: beg\n", __func__);
  tableArea = rtems_heap_allocate_aligned_with_boundary(size, 32, 0);
  PRINTK("%-40s: end - tableArea = %08x\n", __func__, (uint)tableArea);
  return tableArea;
}

static inline void eth_emacps_config_free_table_area(char *table_area)
{
  PRINTK("%-40s: beg\n", __func__);
  /* FIXME: Type */
  free(table_area, (int) 0xdeadbeef);
  PRINTK("%-40s: end\n", __func__);
}

/* ethEmacPs */
void              eth_emacps_low_level_init(void*);
rtems_status_code eth_emacps_output(EthEmacPs*, struct mbuf*);
int               eth_emacps_input(EthEmacPs*, XEmacPs_BdRing*);
void              eth_emacps_handle_error(EthEmacPs*);
void              eth_emacps_handle_tx_errors(EthEmacPs*);
void              eth_emacps_error_task(void *);

/* ethEmacPs_dma */
int               eth_emacps_setup_rx_dscs(EthEmacPs*, XEmacPs_BdRing*);
int               eth_emacps_setup_tx_dscs(EthEmacPs*, XEmacPs_BdRing*);
void              eth_emacps_process_rxd_bds(EthEmacPs*, XEmacPs_BdRing*, int);
void              eth_emacps_process_txd_bds(EthEmacPs*, XEmacPs_BdRing*);
void              eth_emacps_free_tx_rx_mbufs(EthEmacPs*);
void              eth_emacps_free_only_tx_mbufs(EthEmacPs*);
rtems_status_code eth_emacps_sgsend(EthEmacPs*, struct mbuf*);
int               eth_emacps_init_dma(EthEmacPs*);
void              eth_emacps_send_handler(void*);
void              eth_emacps_recv_handler(void*);

/* ethEmacPs_hw */
void              eth_emacps_start(EthEmacPs*);
void              eth_emacps_stop(EthEmacPs*);
void              eth_emacps_reset(EthEmacPs*);
int               eth_emacps_init_mac(EthEmacPs*);
int               eth_emacps_setup_isr(EthEmacPs*);
void              eth_emacps_init_on_error(EthEmacPs*);
void              eth_emacps_error_handler(void*, uint8_t, uint32_t);

/* ethEmacPs_phySpeed */
unsigned          eth_emacps_phy_setup(EthEmacPs*);
unsigned          eth_emacps_get_IEEE_phy_speed(EthEmacPs*);
unsigned          eth_emacps_configure_IEEE_phy_speed(EthEmacPs*, unsigned);

/* ethernet */
struct mbuf      *eth_emacps_add_new_mbuf(EthEmacPs*, struct mbuf**, int);
int               zynq_eth_attach_detach(struct rtems_bsdnet_ifconfig*, int);

extern rtems_id  gEthEmacPsRxTask;
extern rtems_id  gEthEmacPsTxTask;
extern rtems_id  gEthEmacPsErTask;

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBBSP_ARM_ZYNQ_ETHERNET_H */
