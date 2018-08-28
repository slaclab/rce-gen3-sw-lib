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
 * Copyright (c) 2009-2012 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Obere Lagerstr. 30
 *  82178 Puchheim
 *  Germany
 *  <rtems@embedded-brains.de>
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rtems.com/license/LICENSE.
 */

#define __INSIDE_RTEMS_BSD_TCPIP_STACK__ 1
#define __BSD_VISIBLE                    1

#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "network/BsdNet_Config.h"
#include "elf/linker.h"
#include "svt/Svt.h"    

#include "ethernet.h"

#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

#include <bsp.h>
#include <bsp/irq.h>
#include <bsp/utility.h>

//#include "xil_cache.h"
//#include "xil_cache_l.h"

#if MCLBYTES > (2 * 1024)
  #error "MCLBYTES too large"
#endif

#define ETH_EMACPS_CONFIG_TX_BUF_SIZE sizeof(struct mbuf *)

#define DEFAULT_PHY      0
#define WATCHDOG_TIMEOUT 5

/* Status */
#define ETH_EMACPS_LAST_FRAGMENT_FLAG 0x8000000u

static EthEmacPs gEthEmacPs[XPAR_XEMACPS_NUM_INSTANCES];

rtems_id gEthEmacPsRxTask = 0;
rtems_id gEthEmacPsTxTask = 0;
rtems_id gEthEmacPsErTask = 0;

#ifdef RECORD
struct {
  struct mbuf *alloc;
  unsigned     ctrl;
  struct mbuf *free;
  unsigned     status;
}        mbufList[1024];
unsigned mbufIdx     = 0;
unsigned mbufFreeIdx = 0;
#endif


static void eth_emacps_control_request_complete(const EthEmacPs *emacPs)
{
  PRINTK("%-40s: beg\n", __func__);
  rtems_status_code sc = rtems_event_transient_send(emacPs->controlTask);
  assert(sc == RTEMS_SUCCESSFUL);
  PRINTK("%-40s: end\n", __func__);
}

static void eth_emacps_control_request(EthEmacPs      *emacPs,
                                       rtems_id        task,
                                       rtems_event_set event)
{
  PRINTK("%-40s: beg\n", __func__);
  rtems_status_code sc = RTEMS_SUCCESSFUL;
  uint32_t nest_count = 0;

  emacPs->controlTask = rtems_task_self();

  sc = rtems_bsdnet_event_send(task, event);
  assert(sc == RTEMS_SUCCESSFUL);

  nest_count = rtems_bsdnet_semaphore_release_recursive();
  sc = rtems_event_transient_receive(RTEMS_WAIT, RTEMS_NO_TIMEOUT);
  assert(sc == RTEMS_SUCCESSFUL);
  rtems_bsdnet_semaphore_obtain_recursive(nest_count);

  emacPs->controlTask = 0;
  PRINTK("%-40s: end\n", __func__);
}

static inline uint32_t eth_emacps_increment( uint32_t value,
                                             uint32_t cycle )
{
  if (value < cycle) {
    return ++value;
  } else {
    return 0;
  }
}

static void eth_emacps_enable_promiscous_mode(EthEmacPs* emacPs, bool enable)
{
  PRINTK("%-40s: beg\n", __func__);
  if (enable) {
    XEmacPs_SetOptions(&emacPs->instance, (XEMACPS_PROMISC_OPTION   |
                                           XEMACPS_MULTICAST_OPTION |
                                           XEMACPS_BROADCAST_OPTION));
  } else {
    XEmacPs_ClearOptions(&emacPs->instance, (XEMACPS_PROMISC_OPTION   |
                                             XEMACPS_MULTICAST_OPTION |
                                             XEMACPS_BROADCAST_OPTION));
  }
  PRINTK("%-40s: end\n", __func__);
}

static inline void eth_emacps_enable_receive_interrupts(EthEmacPs* emacPs)
{
  rtems_interrupt_level level;

  PRINTK("%-40s: beg\n", __func__);

  rtems_interrupt_disable(level);
  XEmacPs_IntEnable(&emacPs->instance, (XEMACPS_IXR_FRAMERX_MASK |
                                        XEMACPS_IXR_RX_ERR_MASK));
  rtems_interrupt_enable(level);

  PRINTK("%-40s: end\n", __func__);
}

static inline void eth_emacps_disable_receive_interrupts(EthEmacPs* emacPs)
{
  rtems_interrupt_level level;

  PRINTK("%-40s: beg\n", __func__);

  rtems_interrupt_disable(level);
  XEmacPs_IntDisable(&emacPs->instance, (XEMACPS_IXR_FRAMERX_MASK |
                                         XEMACPS_IXR_RX_ERR_MASK));
  rtems_interrupt_enable(level);

  PRINTK("%-40s: end\n", __func__);
}

static inline void eth_emacps_enable_transmit_interrupts(EthEmacPs* emacPs)
{
  rtems_interrupt_level level;

  PRINTK("%-40s: beg\n", __func__);

  rtems_interrupt_disable(level);
  XEmacPs_IntEnable(&emacPs->instance, (XEMACPS_IXR_TXCOMPL_MASK |
                                        XEMACPS_IXR_TX_ERR_MASK));
  rtems_interrupt_enable(level);

  PRINTK("%-40s: end\n", __func__);
}

static inline void eth_emacps_disable_transmit_interrupts(EthEmacPs* emacPs)
{
  rtems_interrupt_level level;

  PRINTK("%-40s: beg\n", __func__);

  rtems_interrupt_disable(level);
  XEmacPs_IntDisable(&emacPs->instance, (XEMACPS_IXR_TXCOMPL_MASK |
                                         XEMACPS_IXR_TX_ERR_MASK));
  rtems_interrupt_enable(level);

  PRINTK("%-40s: end\n", __func__);
}


static struct mbuf *eth_emacps_new_mbuf(struct ifnet *ifp, int wait)
{
  struct mbuf *m  = NULL;
  int          mw = wait ? M_WAIT : M_DONTWAIT;

