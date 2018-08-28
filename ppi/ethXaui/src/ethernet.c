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

#if MCLBYTES > (2 * 1024)
  #error "MCLBYTES too large"
#endif

#define ETH_XAUI_CONFIG_TX_BUF_SIZE sizeof(struct mbuf *)

#define DEFAULT_PHY      0
#define WATCHDOG_TIMEOUT 5

/* Status */
#define ETH_XAUI_LAST_FRAGMENT_FLAG 0x8000000u

static EthXaui gEthXaui[1];

rtems_id gEthXauiTxTask = 0;

static void eth_xaui_control_request_complete(const EthXaui *xaui)
{
  PRINTK("%-40s: beg\n", __func__);
  rtems_status_code sc = rtems_event_transient_send(xaui->controlTask);
  assert(sc == RTEMS_SUCCESSFUL);
  PRINTK("%-40s: end\n", __func__);
}

static void eth_xaui_control_request(EthXaui      *xaui,
                                       rtems_id        task,
                                       rtems_event_set event)
{
  PRINTK("%-40s: beg\n", __func__);
  rtems_status_code sc = RTEMS_SUCCESSFUL;
  uint32_t nest_count = 0;

  xaui->controlTask = rtems_task_self();

  sc = rtems_bsdnet_event_send(task, event);
  assert(sc == RTEMS_SUCCESSFUL);

  nest_count = rtems_bsdnet_semaphore_release_recursive();
  sc = rtems_event_transient_receive(RTEMS_WAIT, RTEMS_NO_TIMEOUT);
  assert(sc == RTEMS_SUCCESSFUL);
  rtems_bsdnet_semaphore_obtain_recursive(nest_count);

  xaui->controlTask = 0;
  PRINTK("%-40s: end\n", __func__);
}

struct mbuf *eth_xaui_next_fragment(struct ifnet *ifp,
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
  while (n != NULL && (n->m_len <= 0)) {
    n = m_free(n);
  }
  m->m_next = n;

  PRINTK("%-40s: end, m = %08x, d = %08x, len = %08x, n = %08x\n", __func__,
         (uint)m, mtod(m, uint), m->m_len, (uint)m->m_next);

  return m;
}

