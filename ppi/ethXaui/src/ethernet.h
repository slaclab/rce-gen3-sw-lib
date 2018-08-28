/**
 * @file
 *
 * @ingroup xaui_eth
 *
 * @brief Xaui Ethernet driver configuration.
 */

/*
 *                              Copyright 2015
 *                                     by
 *                        The Board of Trustees of the
 *                      Leland Stanford Junior University.
 *
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

#ifndef XAUI_ETHERNET_H
#define XAUI_ETHERNET_H

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

#include "xaui/Xaui.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define RTEMS_BSP_NETWORK_DRIVER_NAME "eth0"

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

#define RXBD_CNT 504
#define TXBD_CNT 504

#define ETH_XAUI_RX_DATA_OFFSET 2

#define ETH_XAUI_RX_BUF_CNT 64
#define ETH_XAUI_TX_BUF_CNT 64

#define ETHERNET_TYPE_IPV4 0x0800
#define ETHERNET_TYPE_ARP  0x0806

/* Events */
#define ETH_XAUI_EVENT_INITIALIZE RTEMS_EVENT_1
#define ETH_XAUI_EVENT_TXSTART    RTEMS_EVENT_2
#define ETH_XAUI_EVENT_INTERRUPT  RTEMS_EVENT_3
#define ETH_XAUI_EVENT_STOP       RTEMS_EVENT_4


typedef enum {
  ETH_XAUI_STATE_NOT_INITIALIZED = 0,
  ETH_XAUI_STATE_DOWN,
  ETH_XAUI_STATE_UP
} EthXauiState;

struct EthXauiStats
{
  uint32_t rxd_frames;
  uint32_t rx_interrupts;

  uint32_t txd_frames;
  uint32_t tx_interrupts;

  uint32_t rx_drop;
  uint32_t rx_recv;
  uint32_t rx_memErr;

  uint32_t tx_drop;
  uint32_t tx_xmit;
  uint32_t tx_no_buffers;
  uint32_t tx_full_queues;
};

typedef struct {
  struct arpcom            arpcom;
  EthXauiState             state;
  struct rtems_mdio_info   mdio;
  unsigned                 txUnitCount;
  struct mbuf            **txMbufTable;
  rtems_id                 controlTask;

  unsigned                 linkSpeed;

  struct EthXauiStats      stats;

  unsigned                 txLast;
  unsigned                 txfree;
  unsigned                 txIdx;
  
  Xaui_RxDevice            rxDevice;
  Xaui_TxDevice            txDevice;
  
} EthXaui;

static inline char *eth_xaui_config_alloc_table_area(size_t size)
{
  char* tableArea;

  PRINTK("%-40s: beg\n", __func__);
  tableArea = rtems_heap_allocate_aligned_with_boundary(size, 32, 0);
  PRINTK("%-40s: end - tableArea = %08x\n", __func__, (uint)tableArea);
  return tableArea;
}

static inline void eth_xaui_config_free_table_area(char *table_area)
{
  PRINTK("%-40s: beg\n", __func__);
  /* FIXME: Type */
  free(table_area, (int) 0xdeadbeef);
  PRINTK("%-40s: end\n", __func__);
}

/* ethXaui */
void              eth_xaui_low_level_init(void*);
rtems_status_code eth_xaui_output(EthXaui*, struct mbuf*);

/* ethXaui_dma */
void              eth_xaui_process_rx(EthXaui*, struct ether_header*, unsigned);
void              eth_xaui_process_tx(EthXaui*);
void              eth_xaui_free_rx(void* buffer, unsigned size);
rtems_status_code eth_xaui_sgsend(EthXaui*, struct mbuf*);
void              eth_xaui_recv_handler(void*);

/* ethXaui_hw */
void              eth_xaui_start(EthXaui*);
void              eth_xaui_stop(EthXaui*);
void              eth_xaui_reset(EthXaui*);
int               eth_xaui_init_mac(EthXaui*);

/* ethernet */
struct mbuf *eth_xaui_next_fragment(struct ifnet *ifp,
                                    struct mbuf  *m);

int               xaui_eth_attach_detach(struct rtems_bsdnet_ifconfig*, int);

extern rtems_id   gEthXauiTxTask;

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XAUI_ETHERNET_H */