  MGETHDR(m, mw, MT_DATA);
  if (m != NULL) {
    MCLGET(m, mw);
    if ((m->m_flags & M_EXT) != 0) {
      /* Set receive interface */
      m->m_pkthdr.rcvif = ifp;

      /* Adjust by two bytes for proper IP header alignment */
      m->m_data = mtod(m, char *) + ETH_EMACPS_RX_DATA_OFFSET;

      return m;
    } else {
      m_free(m);
    }
  }

  return NULL;
}

struct mbuf *eth_emacps_add_new_mbuf( EthEmacPs    *emacPs,
                                      struct mbuf **mbufs,
                                      int           wait )
{
  struct ifnet *const ifp = &emacPs->arpcom.ac_if;
  struct mbuf        *m   = eth_emacps_new_mbuf(ifp, wait);

  /* Check mbuf */
  if (m != NULL) {
    /* Invalidate cache to insure fresh data is picked up from the DMA */
#if 1
    rtems_cache_invalidate_multiple_data_lines(mtod(m, void *),
                                               MCLBYTES - ETH_EMACPS_RX_DATA_OFFSET);
#else
    Xil_DCacheInvalidateRange((unsigned)mtod(m, void *),
                              MCLBYTES - ETH_EMACPS_RX_DATA_OFFSET);
#endif
  }
  return m;
}

static void eth_emacps_receive_task(void *arg)
{
  rtems_status_code sc     = RTEMS_SUCCESSFUL;
  rtems_event_set   events = 0;
  EthEmacPs  *const emacPs = (EthEmacPs *)arg;
  XEmacPs_BdRing   *rxRing = &XEmacPs_GetRxRing(&emacPs->instance);
  unsigned          tooLow = emacPs->rxUnitCount - 4; /* Some number... */

  PRINTK("%-40s: beg\n", __func__);

  /* Main event loop */
  for (;;) {
    PRINTK("%-40s: Waiting...\n", __func__);
    /* Wait for events */
    sc = rtems_bsdnet_event_receive( ETH_EMACPS_EVENT_INITIALIZE |
                                     ETH_EMACPS_EVENT_STOP       |
                                     ETH_EMACPS_EVENT_INTERRUPT,
                                     RTEMS_EVENT_ANY | RTEMS_WAIT,
                                     RTEMS_NO_TIMEOUT,
                                     &events );
    assert(sc == RTEMS_SUCCESSFUL);

    PRINTK("%-40s: WAKE up: 0x%08" PRIx32 "\n", __func__, events);

    /* Stop receiver? */
    if ((events & ETH_EMACPS_EVENT_STOP) != 0) {
      PRINTK("%-40s: STOP\n", __func__);
      eth_emacps_control_request_complete(emacPs);

      PRINTK("%-40s: STOP done\n", __func__);

      /* Wait for events */
      continue;
    }

    /* Initialize receiver? */
    if ((events & ETH_EMACPS_EVENT_INITIALIZE) != 0) {
      PRINTK("%-40s: INIT\n", __func__);
      /* Disable receive interrupts */
      eth_emacps_disable_receive_interrupts(emacPs);

      /* Fill receive queue */
      eth_emacps_process_rxd_bds(emacPs, rxRing, M_WAIT);

      /* Enable receive interrupts */
      eth_emacps_enable_receive_interrupts(emacPs);

      eth_emacps_control_request_complete(emacPs);

      PRINTK("%-40s: INIT done\n", __func__);

      /* Wait for events */
      continue;
    }

    /* Event must be a ETH_EMACPS_EVENT_INTERRUPT */
    PRINTK("%-40s: INTERRUPT\n", __func__);

    for (;;) {
      unsigned nBds = eth_emacps_input(emacPs, rxRing);
      bool     wait = XEmacPs_BdRingGetFreeCnt(rxRing) < tooLow ? M_WAIT
                                                                : M_DONTWAIT;
      PRINTK("%-40s: nBds = %08x\n", __func__, nBds);

      /* Increment received frames counter */
      emacPs->stats.rxd_frames += nBds;

      /* Refill the empty slots in the receive queue */
      eth_emacps_process_rxd_bds(emacPs, rxRing, wait);

      if (nBds == 0) {
        /* Nothing to do, enable receive interrupts */
        eth_emacps_enable_receive_interrupts(emacPs);
        break;
      }
    }
    PRINTK("%-40s: INTERRUPT done\n", __func__);
  }

  PRINTK("%-40s: end\n", __func__);
}

static struct mbuf *eth_emacps_next_fragment(struct ifnet *ifp,
                                             struct mbuf  *m)
{
  struct mbuf *n = NULL;
  int size = 0;

  PRINTK("%-40s: beg, ifp = %08x, m = %08x\n", __func__,
         (uint)ifp, (uint)m);

  for (;;) {
    if (m == NULL) {
      /* Dequeue first fragment of the next frame */
      IF_DEQUEUE(&ifp->if_snd, m);

      /* Empty queue? */
      if (m == NULL) {
        return m;
      }
    }

    /* Get fragment size */
    size = m->m_len;

    if (size > 0) {
      /* Now we have a non-empty fragment */
      break;
    } else {
      /* Discard empty fragments */
      m = m_free(m);
    }
  }

  /* Discard empty successive fragments */
  n = m->m_next;
  while (n != NULL && (n->m_len <= 0)) { // || n->m_len >= XEMACPS_MAX_FRAME_SIZE)) {
    n = m_free(n);
  }
  m->m_next = n;

  PRINTK("%-40s: end, m = %08x, d = %08x, len = %08x, n = %08x\n", __func__,
         (uint)m, mtod(m, uint), m->m_len, (uint)m->m_next);

  return m;
}

