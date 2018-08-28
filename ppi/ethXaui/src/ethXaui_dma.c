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

#include <stdio.h>

#include <net/ethernet.h>

#include "xaui/Xaui.h"
#include "sas/Sas.h"

#include "ethernet.h"

void eth_xaui_process_tx(EthXaui *xaui)
{
  unsigned      k;
  struct mbuf **mbufs = xaui->txMbufTable;
  struct mbuf  *m     = mbufs[xaui->txLast];
  struct mbuf  *n;
  int           n_mbufs;
  
  /* First count the number of mbufs */
  for (n = m, n_mbufs = 0; n != NULL; n = n->m_next)
    n_mbufs++;

  PRINTK("%-40s: beg, mbufs  = %08x, txLast = %08x, txIdx = %08x\n", __func__, n_mbufs,xaui->txLast,xaui->txIdx);
  
  /* Free the processed BD's */
  for (k = 0; k < n_mbufs; k++) {
    unsigned     status = 0;
    m = mbufs[xaui->txLast];

    /* Release mbuf */
    if (m != NULL)
      {
      PRINTK("%s: bdIdx = %08x, mbufs = %08x, mbuf = %08x, m_data = %08x flags 0x%x\n", __func__,
           xaui->txLast, (uint)mbufs, (uint)m, (uint)m->m_data,m->m_flags);    
      m_free(m);
      mbufs[xaui->txLast] = NULL;    
     }
     
   if(++xaui->txLast >= TXBD_CNT)
     {    
     PRINTK("%s: wrap tx at %d\n",__func__,xaui->txLast);
     xaui->txLast = 0;
     }
   }
  
  xaui->txfree  += n_mbufs;
    
  PRINTK("%-40s: end, txLast = %08x, txFree = %08x, txIdx = %08x\n", __func__, xaui->txLast, xaui->txfree,xaui->txIdx);
}

/*!
 * sgsend():
 *
 * @brief Scatter/gather send of a frame, potentially composed of multiple
 *        fragments.
 *
 * This function should be called only with interrupts disabled.
 */
rtems_status_code eth_xaui_sgsend(EthXaui *xaui, struct mbuf *m)
{
  int             xs;
  uint32_t        ctrl;
  struct mbuf   **mbufs  = xaui->txMbufTable;
  struct mbuf    *n;
  int             cnt = 0;
  int             size = 0;
  int             i;
  Xaui_Header    *frame;
  uint8_t        *payload;
  uint8_t        *src;
  uint8_t        *dst;

  PRINTK("%-40s: beg\n", __func__);

  PRINTK("%-40s: beg txIdx   = %08x, txLast = %08x\n", __func__, xaui->txIdx,xaui->txLast);

  /* count the size */
  for(n = m; n != NULL; n = n->m_next)
    size += n->m_len;

  frame = (Xaui_Header*)Xaui_Alloc(xaui->txDevice,size+XAUI_HDR_SHIFT);
  if (!frame) {
    RTEMS_SYSLOG("%s: unable to allocate tx frame buffer\n",__func__);
    ++xaui->stats.tx_no_buffers;
    return RTEMS_NO_MEMORY;
  }

  dst = (uint8_t*)frame+XAUI_HDR_SHIFT;
    
  for(n = m, size = 0; n != NULL; n = n->m_next) {
    /* Send the data from the mbuf to the interface, one mbuf at a time.
       The size of the data in each mbuf is kept in the ->m_len variable. */

    PRINTK("%-40s: %02" PRIu32 ": %u %s\n", __func__,
           txIdx, n->m_len, (n->m_next == NULL) ? "L" : "");

    DUMP4("mb:", __func__, false, n, sizeof(*n));
    DUMP2("md:", __func__, true,  n->m_data, n->m_len);
    
    PRINTK("%s: mbuf 0x%x m_data 0x%x m_ext 0x%x flags 0x%x type %d next 0x%x\n",__func__,n,n->m_data,n->m_ext.ext_buf,n->m_flags,n->m_type,n->m_next);

    if (mbufs[xaui->txIdx] != NULL) {
      RTEMS_SYSLOG("mbuf not available for index %d\n", xaui->txIdx);
      return RTEMS_RESOURCE_IN_USE;
    }

    mbufs[xaui->txIdx++] = n;
        
   if(xaui->txIdx >= TXBD_CNT)
     {
     PRINTK("%s: wrap tx at %d\n",__func__,xaui->txIdx);
     xaui->txIdx = 0;
     }
           
    /* SEM: unfortunately, we have to copy over data from the mbuf chain */    
    src = mtod(n, char *);
    
    memcpy(dst,src,n->m_len);
    
    dst  += n->m_len;
    src  += n->m_len;
    size += n->m_len;
    ++cnt;
  }

  xaui->txfree -= cnt;;
  
  PRINTK("%-40s: Xaui_Post, hdr = %08x size = %d\n", __func__, header,size);
    
  Xaui_Post(xaui->txDevice, frame,size+XAUI_HDR_SHIFT);
  
  /* Increment transmitted frames counter */
  ++xaui->stats.txd_frames;

  /* Process completed TXs */
  eth_xaui_process_tx(xaui);
    
  PRINTK("%-40s: txd_frames = %08x\n", __func__, xaui->stats.txd_frames);

  PRINTK("%-40s: end txIdx   = %08x, txLast = %08x\n", __func__, xaui->txIdx,xaui->txLast);

  PRINTK("%-40s: end, status = %d\n", __func__, xs);
  
  return RTEMS_SUCCESSFUL;
}