static void eth_xaui_transmit_task(void *arg)
{
  rtems_status_code  sc       = RTEMS_SUCCESSFUL;
  rtems_event_set    events   = 0;
  EthXaui           *xaui     = (EthXaui *) arg;
  struct ifnet      *ifp      = &xaui->arpcom.ac_if;
  struct mbuf       *m        = NULL;
  struct mbuf       *m0       = NULL;

  PRINTK("%-40s: beg\n", __func__);

  /* Main event loop */
  for (;;) {

    PRINTK("%-40s: Waiting...\n", __func__);
    /* Wait for events */
    sc = rtems_bsdnet_event_receive( ETH_XAUI_EVENT_INITIALIZE |
                                     ETH_XAUI_EVENT_STOP       |
                                     ETH_XAUI_EVENT_TXSTART    |
                                     ETH_XAUI_EVENT_INTERRUPT,
                                     RTEMS_EVENT_ANY | RTEMS_WAIT,
                                     RTEMS_NO_TIMEOUT,
                                     &events );
    assert(sc == RTEMS_SUCCESSFUL);

    PRINTK("%-40s: WAKE up: 0x%08" PRIx32 "\n", __func__, events);

    /* Stop transmitter? */
    if ((events & ETH_XAUI_EVENT_STOP) != 0) {
      PRINTK("%-40s: STOP\n", __func__);
      
      eth_xaui_control_request_complete(xaui);

      PRINTK("%-40s: STOP done\n", __func__);

      /* Wait for events */
      continue;
    }

    /* Initialize transmitter? */
    if ((events & ETH_XAUI_EVENT_INITIALIZE) != 0) {
      PRINTK("%-40s: INIT\n", __func__);

      /* Discard outstanding fragments (= data loss) */
      eth_xaui_process_tx(xaui);

      eth_xaui_control_request_complete(xaui);

      PRINTK("%-40s: INIT done\n", __func__);

      /* Wait for events */
      continue;
    }
    
    /* Event must be an ETH_XAUI_EVENT_INTERRUPT */
    PRINTK("%-40s: INTERRUPT\n", __func__);
    
    ++xaui->stats.tx_xmit;

    /* Transmit new fragments */
    for (;;) {
      /* Get next fragment from the network stack */      
      m = eth_xaui_next_fragment(ifp, m);

      /* New fragment? */
      if (m != NULL) {
        /* Remember which is the first fragment of the frame */
        if (m0 == NULL) {
          m0 = m;
        }
                
        /* Next fragment of the frame */
        m = m->m_next;

        /* If the fragment is the last in the frame, hand it to the DMA engine */
        if (m == NULL) {
          sc = eth_xaui_output(xaui, m0);
          if (sc != RTEMS_SUCCESSFUL) {
            PRINTK("%-40s: Full buffer: 0x%08x\n", __func__, m0);

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
    }
    PRINTK("%-40s: INTERRUPT done\n", __func__);    
    
  }
  PRINTK("%-40s: end\n", __func__);
}

static int eth_xaui_mdio_read(int phy, void *arg, unsigned reg, uint32_t *val)
{
  int      eno;

  PRINTK("%-40s: beg\n", __func__);

  PRINTK("%-40s: end\n", __func__);

  return eno;
}

static int eth_xaui_mdio_write(int phy, void *arg, unsigned reg, uint32_t val)
{
  int eno;

  PRINTK("%-40s: beg\n", __func__);

  PRINTK("%-40s: end\n", __func__);
  return eno;
}

static int eth_xaui_up_or_down(EthXaui *xaui, bool up)
{
  int               eno = 0;
  rtems_status_code sc;
  struct ifnet     *ifp = &xaui->arpcom.ac_if;

  PRINTK("%-40s: beg, currently %s, going %s\n", __func__,
         (xaui->state == ETH_XAUI_STATE_DOWN ? "DOWN" :
          (xaui->state == ETH_XAUI_STATE_UP ? "UP" : "UNDEFINED")),
         (up ? "UP" : "DOWN"));

  if (up && xaui->state == ETH_XAUI_STATE_DOWN) {
    /* Initialize the hardware */
    eth_xaui_low_level_init(xaui);

    /* Initialize tasks */
    eth_xaui_control_request(xaui, gEthXauiTxTask, ETH_XAUI_EVENT_INITIALIZE);

    /* Enable transmitter and receiver */
    eth_xaui_start(xaui);

    /* Start watchdog timer */
    ifp->if_timer = 0; //1; /* @todo - RiC: Punt on the watchdog for now. */

    /* Change state */
    xaui->state = ETH_XAUI_STATE_UP;

  } else if (!up && xaui->state == ETH_XAUI_STATE_UP) {
    /* Stop the MAC */
    eth_xaui_stop(xaui);

    /* Stop tasks */
    eth_xaui_control_request(xaui, gEthXauiTxTask, ETH_XAUI_EVENT_STOP);

    /* Stop watchdog timer */
    ifp->if_timer = 0;

    /* Change state */
    xaui->state = ETH_XAUI_STATE_DOWN;
  }

  PRINTK("%-40s: end\n", __func__);
  return eno;
}

static void eth_xaui_interface_init(void *arg)
{
  PRINTK("%-40s: beg, arg = %08x, *arg = %08x\n", __func__, (uint)arg, *(uint*)arg);
  /* Nothing to do */
  PRINTK("%-40s: end\n", __func__);
}

static void eth_xaui_interface_stats(EthXaui *xaui)
{
  int                    eno   = EIO;
  int                    media = 0;
  struct ifnet          *ifp   = &xaui->arpcom.ac_if;
  struct EthXauiStats *s     = &xaui->stats;

  PRINTK("%-40s: beg\n", __func__);

  if (xaui->state == ETH_XAUI_STATE_UP) {
    media = IFM_MAKEWORD(0, 0, 0, 0);
    eno = rtems_mii_ioctl(&xaui->mdio, xaui, SIOCGIFMEDIA, &media);
  }

  rtems_bsdnet_semaphore_release();

  if (eno == 0) {
    rtems_ifmedia2str(media, NULL, 0);
    printf("\n");
  }

  printf("\nSW counters for interface '%s%d':\n", ifp->if_name, ifp->if_unit);

  printf(" Frames Transmitted:           %08lx",   s->txd_frames);
  printf(" Frames Received:              %08lx\n", s->rxd_frames);
  printf(" Tx Interrupts:                %08lx",   s->tx_interrupts);
  printf(" Rx Interrupts:                %08lx\n", s->rx_interrupts);

  printf(" Tx fragments:                 %08lx",   s->tx_xmit);
  printf(" Rx fragments:                 %08lx\n", s->rx_recv);
  printf(" Tx drop            errors:    %08lx",   s->tx_drop);
  printf(" Rx drop            errors:    %08lx\n", s->rx_drop);
  printf(" Tx no buffer       errors:    %08lx",   s->tx_no_buffers);
  printf(" Rx No new mbuf     errors:    %08lx\n", s->rx_memErr);
  printf(" Tx full queue      errors:    %08lx\n", s->tx_full_queues);
  printf("\n");
  
  Xaui_Stats(0);
  
  printf("\n Xaui HW status:\n");
  
  Xaui_Dump();

  rtems_bsdnet_semaphore_obtain();
  PRINTK("%-40s: end\n", __func__);
}

// @todo - RiC: Still need to sort this out
static int eth_xaui_multicast_control( bool           add,
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

static int eth_xaui_interface_ioctl( struct ifnet   *ifp,
                                     ioctl_command_t cmd,
                                     caddr_t         data )
{
  EthXaui    *xaui   = (EthXaui *) ifp->if_softc;
  struct ifreq *ifr  = (struct ifreq *) data;
  int           eno  = 0;

  PRINTK("%-40s: beg, cmd = %08x\n", __func__, cmd);

  switch (cmd)  {
    case SIOCGIFMEDIA:
    case SIOCSIFMEDIA:
      PRINTK("%-40s: MEDIA beg, cmd = %08x, data = %08x\n", __func__, cmd, data);
      rtems_mii_ioctl(&xaui->mdio, xaui, cmd, &ifr->ifr_media);
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
      eno = eth_xaui_up_or_down(xaui, (ifp->if_flags & IFF_UP) != 0);
      PRINTK("%-40s: FLAGS end\n", __func__);
      break;
    case SIOCADDMULTI:
    case SIOCDELMULTI:
      PRINTK("%-40s: MULTI beg\n", __func__);
      eno = eth_xaui_multicast_control(cmd == SIOCADDMULTI, ifr, &xaui->arpcom);
      PRINTK("%-40s: MULTI end\n", __func__);
      break;
    case SIO_RTEMS_SHOW_STATS:
      PRINTK("%-40s: STATS beg\n", __func__);
      eth_xaui_interface_stats(xaui);
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

static void eth_xaui_interface_start(struct ifnet *ifp)
{
  PRINTK("%-40s: beg\n", __func__);

  EthXaui *xaui = (EthXaui *)ifp->if_softc;

  ifp->if_flags |= IFF_OACTIVE;

  /* Wake up tx thread with outbound interface's signal */

  if (xaui->state == ETH_XAUI_STATE_UP) {      
    rtems_status_code sc = RTEMS_SUCCESSFUL;
    sc = rtems_bsdnet_event_send(gEthXauiTxTask, ETH_XAUI_EVENT_TXSTART);
    assert(sc == RTEMS_SUCCESSFUL);
  }
  PRINTK("%-40s: end\n", __func__);
}

static int eth_xaui_attach(struct rtems_bsdnet_ifconfig *ifCfg)
{
  EthXaui      *xaui;
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

  if (unitIdx >= 1) {
    RTEMS_SYSLOG("Bad unit number (%d)\n", unitIdx);
    goto cleanup;
  }

  if (ifCfg->hardware_address == NULL) {
    RTEMS_SYSLOG("No MAC address given for interface '%s%d'\n",
                 unitName, unitIdx);
    goto cleanup;
  }

  xaui = &gEthXaui[unitIdx];
  if (xaui->state != ETH_XAUI_STATE_NOT_INITIALIZED) {
    RTEMS_SYSLOG("Device '%s%d' is already attached\n", unitName, unitIdx);
    goto cleanup;
  }

  /* MDIO */                 /*@todo - RiC: Still need to sort this out */
  xaui->mdio.mdio_r   = eth_xaui_mdio_read;
  xaui->mdio.mdio_w   = eth_xaui_mdio_write;
  xaui->mdio.has_gmii = 1;

  /* Device control */
  ifCfg->drv_ctrl = xaui;

  /* Transmit unit count */
  xaui->txUnitCount = TXBD_CNT;
  
  /* initialize counters */
  xaui->txIdx  = 0;
  xaui->txLast = 0;
  xaui->txfree = xaui->txUnitCount;

  /* Copy MAC address */
  memcpy(xaui->arpcom.ac_enaddr, ifCfg->hardware_address, ETHER_ADDR_LEN);

  PRINTK("%-40s: MAC address = %02x:%02x:%02x:%02x:%02x:%02x\n", __func__,
         ((char*)xaui->arpcom.ac_enaddr)[0],
         ((char*)xaui->arpcom.ac_enaddr)[1],
         ((char*)xaui->arpcom.ac_enaddr)[2],
         ((char*)xaui->arpcom.ac_enaddr)[3],
         ((char*)xaui->arpcom.ac_enaddr)[4],
         ((char*)xaui->arpcom.ac_enaddr)[5]);

  /* Allocate and clear table area */
  tableAreaSize = xaui->txUnitCount * (sizeof(struct mbuf *) +
                                       ETH_XAUI_CONFIG_TX_BUF_SIZE);
  tableArea     = eth_xaui_config_alloc_table_area(tableAreaSize);
  if (tableArea == NULL) {
    RTEMS_SYSLOG("Unable to allocate table area");
    goto cleanup;
  }
  memset(tableArea, 0, tableAreaSize);

  tableLocation = tableArea;

  xaui->txMbufTable = (struct mbuf**)tableLocation;

  PRINTK("%-40s: txMbufTable = %08x\n", __func__, xaui->txMbufTable);

  /* Set interface data */
  ifp = &xaui->arpcom.ac_if;

  ifp->if_softc          = xaui;
  ifp->if_unit           = (short)unitIdx;
  ifp->if_name           = unitName;
  ifp->if_mtu            = (ifCfg->mtu > 0) ? (u_long) ifCfg->mtu : ETHERMTU;
  ifp->if_init           = eth_xaui_interface_init;
  ifp->if_ioctl          = eth_xaui_interface_ioctl;
  ifp->if_start          = eth_xaui_interface_start;
  ifp->if_output         = ether_output;
  ifp->if_watchdog       = 0; /* SEM: punt for now */
  ifp->if_flags          = IFF_MULTICAST | IFF_BROADCAST | IFF_SIMPLEX;
  ifp->if_snd.ifq_maxlen = ifqmaxlen;
  ifp->if_timer          = 0;           /* Leave the watchdog stopped */

  /* Create shared tx task */
  if (gEthXauiTxTask == 0) 
  {
    gEthXauiTxTask = rtems_bsdnet_newproc("nttx", 4096, eth_xaui_transmit_task, xaui);
  }

  /* Change status */
  ifp->if_flags |= IFF_RUNNING;
  xaui->state  = ETH_XAUI_STATE_DOWN;

  /* Attach the interface */
  if_attach(ifp);
  ether_ifattach(ifp);

  PRINTK("%-40s: end\n", __func__);

  return 1;

cleanup:

  PRINTK ("%-40s: cleanup\n", __func__);

  eth_xaui_config_free_table_area(tableArea);

  /* FIXME: Type */
  free(unitName, (int) 0xdeadbeef);

  PRINTK("%-40s: end\n", __func__);

  return 0;
}

static int eth_xaui_detach(struct rtems_bsdnet_ifconfig *ifCfg)
{
  EthXaui* xaui = (EthXaui*)ifCfg->drv_ctrl;

  /* FIXME: Detach the interface from the upper layers? */

  return 0;
}

int xaui_eth_attach_detach(struct rtems_bsdnet_ifconfig *ifCfg,
                           int                           attaching)
{
  /* FIXME: Return value */

  if (attaching) {
    return eth_xaui_attach(ifCfg);
  } else {
    return eth_xaui_detach(ifCfg);
  }
}

/*
 * lnk_prelude():
 *
 * This function should be called when the ethXaui library is 
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
static const char _attrs_error[] = "ethXaui BsdNet attributes lookup failed for symbol %s\n";

int lnk_prelude(BsdNet_Attributes *prefs, void *elfHdr)
  {
  int error = SVT_SUCCESS;
    prefs = (BsdNet_Attributes*)Svt_Translate(BSDNET_ATTRS, SVT_SYS_TABLE);
    error = prefs ? SVT_SUCCESS: SVT_NOSUCH_SYMBOL;
    if(error != SVT_SUCCESS) 
      {
      printf(_attrs_error,BSDNET_ATTRS);    
      return error;
      }
      
  // configure the interface
  printf("ethXaui: configuring interface\n");  
  BsdNet_Configure(prefs,xaui_eth_attach_detach);
  
  return error;
  }

/*!
 * eth_xaui_process_rx():
 *
 * @brief Callback handler for inbound frames.
 *
 */

void eth_xaui_free_rx(void* buffer, unsigned size)
  {
  EthXaui* xaui = &gEthXaui[0];
  Xaui_Free(xaui->rxDevice,buffer,size);
  }