static void eth_emacps_transmit_task(void *arg)
{
  rtems_status_code  sc       = RTEMS_SUCCESSFUL;
  rtems_event_set    events   = 0;
  EthEmacPs         *emacPs   = (EthEmacPs *) arg;
  XEmacPs_BdRing    *txRing   = &XEmacPs_GetTxRing(&emacPs->instance);
  struct ifnet      *ifp      = &emacPs->arpcom.ac_if;
  struct mbuf       *m        = NULL;
  struct mbuf       *m0       = NULL;

  PRINTK("%-40s: beg\n", __func__);

  /* Main event loop */
  for (;;) {
    PRINTK("%-40s: Waiting...\n", __func__);
    /* Wait for events */
    sc = rtems_bsdnet_event_receive( ETH_EMACPS_EVENT_INITIALIZE |
                                     ETH_EMACPS_EVENT_STOP       |
                                     ETH_EMACPS_EVENT_TXSTART    |
                                     ETH_EMACPS_EVENT_INTERRUPT,
                                     RTEMS_EVENT_ANY | RTEMS_WAIT,
                                     RTEMS_NO_TIMEOUT,
                                     &events );
    assert(sc == RTEMS_SUCCESSFUL);

    PRINTK("%-40s: WAKE up: 0x%08" PRIx32 "\n", __func__, events);

    /* Stop transmitter? */
    if ((events & ETH_EMACPS_EVENT_STOP) != 0) {
      PRINTK("%-40s: STOP\n", __func__);
      eth_emacps_control_request_complete(emacPs);

      PRINTK("%-40s: STOP done\n", __func__);

      /* Wait for events */
      continue;
    }

    /* Initialize transmitter? */
    if ((events & ETH_EMACPS_EVENT_INITIALIZE) != 0) {
      PRINTK("%-40s: INIT\n", __func__);
      /* Disable transmit interrupts */
      eth_emacps_disable_transmit_interrupts(emacPs);

      /* Discard outstanding fragments (= data loss) */
      eth_emacps_process_txd_bds(emacPs, txRing);

      eth_emacps_control_request_complete(emacPs);

      PRINTK("%-40s: INIT done\n", __func__);

      /* Wait for events */
      continue;
    }

    /* Event must be an ETH_EMACPS_EVENT_INTERRUPT */
    PRINTK("%-40s: INTERRUPT\n", __func__);

    /* Process completed BDs */
    eth_emacps_process_txd_bds(emacPs, txRing);

    /* Transmit new fragments */
    for (;;) {
      /* Get next fragment from the network stack */
      m = eth_emacps_next_fragment(ifp, m);

      /* New fragment? */
      if (m != NULL) {
        /* Remember which is the first fragment of the frame */
        if (m0 == NULL) {
          m0 = m;
        }

#ifdef RECORD
        mbufList[mbufIdx   & 0x3ff].alloc = m;
        mbufList[mbufIdx++ & 0x3ff].ctrl  = (m->m_next ? 0 : XEMACPS_TXBUF_LAST_MASK) | (mbufFreeIdx << 16) | m->m_len;
#endif

        /* Next fragment of the frame */
        m = m->m_next;

        /* If the fragment is the last in the frame, hand it to the DMA engine */
        if (m == NULL) {
          sc = eth_emacps_output(emacPs, m0);
          if (sc != RTEMS_SUCCESSFUL) {
            PRINTK("%-40s: Full ring buffer: 0x%08x\n", __func__, m0);

            /* Frame not sent, wait for transmit interrupt */
            break;
          }
          m0 = m;                       /* i.e., NULL */
        }
      } else {
        /* Nothing to transmit */
        break;
      }
    }

    /* No more fragments? */
    if (m == NULL) {
      /* Interface is now inactive */
      ifp->if_flags &= ~IFF_OACTIVE;
    } else {
      /* Enable transmit interrupts */
      eth_emacps_enable_transmit_interrupts(emacPs);
    }
    PRINTK("%-40s: INTERRUPT done\n", __func__);
  }
  PRINTK("%-40s: end\n", __func__);
}

static int eth_emacps_mdio_wait_for_not_busy(XEmacPs* instance)
{
  rtems_interval one_second = rtems_clock_get_ticks_per_second();
  rtems_interval i          = 0;
  unsigned       baseAdx    = instance->Config.BaseAddress;

  PRINTK("%-40s: beg\n", __func__);
  while ((XEmacPs_ReadReg(baseAdx, XEMACPS_NWSR_OFFSET) &
          XEMACPS_NWSR_MDIOIDLE_MASK) == 0 && i < one_second) {
    rtems_task_wake_after(1);
    ++i;
  }

  PRINTK("%-40s: end\n", __func__);
  return i != one_second ? 0 : ETIMEDOUT;
}

static int eth_emacps_mdio_read(int phy, void *arg, unsigned reg, uint32_t *val)
{
  int      eno;
  uint16_t v16;

  PRINTK("%-40s: beg\n", __func__);

  if (phy == -1 && phy < XPAR_XEMACPS_NUM_INSTANCES) {
    EthEmacPs* emacPs;

    if (phy == -1)
      phy = DEFAULT_PHY;

    emacPs = &gEthEmacPs[phy];

    eno = XEmacPs_PhyRead(&emacPs->instance, (unsigned)arg, reg, &v16);
    if (eno == XST_EMAC_MII_BUSY)
      eno = eth_emacps_mdio_wait_for_not_busy(&emacPs->instance);
    *val = v16;
  } else {
    eno = EINVAL;
  }

  PRINTK("%-40s: end\n", __func__);

  return eno;
}

static int eth_emacps_mdio_write(int phy, void *arg, unsigned reg, uint32_t val)
{
  int eno;

  PRINTK("%-40s: beg\n", __func__);

  if (phy == -1 && phy < XPAR_XEMACPS_NUM_INSTANCES) {
    EthEmacPs* emacPs;

    if (phy == -1)
      phy = DEFAULT_PHY;

    emacPs = &gEthEmacPs[phy];

    eno = XEmacPs_PhyWrite(&emacPs->instance, (unsigned)arg, reg, val);
    if (eno == XST_EMAC_MII_BUSY)
      eno = eth_emacps_mdio_wait_for_not_busy(&emacPs->instance);
  } else {
    eno = EINVAL;
  }

  PRINTK("%-40s: end\n", __func__);
  return eno;
}