/*!
 * eth_xaui_process_rx():
 *
 * @brief Callback handler for inbound frames.
 *
 */

void eth_xaui_process_rx(EthXaui             *xaui,
                         struct ether_header *header,
                         unsigned             size)
{
  struct mbuf *m;
  
  if(size)
    {
    rtems_bsdnet_semaphore_obtain();
    
    /* Allocated an mbuf */
    MGETHDR(m, M_NOWAIT, MT_DATA);
    if (m == NULL) {
      ++xaui->stats.rx_memErr;
      ++xaui->stats.rx_drop;
      Xaui_Free(xaui->rxDevice,header,size);
      RTEMS_SYSLOG("%s: unable to allocate mbuf\n",__func__);
    }
        
    if(m)
      {
      PRINTK("%s: alloc mbuf 0x%x m_data 0x%x size %d m_ext 0x%x flags 0x%x type %d next 0x%x\n",__func__,m,m->m_data,size,m->m_ext.ext_buf,m->m_flags,m->m_type,m->m_next);

      /* Adjust by two bytes for proper IP header alignment */
      m->m_data  = (void*)((uint32_t)header + ETH_XAUI_RX_DATA_OFFSET);
      m->m_len   = size;

      struct ifnet *const  ifp = &xaui->arpcom.ac_if;
      struct ether_header *eh  = mtod(m, struct ether_header*);

      /* Update mbuf */
      m->m_ext.ext_buf  = (void*)header;
      m->m_ext.ext_size = size;
      m->m_ext.ext_free = (void*)eth_xaui_free_rx;
      m->m_flags       |= M_EXT;  
      m->m_pkthdr.rcvif = ifp;

      DUMP4("mb:", __func__, false, m, sizeof(*m));
      DUMP2("eh:", __func__, true,  eh, size);

      /* Increment received frames counter */
      ++xaui->stats.rxd_frames;
      ++xaui->stats.rx_interrupts;

      m->m_pkthdr.len   = m->m_len        - ETHER_HDR_LEN;
      m->m_len          = m->m_len        - ETHER_HDR_LEN;
      m->m_data         = mtod(m, char *) + ETHER_HDR_LEN;

      PRINTK("%s: attach dma mbuf 0x%x m_data 0x%x size %d m_ext 0x%x flags 0x%x\n",__func__,m,m->m_data,size,m->m_ext.ext_buf,m->m_flags);

      /* Hand it over */
      ether_input(ifp, eh, m);
      }
      
    rtems_bsdnet_semaphore_release();
    }    
}