static int eth_emacps_up_or_down(EthEmacPs *emacPs, bool up)
{
  int               eno = 0;
  rtems_status_code sc;
  struct ifnet     *ifp = &emacPs->arpcom.ac_if;

  PRINTK("%-40s: beg, currently %s, going %s\n", __func__,
         (emacPs->state == ETH_EMACPS_STATE_DOWN ? "DOWN" :
          (emacPs->state == ETH_EMACPS_STATE_UP ? "UP" : "UNDEFINED")),
         (up ? "UP" : "DOWN"));

  if (up && emacPs->state == ETH_EMACPS_STATE_DOWN) {
    /* Initialize the hardware */
    eth_emacps_low_level_init(emacPs);

    /* Initialize tasks */
    eth_emacps_control_request(emacPs, gEthEmacPsRxTask, ETH_EMACPS_EVENT_INITIALIZE);
    eth_emacps_control_request(emacPs, gEthEmacPsTxTask, ETH_EMACPS_EVENT_INITIALIZE);

    /* Install interrupt handler and enable the vector */
    sc = rtems_interrupt_handler_install( emacPs->irqNumber,
                                          "Ethernet",
                                          RTEMS_INTERRUPT_UNIQUE,
                                          XEmacPs_IntrHandler,
                                          emacPs );
    assert(sc == RTEMS_SUCCESSFUL);

    /* Enable transmitter and receiver */
    eth_emacps_start(emacPs);

    /* Start watchdog timer */
    ifp->if_timer = 0; //1; /* @todo - RiC: Punt on the watchdog for now. */

    /* Change state */
    emacPs->state = ETH_EMACPS_STATE_UP;

  } else if (!up && emacPs->state == ETH_EMACPS_STATE_UP) {
    /* Stop the MAC */
    eth_emacps_stop(emacPs);

    /* Remove interrupt handler */
    sc = rtems_interrupt_handler_remove( emacPs->irqNumber,
                                         XEmacPs_IntrHandler,
                                         emacPs );
    assert(sc == RTEMS_SUCCESSFUL);

    /* Stop tasks */
    eth_emacps_control_request(emacPs, gEthEmacPsRxTask, ETH_EMACPS_EVENT_STOP);
    eth_emacps_control_request(emacPs, gEthEmacPsTxTask, ETH_EMACPS_EVENT_STOP);

    /* Stop watchdog timer */
    ifp->if_timer = 0;

    /* Change state */
    emacPs->state = ETH_EMACPS_STATE_DOWN;
  }

  PRINTK("%-40s: end\n", __func__);
  return eno;
}

static void eth_emacps_interface_init(void *arg)
{
  PRINTK("%-40s: beg, arg = %08x, *arg = %08x\n", __func__, (uint)arg, *(uint*)arg);
  /* Nothing to do */
  PRINTK("%-40s: end\n", __func__);
}

static void eth_emacps_interface_stats(EthEmacPs *emacPs)
{
  int                    eno   = EIO;
  int                    media = 0;
  struct ifnet          *ifp   = &emacPs->arpcom.ac_if;
  struct EthEmacPsStats *s     = &emacPs->stats;
  unsigned               base  = emacPs->instance.Config.BaseAddress;

  PRINTK("%-40s: beg\n", __func__);

  if (emacPs->state == ETH_EMACPS_STATE_UP) {
    media = IFM_MAKEWORD(0, 0, 0, 0);
    eno = rtems_mii_ioctl(&emacPs->mdio, emacPs, SIOCGIFMEDIA, &media);
  }

  rtems_bsdnet_semaphore_release();

  if (eno == 0) {
    rtems_ifmedia2str(media, NULL, 0);
    printf("\n");
  }

  printf("\n%s: Statistics for interface '%s%d':\n", __func__, ifp->if_name, ifp->if_unit);

  printf(" SW counters:\n");
  printf("  Frames Transmitted:           %08lx",   s->txd_frames);
  printf("  Frames Received:              %08lx\n", s->rxd_frames);
  printf("  Tx Interrupts:                %08lx",   s->tx_interrupts);
  printf("  Rx Interrupts:                %08lx\n", s->rx_interrupts);

  printf("  Tx fragments:                 %08lx",   s->tx_xmit);
  printf("  Rx fragments:                 %08lx\n", s->rx_recv);
  printf("  Tx drop            errors:    %08lx",   s->tx_drop);
  printf("  Rx drop            errors:    %08lx\n", s->rx_drop);
  printf("  Tx DMA             errors:    %08lx",   s->tx_dma_errors);
  printf("  Rx DMA             errors:    %08lx\n", s->rx_dma_errors);
  printf("  Tx underrun        errors:    %08lx",   s->tx_underruns);
  printf("  Rx overrun         errors:    %08lx\n", s->rx_overruns);
  printf("  Tx no buffer       errors:    %08lx",   s->tx_no_buffers);
  printf("  Rx no buffer       errors:    %08lx\n", s->rx_no_buffers);
  printf("  Tx excessive retry errors:    %08lx",   s->tx_excessive_retries);
  printf("  Rx No new mbuf     errors:    %08lx\n", s->rx_memErr);
  printf("  Tx collisions      errors:    %08lx\n", s->tx_collisions);
  printf("  Tx used bit set    errors:    %08lx\n", s->tx_usedread);
  printf("  Tx overflow        errors:    %08lx\n", s->tx_overflows);
  printf("  Tx full queue      errors:    %08lx\n", s->tx_full_queues);
  printf("  Tx unknown         errors:    %08lx",   s->tx_unknowns);
  printf("  Rx unknown         errors:    %08lx\n", s->rx_unknowns);

  printf("\n Buffer descriptor counters:\n");
  printf("  Tx Retry limit exceeded:      %08lx",   s->tx_stat_retries);
  printf("  Rx Broadcast adx detected:    %08lx\n", s->rx_stat_bcast);
  printf("  Tx Frame corruption:          %08lx",   s->tx_stat_ahb_errors);
  printf("  Rx Multicast hash matches:    %08lx\n", s->rx_stat_mcast);
  printf("  Tx Late collision:            %08lx",   s->tx_stat_late_collisions);
  printf("  Rx Unicast   hash matches:    %08lx\n", s->rx_stat_ucast);
  printf("  Tx IP/TCP/UDP checksum                ");
  printf("  Rx Specific  adx1 matches:    %08lx\n", s->rx_stat_adx1);
  printf("     generation offload errors:         ");
  printf("  Rx Specific  adx2 matches:    %08lx\n", s->rx_stat_adx2);
  printf("     No error:                  %08lx",   s->tx_stat_no_errors);
  printf("  Rx Specific  adx3 matches:    %08lx\n", s->rx_stat_adx3);
  printf("     VLAN packet error:         %08lx",   s->tx_stat_vlan_errors);
  printf("  Rx Specific  adx4 matches:    %08lx\n", s->rx_stat_adx4);
  printf("     SNAP packet error:         %08lx",   s->tx_stat_snap_errors);
  printf("  Rx Bit  24    (see UG585):    %08lx\n", s->rx_stat_bit_24);
  printf("     Pkt not IP type or short:  %08lx",   s->tx_stat_ip_errors);
  printf("  Rx Bits 22,23 = 0 (UG585):    %08lx\n", s->rx_stat_reg1);
  printf("     Pkt not VLAN, SNAP or IP:  %08lx",   s->tx_stat_id_errors);
  printf("  Rx Bits 22,23 = 1 (UG585):    %08lx\n", s->rx_stat_reg2);
  printf("     Unsupported packet frag.:  %08lx",   s->tx_stat_badfrag_errors);
  printf("  Rx Bits 22,23 = 2 (UG585):    %08lx\n", s->rx_stat_reg3);
  printf("     Not TCP or UDP:            %08lx",   s->tx_stat_tcpudp_errors);
  printf("  Rx Bits 22,23 = 3 (UG585):    %08lx\n", s->rx_stat_reg4);
  printf("     Premature end of packet:   %08lx",   s->tx_stat_eop_errors);
  printf("  Rx VLAN     tag detected:     %08lx\n", s->rx_stat_vlan);
  printf("                                        ");
  printf("  Rx Priority tag detected:     %08lx\n", s->rx_stat_priority);
  printf("                                        ");
  printf("  Rx CFI:                       %08lx\n", s->rx_stat_cfi);
  printf("                                        ");
  printf("  Rx End   of Frame:            %08lx\n", s->rx_stat_eof);
  printf("                                        ");
  printf("  Rx Start of Frame:            %08lx\n", s->rx_stat_sof);
  printf("                                        ");
  printf("  Rx Bad FCS (if enabled):      %08lx\n", s->rx_stat_fcs);

  printf("\n HW counters:\n");
  printf("  Octets Txd (w/o err): %08lx%08lx",      XEmacPs_ReadReg(base, XEMACPS_OCTTXH_OFFSET        ), XEmacPs_ReadReg(base, XEMACPS_OCTTXL_OFFSET));
  printf("  Octets Rxd:           %08lx%08lx\n",    XEmacPs_ReadReg(base, XEMACPS_OCTRXH_OFFSET        ), XEmacPs_ReadReg(base, XEMACPS_OCTRXL_OFFSET));
  printf("  Frames Txd:                   %08lx",   XEmacPs_ReadReg(base, XEMACPS_TXCNT_OFFSET         ));
  printf("  Frames Rxd:                   %08lx\n", XEmacPs_ReadReg(base, XEMACPS_RXCNT_OFFSET         ));
  printf("  Broadcast frames Tx:          %08lx",   XEmacPs_ReadReg(base, XEMACPS_TXBCCNT_OFFSET       ));
  printf("  Broadcast frames Rx:          %08lx\n", XEmacPs_ReadReg(base, XEMACPS_RXBROADCNT_OFFSET    ));
  printf("  Multicast frames Tx:          %08lx",   XEmacPs_ReadReg(base, XEMACPS_TXMCCNT_OFFSET       ));
  printf("  Multicast frames Rx:          %08lx\n", XEmacPs_ReadReg(base, XEMACPS_RXMULTICNT_OFFSET    ));
  printf("  Pause     frames Tx:          %08lx",   XEmacPs_ReadReg(base, XEMACPS_TXPAUSECNT_OFFSET    ));
  printf("  Pause     frames Rx:          %08lx\n", XEmacPs_ReadReg(base, XEMACPS_RXPAUSECNT_OFFSET    ));
  printf("  Frames Tx,          64 Bytes: %08lx",   XEmacPs_ReadReg(base, XEMACPS_TX64CNT_OFFSET       ));
  printf("  Frames Rx,          64 Bytes: %08lx\n", XEmacPs_ReadReg(base, XEMACPS_RX64CNT_OFFSET       ));
  printf("  Frames Tx,   65 -  127 Bytes: %08lx",   XEmacPs_ReadReg(base, XEMACPS_TX65CNT_OFFSET       ));
  printf("  Frames Rx,   65 -  127 Bytes: %08lx\n", XEmacPs_ReadReg(base, XEMACPS_RX65CNT_OFFSET       ));
  printf("  Frames Tx,  128 -  255 Bytes: %08lx",   XEmacPs_ReadReg(base, XEMACPS_TX128CNT_OFFSET      ));
  printf("  Frames Rx,  128 -  255 Bytes: %08lx\n", XEmacPs_ReadReg(base, XEMACPS_RX128CNT_OFFSET      ));
  printf("  Frames Tx,  256 -  511 Bytes: %08lx",   XEmacPs_ReadReg(base, XEMACPS_TX256CNT_OFFSET      ));
  printf("  Frames Rx,  256 -  511 Bytes: %08lx\n", XEmacPs_ReadReg(base, XEMACPS_RX256CNT_OFFSET      ));
  printf("  Frames Tx,  512 - 1023 Bytes: %08lx",   XEmacPs_ReadReg(base, XEMACPS_TX512CNT_OFFSET      ));
  printf("  Frames Rx,  512 - 1023 Bytes: %08lx\n", XEmacPs_ReadReg(base, XEMACPS_RX512CNT_OFFSET      ));
  printf("  Frames Tx, 1024 - 1518 Bytes: %08lx",   XEmacPs_ReadReg(base, XEMACPS_TX1024CNT_OFFSET     ));
  printf("  Frames Rx, 1024 - 1518 Bytes: %08lx\n", XEmacPs_ReadReg(base, XEMACPS_RX1024CNT_OFFSET     ));
  printf("  Frames Tx,      > 1518 Bytes: %08lx\n", XEmacPs_ReadReg(base, XEMACPS_TX1519CNT_OFFSET     ));
  printf("  Transmit  underrun    errors: %08lx",   XEmacPs_ReadReg(base, XEMACPS_TXURUNCNT_OFFSET     ));
  printf("  Receive   overrun     errors: %08lx\n", XEmacPs_ReadReg(base, XEMACPS_RXRESERRCNT_OFFSET   ));
  printf("  Single    Collision   Frames: %08lx",   XEmacPs_ReadReg(base, XEMACPS_SNGLCOLLCNT_OFFSET   ));
  printf("  Undersize frames    received: %08lx\n", XEmacPs_ReadReg(base, XEMACPS_RX1519CNT_OFFSET     ));
  printf("  Multiple  Collision   Frames: %08lx",   XEmacPs_ReadReg(base, XEMACPS_MULTICOLLCNT_OFFSET  ));
  printf("  Oversize  frames    received: %08lx\n", XEmacPs_ReadReg(base, XEMACPS_RXUNDRCNT_OFFSET     ));
  printf("  Excessive Collisions:         %08lx",   XEmacPs_ReadReg(base, XEMACPS_EXCESSCOLLCNT_OFFSET ));
  printf("  Jabbers             received: %08lx\n", XEmacPs_ReadReg(base, XEMACPS_RXOVRCNT_OFFSET      ));
  printf("  Late      Collisions:         %08lx",   XEmacPs_ReadReg(base, XEMACPS_LATECOLLCNT_OFFSET   ));
  printf("  Frame check sequence  errors: %08lx\n", XEmacPs_ReadReg(base, XEMACPS_RXJABCNT_OFFSET      ));
  printf("  Deferred Transmission Frames: %08lx",   XEmacPs_ReadReg(base, XEMACPS_TXDEFERCNT_OFFSET    ));
  printf("  Length    field frame errors: %08lx\n", XEmacPs_ReadReg(base, XEMACPS_RXFCSCNT_OFFSET      ));
  printf("  Carrier   Sense       errors: %08lx",   XEmacPs_ReadReg(base, XEMACPS_TXCSENSECNT_OFFSET   ));
  printf("  Receive   symbol      errors: %08lx\n", XEmacPs_ReadReg(base, XEMACPS_RXLENGTHCNT_OFFSET   ));
  printf("  IP header checksum    errors: %08lx",   XEmacPs_ReadReg(base, XEMACPS_RXORCNT_OFFSET       ));
  printf("  Receive   resource    errors: %08lx\n", XEmacPs_ReadReg(base, XEMACPS_RXALIGNCNT_OFFSET    ));
  printf("  TCP       checksum    errors: %08lx",   XEmacPs_ReadReg(base, XEMACPS_RXIPCCNT_OFFSET      ));
  printf("  Alignment             errors: %08lx\n", XEmacPs_ReadReg(base, XEMACPS_RXSYMBCNT_OFFSET     ));
  printf("  UDP       checksum    errors: %08lx\n", XEmacPs_ReadReg(base, XEMACPS_RXTCPCCNT_OFFSET     ));

  printf("  PTP      event frame Txd S:   %08lx",   XEmacPs_ReadReg(base, XEMACPS_PTP_TXSEC_OFFSET     ));
  printf("  PTP      event frame Rxd S:   %08lx\n", XEmacPs_ReadReg(base, XEMACPS_PTP_RXSEC_OFFSET     ));
  printf("  PTP      event frame Txd nS:  %08lx",   XEmacPs_ReadReg(base, XEMACPS_PTP_TXNANOSEC_OFFSET ));
  printf("  PTP      event frame Rxd nS:  %08lx\n", XEmacPs_ReadReg(base, XEMACPS_PTP_RXNANOSEC_OFFSET ));
  printf("  PTP peer event frame Txd S:   %08lx",   XEmacPs_ReadReg(base, XEMACPS_PTPP_TXSEC_OFFSET    ));
  printf("  PTP peer event frame Rxd S:   %08lx\n", XEmacPs_ReadReg(base, XEMACPS_PTPP_RXSEC_OFFSET    ));
  printf("  PTP peer event frame Txd nS:  %08lx",   XEmacPs_ReadReg(base, XEMACPS_PTPP_TXNANOSEC_OFFSET));
  printf("  PTP peer event frame Rxd nS:  %08lx\n", XEmacPs_ReadReg(base, XEMACPS_PTPP_RXNANOSEC_OFFSET));
  printf("  1588 timer seconds (S):       %08lx",   XEmacPs_ReadReg(base, XEMACPS_1588_SEC_OFFSET      ));
  printf("  1588 timer sync strobe S:     %08lx\n", XEmacPs_ReadReg(base, XEMACPS_RXUDPCCNT_OFFSET     ));
  printf("  1588 timer nanoseconds (nS):  %08lx",   XEmacPs_ReadReg(base, XEMACPS_1588_NANOSEC_OFFSET  ));
  printf("  1588 timer sync strobe nS:    %08lx\n", XEmacPs_ReadReg(base, XEMACPS_LAST_OFFSET          ));
  printf("  1588 timer adjust:            %08lx",   XEmacPs_ReadReg(base, XEMACPS_1588_ADJ_OFFSET      ));
  printf("  1588 timer increment:         %08lx\n", XEmacPs_ReadReg(base, XEMACPS_1588_INC_OFFSET      ));
  printf("\n");

  rtems_bsdnet_semaphore_obtain();
  PRINTK("%-40s: end\n", __func__);
}

// @todo - RiC: Still need to sort this out
static int eth_emacps_multicast_control( bool           add,
                                         struct ifreq  *ifr,
                                         struct arpcom *ac )
{
  int eno = 0;

  PRINTK("%-40s: beg\n", __func__);
  if (add) {
    eno = ether_addmulti(ifr, ac);
  } else {
    eno = ether_delmulti(ifr, ac);
  }

  if (eno == ENETRESET) {
    struct ether_multistep step;
    struct ether_multi *enm;

    eno = 0;

    //eth->hashfilterl = 0;
    //eth->hashfilterh = 0;

    ETHER_FIRST_MULTI(step, ac, enm);
    while (enm != NULL) {
      uint64_t addrlo = 0;
      uint64_t addrhi = 0;

      memcpy(&addrlo, enm->enm_addrlo, ETHER_ADDR_LEN);
      memcpy(&addrhi, enm->enm_addrhi, ETHER_ADDR_LEN);
      while (addrlo <= addrhi) {
        /* XXX: ether_crc32_le() does not work, why? */
        uint32_t crc = ether_crc32_be((uint8_t *) &addrlo, ETHER_ADDR_LEN);
        uint32_t idx = (crc >> 23) & 0x3f;

        if (idx < 32) {
        //  eth->hashfilterl |= 1U << idx;
        } else {
        //  eth->hashfilterh |= 1U << (idx - 32);
        }
        ++addrlo;
      }
      ETHER_NEXT_MULTI(step, enm);
    }
  }

  PRINTK("%-40s: end\n", __func__);
  return eno;
}

static int eth_emacps_interface_ioctl( struct ifnet   *ifp,
                                       ioctl_command_t cmd,
                                       caddr_t         data )
{
  EthEmacPs    *emacPs = (EthEmacPs *) ifp->if_softc;
  struct ifreq *ifr    = (struct ifreq *) data;
  int           eno    = 0;

  PRINTK("%-40s: beg, cmd = %08x\n", __func__, cmd);

  switch (cmd)  {
    case SIOCGIFMEDIA:
    case SIOCSIFMEDIA:
      PRINTK("%-40s: MEDIA beg, cmd = %08x, data = %08x\n", __func__, cmd, data);
      rtems_mii_ioctl(&emacPs->mdio, emacPs, cmd, &ifr->ifr_media);
      PRINTK("%-40s: MEDIA end\n", __func__);
      break;
    case SIOCGIFADDR:
    case SIOCSIFADDR:
      PRINTK("%-40s: ADDR beg, cmd = %08x, data = %08x\n", __func__, cmd, data);
      ether_ioctl(ifp, cmd, data);
      PRINTK("%-40s: ADDR end\n", __func__);
      break;
    case SIOCSIFFLAGS:
      PRINTK("%-40s: FLAGS beg\n", __func__);
      eno = eth_emacps_up_or_down(emacPs, (ifp->if_flags & IFF_UP) != 0);
      if (eno == 0 && (ifp->if_flags & IFF_UP) != 0) {
        PRINTK("%-40s: FLAGS mid\n", __func__);
        eth_emacps_enable_promiscous_mode(emacPs, (ifp->if_flags & IFF_PROMISC) != 0);
      }
      PRINTK("%-40s: FLAGS end\n", __func__);
      break;
    case SIOCADDMULTI:
    case SIOCDELMULTI:
      PRINTK("%-40s: MULTI beg\n", __func__);
      eno = eth_emacps_multicast_control(cmd == SIOCADDMULTI, ifr, &emacPs->arpcom);
      PRINTK("%-40s: MULTI end\n", __func__);
      break;
    case SIO_RTEMS_SHOW_STATS:
      PRINTK("%-40s: STATS beg\n", __func__);
      eth_emacps_interface_stats(emacPs);
      PRINTK("%-40s: STATS end\n", __func__);
      break;
    default:
      PRINTK("%-40s: Error\n", __func__);
      eno = EINVAL;
      break;
  }

  PRINTK("%-40s: end\n", __func__);
  return eno;
}

static void eth_emacps_interface_start(struct ifnet *ifp)
{
  PRINTK("%-40s: beg\n", __func__);

  EthEmacPs *emacPs = (EthEmacPs *)ifp->if_softc;

  ifp->if_flags |= IFF_OACTIVE;

  /* Wake up tx thread with outbound interface's signal */
  if (emacPs->state == ETH_EMACPS_STATE_UP) {
    rtems_status_code sc = RTEMS_SUCCESSFUL;
    sc = rtems_bsdnet_event_send(gEthEmacPsTxTask, ETH_EMACPS_EVENT_TXSTART);
    assert(sc == RTEMS_SUCCESSFUL);
  }
  PRINTK("%-40s: end\n", __func__);
}

// @todo - RiC: Punt on this for now
//static void eth_emacps_interface_watchdog(struct ifnet *ifp)
//{
//  EthEmacPs *emacPs = (EthEmacPs *) ifp->if_softc;
//
//  if (emacPs->state == ETH_EMAC_STATE_UP) {
//    eth_emacps_phy_setup(emacPs);  <-- I think this isn't right.
//
//    ifp->if_timer = WATCHDOG_TIMEOUT;
//  }
//}

static int eth_emacps_attach(struct rtems_bsdnet_ifconfig *ifCfg)
{
  EthEmacPs    *emacPs;
  struct ifnet *ifp;
  char         *unitName;
  int           unitIdx;
  size_t        tableAreaSize = 0;
  char         *tableArea     = NULL;
  char         *tableLocation = NULL;

  PRINTK("%-40s: beg\n", __func__);

  /* Check parameters */
  unitIdx = rtems_bsdnet_parse_driver_name(ifCfg, &unitName);
  if (unitIdx < 0) {
    RTEMS_SYSLOG("Interface not found (%d)\n", unitIdx);
    return 0;
  }

  if (unitIdx >= XPAR_XEMACPS_NUM_INSTANCES) {
    RTEMS_SYSLOG("Bad unit number (%d)\n", unitIdx);
    goto cleanup;
  }

  if (ifCfg->hardware_address == NULL) {
    RTEMS_SYSLOG("No MAC address given for interface '%s%d'\n",
                 unitName, unitIdx);
    goto cleanup;
  }

  emacPs = &gEthEmacPs[unitIdx];
  if (emacPs->state != ETH_EMACPS_STATE_NOT_INITIALIZED) {
    RTEMS_SYSLOG("Device '%s%d' is already attached\n", unitName, unitIdx);
    goto cleanup;
  }

  /* MDIO */                 /*@todo - RiC: Still need to sort this out */
  emacPs->mdio.mdio_r   = eth_emacps_mdio_read;
  emacPs->mdio.mdio_w   = eth_emacps_mdio_write;
  emacPs->mdio.has_gmii = 1;

  /* Interrupt number */
  ifCfg->irno       = ETH_EMACPS_CONFIG_INTERRUPT;
  emacPs->irqNumber = ifCfg->irno;

  /* Device control */
  ifCfg->drv_ctrl = emacPs;

  /* Receive unit count */
  emacPs->rxUnitCount = RXBD_CNT;
  ifCfg->rbuf_count   = (int) emacPs->rxUnitCount;

  /* Transmit unit count */
  emacPs->txUnitCount = TXBD_CNT;
  ifCfg->xbuf_count   = (int) emacPs->txUnitCount;

  /* Copy MAC address */
  memcpy(emacPs->arpcom.ac_enaddr, ifCfg->hardware_address, ETHER_ADDR_LEN);

  PRINTK("%-40s: MAC address = %02x:%02x:%02x:%02x:%02x:%02x\n", __func__,
         ((char*)emacPs->arpcom.ac_enaddr)[0],
         ((char*)emacPs->arpcom.ac_enaddr)[1],
         ((char*)emacPs->arpcom.ac_enaddr)[2],
         ((char*)emacPs->arpcom.ac_enaddr)[3],
         ((char*)emacPs->arpcom.ac_enaddr)[4],
         ((char*)emacPs->arpcom.ac_enaddr)[5]);

  /* Allocate and clear table area */
  tableAreaSize = emacPs->rxUnitCount * (sizeof(struct mbuf *))
                + emacPs->txUnitCount * (sizeof(struct mbuf *) +
                                         ETH_EMACPS_CONFIG_TX_BUF_SIZE);
  tableArea     = eth_emacps_config_alloc_table_area(tableAreaSize);
  if (tableArea == NULL) {
    RTEMS_SYSLOG("Unable to allocate table area");
    goto cleanup;
  }
  memset(tableArea, 0, tableAreaSize);

  tableLocation = tableArea;

  /*
   * The receive status table must be the first one since it has the strictest
   * alignment requirements.
   */
  emacPs->rxMbufTable = (struct mbuf**)tableLocation;
  tableLocation      += emacPs->rxUnitCount * sizeof(emacPs->rxMbufTable[0]);

  emacPs->txMbufTable = (struct mbuf**)tableLocation;

  PRINTK("%-40s: rxMbufTable = %08x\n", __func__, emacPs->rxMbufTable);
  PRINTK("%-40s: txMbufTable = %08x\n", __func__, emacPs->txMbufTable);

  /* Set interface data */
  ifp = &emacPs->arpcom.ac_if;

  ifp->if_softc          = emacPs;
  ifp->if_unit           = (short)unitIdx;
  ifp->if_name           = unitName;
  ifp->if_mtu            = (ifCfg->mtu > 0) ? (u_long) ifCfg->mtu : ETHERMTU;
  ifp->if_init           = eth_emacps_interface_init;
  ifp->if_ioctl          = eth_emacps_interface_ioctl;
  ifp->if_start          = eth_emacps_interface_start;
  ifp->if_output         = ether_output;
  //ifp->if_watchdog       = eth_emacps_interface_watchdog; /* RiC: Punt. */
  ifp->if_flags          = IFF_MULTICAST | IFF_BROADCAST | IFF_SIMPLEX;
  ifp->if_snd.ifq_maxlen = ifqmaxlen;
  ifp->if_timer          = 0;           /* Leave the watchdog stopped */

  /* Create shared rx & tx tasks */
  if ( (gEthEmacPsRxTask == 0) && (gEthEmacPsTxTask == 0) )
  {
    gEthEmacPsRxTask = rtems_bsdnet_newproc("ntrx", 4096, eth_emacps_receive_task,  emacPs);
    gEthEmacPsTxTask = rtems_bsdnet_newproc("nttx", 4096, eth_emacps_transmit_task, emacPs);
    gEthEmacPsErTask = rtems_bsdnet_newproc("nter", 4096, eth_emacps_error_task,    emacPs);
  }

  /* Change status */
  ifp->if_flags |= IFF_RUNNING;
  emacPs->state  = ETH_EMACPS_STATE_DOWN;

  /* Attach the interface */
  if_attach(ifp);
  ether_ifattach(ifp);

  PRINTK("%-40s: end\n", __func__);

  return 1;

cleanup:

  PRINTK ("%-40s: cleanup\n", __func__);

  eth_emacps_config_free_table_area(tableArea);

  /* FIXME: Type */
  free(unitName, (int) 0xdeadbeef);

  PRINTK("%-40s: end\n", __func__);

  return 0;
}

static int eth_emacps_detach(struct rtems_bsdnet_ifconfig *ifCfg)
{
  EthEmacPs* emacPs = (EthEmacPs*)ifCfg->drv_ctrl;

  /* FIXME: Detach the interface from the upper layers? */

  /* Module soft reset */
  XEmacPs_Reset(&emacPs->instance);

  return 0;
}

int zynq_eth_attach_detach(struct rtems_bsdnet_ifconfig *ifCfg,
                           int                           attaching)
{
  /* FIXME: Return value */

  if (attaching) {
    return eth_emacps_attach(ifCfg);
  } else {
    return eth_emacps_detach(ifCfg);
  }
}

/*
 * lnk_prelude():
 *
 * This function should be called when the ethEmacPs library is 
 * dynamically loaded.
 *
 * This routine looks up the BsdNet attributes from the system
 * svt, and executes the network interface configuration.
 *
 * Returns the error status
 *
 */

int lnk_options __attribute__((visibility("default"))) =  LNK_INSTALL;

static const char BSDNET_ATTRS[] = "BSDNET_ATTRS";
static const char _attrs_error[] = "Xaui BsdNet attributes lookup failed for symbol %s\n";

int lnk_prelude(BsdNet_Attributes *prefs, void *elfHdr)
  {
  int error = SVT_SUCCESS;
  if(!prefs)
    {
    prefs = (BsdNet_Attributes*)Svt_Translate(BSDNET_ATTRS, SVT_SYS_TABLE);
    error = prefs ? SVT_SUCCESS: SVT_NOSUCH_SYMBOL;
    if(error != SVT_SUCCESS) 
      {
      printf(_attrs_error,BSDNET_ATTRS);    
      return error;
      }
    }
  
  // configure the interface
  BsdNet_Configure(prefs,zynq_eth_attach_detach);
  
  return error;
  }
