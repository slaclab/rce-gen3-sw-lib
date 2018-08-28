// -*-Mode: C++;-*-
/*!@file     ethPio.hh
*
* @brief     Driver for programmed IO ethernet implemented in Xilinx firmware.
*
*            Patterned after:
*              Driver for Xilinx plb temac v3.00a
*
*              Author: Keith Robertson <kjrobert@alumni.uwaterloo.ca>
*              Copyright (c) 2007 Linn Products Ltd, Scotland.
*
*              The license and distribution terms for this file may be
*              found in the file LICENSE in this distribution or at
*              http://www.rtems.com/license/LICENSE.
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      June 26, 2012 -- Created
*
* $Revision: 3121 $
*
* @verbatim                    Copyright 2012
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
//#ifndef DEBUG
//#  define DEBUG
//#endif

#ifndef __INSIDE_RTEMS_BSD_TCPIP_STACK__
#define __INSIDE_RTEMS_BSD_TCPIP_STACK__
#endif

#ifndef __BSD_VISIBLE
#define __BSD_VISIBLE
#endif

#include <rtems.h>
#include <rtems/bspIo.h>
#include <rtems/rtems_bsdnet.h>

extern "C" {
#include <sys/mbuf.h>

#include <sys/socket.h>
#include <sys/sockio.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <net/ethernet.h>
}

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include <rtems/irq.h>


#include "ethPio/impl/ethPio.hh"
#include "ethPio/impl/etherIO.hh"


namespace driver {

  namespace ethPio {


#define NUM_ETHPIO_UNITS 1

extern rtems_isr ethPioIsr( void *handle );
extern void      ethPioIsrOn(const rtems_irq_connect_data *);
extern void      ethPioIsrOff(const rtems_irq_connect_data *);
extern int       ethPioIsrIsOn(const rtems_irq_connect_data *);

void ethPioInit( void *voidptr );
void ethPioReset(struct ifnet *ifp);
void ethPioStop(struct ifnet *ifp);
void ethPioSend(struct ifnet *ifp);
void ethPioStart(struct ifnet *ifp);
void ethPioPrintStats(struct ifnet *ifp);

void ethPioRxThread( void *ignore );
void ethPioTxThread( void *ignore );

static struct EthPio gEthPio[ NUM_ETHPIO_UNITS ];

static rtems_id      gEthPioRxThread = 0;
static rtems_id      gEthPioTxThread = 0;

/*
** Events, one per unit.  The event is sent to the rx task from the isr
** or from the stack to the tx task whenever a unit needs service.  The
** rx/tx tasks identify the requesting unit(s) by their particular
** events so only requesting units are serviced.
*/

static rtems_event_set gUnitSignals[ NUM_ETHPIO_UNITS ]= { RTEMS_EVENT_1 };


uint32_t ethPioTxFifoVacancyBytes(struct ifnet *ifp)
{
  struct EthPio* ethPio = (struct EthPio*)ifp->if_softc;
  uint32_t bytes;
  if (ethPio->cmdFifoDepth < 1) {
    /* If there's no room in the transmit length fifo, then any room in the
     * data fifo is irrelevant, return 0 */
    bytes = 0;
    ethPio->iStats.iCmdFifoFull++;
  } else {
    bytes = ethPio->txFifoDepth;
    bytes *= 8;                         /// @todo - (RiC) Why's this?
  }
  return bytes;
}

void ethPioFifoRead64(uint64_t* aBuf, uint32_t aBytes)
{
  uint32_t numqwords = aBytes >> 3;
  uint32_t xtrabytes = aBytes & 0x7;
  uint     i;

  EtherIO   eio;
  uint64_t* buf = &aBuf[-1];
  for (i = 0; i < numqwords; i++)
  {
    buf = eio.receiveU(buf, sizeof(*buf)); // Note this instruction preincrements
  }

  /* If there was a non qword sized read */
  if (xtrabytes)
  {
    uint64_t lastword;

    eio.receive(&lastword, 0);

    uint8_t* bf = (uint8_t*)++buf;
    uint8_t* u8 = (uint8_t*)&lastword;
    switch (xtrabytes)
    {
      case 7:  *bf++ = *u8++;  // Fall through
      case 6:  *bf++ = *u8++;  // Fall through
      case 5:  *bf++ = *u8++;  // Fall through
      case 4:  *bf++ = *u8++;  // Fall through
      case 3:  *bf++ = *u8++;  // Fall through
      case 2:  *bf++ = *u8++;  // Fall through
      case 1:  *bf++ = *u8++;  // Fall through
    }
  }
}

void ethPioFifoWrite64(uint64_t* aBuf, uint32_t aBytes)
{
  uint32_t numqwords = aBytes >> 3;
  uint32_t xtrabytes = aBytes & 0x7;
  uint32_t i;

  // aBuf is aligned to 32 bytes (see below)
  EtherIO   eio;
  uint64_t* buf = &aBuf[-1];
  for (i = 0; i < numqwords; i++)
  {
    buf = eio.transmitU(buf, sizeof(*buf)); // Note this instruction preincrements
  }

  /* If there was a non word sized write */
  if (xtrabytes)
  {
    uint64_t lastword;

    uint8_t* u8 = (uint8_t*)&lastword;
    uint8_t* bf = (uint8_t*)++buf;
    switch (xtrabytes)
    {
      case 7:  *u8++ = *bf++;  // Fall through
      case 6:  *u8++ = *bf++;  // Fall through
      case 5:  *u8++ = *bf++;  // Fall through
      case 4:  *u8++ = *bf++;  // Fall through
      case 3:  *u8++ = *bf++;  // Fall through
      case 2:  *u8++ = *bf++;  // Fall through
      case 1:  *u8++ = *bf++;  // Fall through
    }

    eio.transmit(&lastword, 0);
  }
}

#if 0
static void _copy(char* from, caddr_t to, u_int len)
{
  ethPioFifoRead64((uint64_t*)to, len);
}
#endif


void ethPioStop(struct ifnet *ifp)
{
  ifp->if_flags &= ~IFF_RUNNING;
}

void ethPioStart(struct ifnet *ifp)
{
  ifp->if_flags |= IFF_RUNNING;
}

void ethPioInit( void* arg )
{
  struct EthPio* ethPio = (struct EthPio*)arg;

  /* Initialize the FIFO occupancy accounting */
  ethPio->cmdFifoDepth = XPE_CMD_FIFO_DEPTH;
  ethPio->txFifoDepth  = XPE_TX_FIFO_DEPTH;

  /* Empty out the response and receive FIFOs */
  EtherIO  eio;
  uint32_t j4;
  uint32_t i = XPE_RSP_FIFO_DEPTH;
  while ( !(eio.response(&j4, &j4, &j4) & XPE_RSP_FIFO_EMPTY) )
  {
    uint32_t opcode = j4 & 0xf0000000;
    if ((opcode == XPE_RSP_OPCODE_RECV) || (opcode = XPE_RSP_OPCODE_RBAD))
    {
      uint64_t j8;
      uint     l = ((j4 & 0xffff) + 7) >> 3;
      while (l--)  eio.receive(&j8, 0);
    }
    if (i-- == 0)  break;
  }
}

void ethPioReset(struct ifnet *ifp)
{
   ethPioStop( ifp );
   ethPioStart( ifp );
}

void ethPioPrintStats( struct ifnet *ifp )
{
  struct EthPio* ethPio = (struct EthPio*)ifp->if_softc;

   printf("\n");
   printf("%s: Statistics for interface '%s'\n", DRIVER_PREFIX, ethPio->iUnitName );

   printf("%s:         Rs Bad Opcodes: %lu\n", DRIVER_PREFIX, ethPio->iStats.iBadOpcode);

   for (uint i = 0; i < 16; ++i)
   {
     printf("%s:    Rx OpCode %2u count : %lu\n", DRIVER_PREFIX, i, ethPio->iStats.iRxOpCode[i]);
   }

   printf("%s:            Rx no MBUFs: %lu\n", DRIVER_PREFIX, ethPio->iStats.iRxNoMbufs);
   printf("%s:               Rx Okays: %lu\n", DRIVER_PREFIX, ethPio->iStats.iRxOkay);
   printf("%s:                 Rx Bad: %lu\n", DRIVER_PREFIX, ethPio->iStats.iRxBad);
   printf("%s:             Rx Tx Acks: %lu\n", DRIVER_PREFIX, ethPio->iStats.iRxTxAcks);
   printf("%s:        Rx Stray Events: %lu\n", DRIVER_PREFIX, ethPio->iStats.iRxStrayEvents);
   printf("%s:         Rx Max Drained: %lu\n", DRIVER_PREFIX, ethPio->iStats.iRxMaxDrained);
   printf("%s:         Tx Max Drained: %lu\n", DRIVER_PREFIX, ethPio->iStats.iTxMaxDrained);
   printf("%s:  Tx Rej Data Fifo Full: %lu\n", DRIVER_PREFIX, ethPio->iStats.iTxFifoFull);
   printf("%s: Tx Data Fifo Underflow: %lu\n", DRIVER_PREFIX, ethPio->iStats.iTxUnderflow);
   printf("%s:  Tx Data Fifo Overflow: %lu\n", DRIVER_PREFIX, ethPio->iStats.iTxOverflow);
   printf("%s:      Cmd Rej Fifo Full: %lu\n", DRIVER_PREFIX, ethPio->iStats.iCmdFifoFull);
   printf("%s:     Cmd Fifo Underflow: %lu\n", DRIVER_PREFIX, ethPio->iStats.iCmdUnderflow);
   printf("%s:      Cmd Fifo Overflow: %lu\n", DRIVER_PREFIX, ethPio->iStats.iCmdOverflow);

   printf("\n");
}


int32_t ethPioSetMulticastFilter(struct ifnet *ifp)
{
  return 0;
}

int ethPioIoctl(struct ifnet* ifp, ioctl_command_t   aCommand, caddr_t aData)
{
  struct EthPio* ethPio = (struct EthPio*)ifp->if_softc;
  int32_t        error  = 0;

  switch(aCommand) {
    case SIOCGIFADDR:
    case SIOCSIFADDR:
      ether_ioctl(ifp, aCommand, aData);
      break;

    case SIOCSIFFLAGS:
      switch(ifp->if_flags & (IFF_UP | IFF_RUNNING))
      {
        case IFF_RUNNING:
          ethPioStop(ifp);
          break;

        case IFF_UP:
          ethPioStart(ifp);
          break;

        case IFF_UP | IFF_RUNNING:
          ethPioReset(ifp);
          break;

        default:
          break;
      }
      break;

    case SIOCADDMULTI:
    case SIOCDELMULTI: {
        struct ifreq* ifr = (struct ifreq*) aData;
        error = ((aCommand == SIOCADDMULTI) ?
                 ( ether_addmulti(ifr, &(ethPio->iArpcom)) ) :
                 ( ether_delmulti(ifr, &(ethPio->iArpcom)))
           );
        /* ENETRESET indicates that driver should update its multicast filters */
        if (error == ENETRESET)
        {
            error = ethPioSetMulticastFilter( ifp );
        }
        break;
    }

    case SIO_RTEMS_SHOW_STATS:
      ethPioPrintStats( ifp );
      break;

    default:
      error = EINVAL;
      break;
  }
  return error;
}

void ethPioSend(struct ifnet* ifp)
{
  struct EthPio* ethPio = (struct EthPio*)ifp->if_softc;

  /* wake up tx thread w/ outbound interface's signal */
  rtems_event_send( gEthPioTxThread, ethPio->iIoEvent );

  ifp->if_flags |= IFF_OACTIVE;
}

/* align the tx buffer to 32 bytes just for kicks, should make it more
 * cache friendly */
static unsigned char gTxBuf[2048] __attribute__ ((aligned (32)));

void ethPioSendPacket(struct ifnet *ifp, struct mbuf* aMbuf)
{
  struct EthPio* ethPio = (struct EthPio*)ifp->if_softc;
  struct mbuf*   n      = aMbuf;
  uint32_t       len    = 0;

#ifdef DEBUG
  printk ("SendPacket: aMbuf = %p\n", n);
  printk ("TXD: 0x%08x, len = %08x\n", (int32_t) n->m_data, n->m_len);
#endif

  /* assemble the packet into the tx buffer */
  // @todo - RiC: Why can't we write the mbuf contents directly to the interface?
#if 1
  for (;;) {
    if ( n->m_len > 0 ) {
      memcpy( &gTxBuf[ len ], mtod(n, char*), n->m_len);
      len += n->m_len;
    }
    if ( (n = n->m_next) == 0) {
      break;
    }
  }
#else
  if (unsigned(n->m_pkthdr.len) <= sizeof(struct ether_header))
  {
    m_copydata(n, 0, n->m_pkthdr.len, (caddr_t)gTxBuf);
    len = sizeof(struct ether_header);
  }
  else
  {
    unsigned payloadsize = n->m_pkthdr.len - sizeof(struct ether_header);
    m_copydata(n, 0, sizeof(struct ether_header), (caddr_t)gTxBuf);
    m_copydata(n, sizeof(struct ether_header), payloadsize, (caddr_t)&gTxBuf[sizeof(struct ether_header)]);
    len = sizeof(struct ether_header) + payloadsize;
  }
#endif

  ethPioFifoWrite64( (uint64_t*)gTxBuf, len );
  ethPio->txFifoDepth -= len >> 3;    // Number of quadwords
  if (ethPio->txFifoDepth < 0)
  {
    ethPio->iStats.iTxOverflow++;
    ethPio->txFifoDepth = 0;
  }
  /* Set the Transmit Packet Length Register which registers the packet
   * length, enqueues the packet and signals the xmit unit to start
   * sending. */
  EtherIO().command(XPE_CTL_OPCODE_XMIT, ethPio->txSeq++, len);
  if (--ethPio->cmdFifoDepth < 0)
  {
    ethPio->iStats.iCmdOverflow++;
    ethPio->cmdFifoDepth = 0;
  }

#ifdef DEBUG
  printk ("%s: txpkt, len %d\n", DRIVER_PREFIX, len );
  memset(gTxBuf, 0, len);
#endif
}

void ethPioTxThreadSingle(struct ifnet* ifp)
{
  struct EthPio* ethPio = (struct EthPio*)ifp->if_softc;
  struct mbuf*   m;

#ifdef DEBUG
  printk ("%s: tx send packet, interface '%s'\n", DRIVER_PREFIX, ethPio->iUnitName );
#endif

  /* Send packets until mbuf queue empty or tx fifo full */
  for (;;)
  {
    /* Check if enough space in tx data fifo _and_ tx tplr for an entire
       ethernet frame */
    if ( ethPioTxFifoVacancyBytes( ifp ) <= ifp->if_mtu ) {
      /* If not, break out of loop to wait for space */
      ethPio->pendingSend = true;
      break;
    }

    /* Continue to dequeue mbuf chains until none left */
    IF_DEQUEUE( &(ifp->if_snd), m);
    if ( !m ) {
      ethPio->pendingSend = false;
      break;
    }

    /* Send dequeued mbuf chain */
    ethPioSendPacket( ifp, m );

    /* Free mbuf chain */
    m_freem( m );
  }
  ifp->if_flags &= ~IFF_OACTIVE;
}

void ethPioTxThread( void *ignore )
{
  struct EthPio   *ethPio;
  struct ifnet    *ifp;

  rtems_event_set  events;
  int              i;

  for (;;) {
    /* Wait for:
       - notification from stack of packet to send OR
       - notification from interrupt handler that there is space available to
         send already queued packets
    */
    rtems_bsdnet_event_receive( RTEMS_ALL_EVENTS,
                                RTEMS_EVENT_ANY | RTEMS_WAIT,
                                RTEMS_NO_TIMEOUT,
                                &events );

    for (i=0; i< NUM_ETHPIO_UNITS; i++) {
      ethPio = &gEthPio[i];

      if ( ethPio->iIsPresent ) {
        ifp = ethPio->iIfp;

        if ( (ifp->if_flags & IFF_RUNNING) ) {

          if ( events & ethPio->iIoEvent ) {
            ethPioTxThreadSingle(ifp);
          }

        } else {
          printk ("%s: ethPioTxThread: event received for device: %s, but device not active\n",
            DRIVER_PREFIX, ethPio->iUnitName);
          assert(0);
        }
      }
    }
  }
}

void ethPioRxThreadSingle(struct ifnet* ifp)
{
  struct EthPio* ethPio = (struct EthPio*)ifp->if_softc;

  uint32_t npkts = 0;
#ifdef DEBUG
  printk ("%s: RxThread, packet rx on interface %s\n", DRIVER_PREFIX, ethPio->iUnitName );
#endif

  /* While the response FIFO is not empty */
  uint32_t opcode;
  uint32_t tid;
  uint32_t response;
  EtherIO  eio;
  while ( !(eio.response(&opcode, &tid, &response) & XPE_RSP_FIFO_EMPTY) )
  {
    /* Decode the opcode */
    ethPio->iStats.iRxOpCode[opcode >> 28]++;
    npkts++;

    switch (opcode)
    {
      case XPE_RSP_OPCODE_RECV:         // Receive OK
      {
        /* Decode the length of the packet */
        uint32_t bytes = response;
#if 1
        struct mbuf* m;
        struct ether_header* eh;

        /* Get some memory from the ip stack to store the packet in */
        MGETHDR(m, M_WAIT, MT_DATA);
        MCLGET(m, M_WAIT);

        m->m_pkthdr.rcvif = ifp;

        /* Copy the packet into the ip stack's memory */
        ethPioFifoRead64(mtod(m, uint64_t*), bytes);

        m->m_len        = bytes - sizeof(struct ether_header);
        m->m_pkthdr.len = bytes - sizeof(struct ether_header);

        eh = mtod(m, struct ether_header*);

        m->m_data += sizeof(struct ether_header);
#else
        char* buf = 0;                  // Not used
        struct mbuf*         m  = m_devget(buf, bytes, 0, ifp, _copy);
        if (!m) {
          ethPio->iStats.iRxNoMbufs++;
          break;
        }
        struct ether_header* eh = mtod(m, struct ether_header*);

        /* Remove header from mbuf and pass it on. */
        m_adj(m, sizeof(struct ether_header));
#endif

        ifp->if_ipackets++;

        /* Tell the ip stack about the received packet */
        ether_input(ifp, eh, m);
        ethPio->iStats.iRxOkay++;

        break;
      }
      case XPE_RSP_OPCODE_RBAD:         // Receive Bad
      {
        uint     l = (response + 7) >> 3;
        uint64_t junk64;
        while (l--)  eio.receive(&junk64, 0);

        ethPio->iStats.iRxBad++;
        break;
      }
      case XPE_RSP_OPCODE_TACK:         // Transmit Acknowledge
      {
        ethPio->txFifoDepth += response >> 3;
        if (ethPio->txFifoDepth > XPE_TX_FIFO_DEPTH)
        {
          ethPio->iStats.iTxUnderflow++;
          ethPio->cmdFifoDepth = XPE_TX_FIFO_DEPTH;
        }
        if (++ethPio->cmdFifoDepth > XPE_CMD_FIFO_DEPTH)
        {
          ethPio->iStats.iCmdUnderflow++;
          ethPio->cmdFifoDepth = XPE_CMD_FIFO_DEPTH;
        }
        ethPio->iStats.iRxTxAcks++;

        /* If there's a pending send, wake up the send process */
        if (ethPio->pendingSend)  ethPioSend(ifp);
        break;
      }
      default:
      {
        printk ("RxThread: Unrecognized Opcode \n");

        ethPio->iStats.iBadOpcode++;
        break;
      }
    }
  }

#ifdef DEBUG
  if (npkts) // RiC
    printk ("%s: RxThread, retrieved %d packets\n", DRIVER_PREFIX, npkts );
#endif
  if (npkts > ethPio->iStats.iRxMaxDrained)
    ethPio->iStats.iRxMaxDrained = npkts;
}

void ethPioRxThread( void *ignore )
{
  struct EthPio*   ethPio;
  struct ifnet*    ifp;
  int              i;
  rtems_event_set  events;
  uint32_t         tmoTicks = rtems_bsdnet_ticks_per_second / 1000;
  if (tmoTicks == 0)  tmoTicks = 1;

#ifdef DEBUG
  printk ("%s: ethPioRxThread polling every %lu ticks, # ticks/sec = %u, MSIZE = %u\n",
         DRIVER_PREFIX, tmoTicks, rtems_bsdnet_ticks_per_second, MSIZE);
#endif

  /* This driver doesn't use interrupts, so we poll */
  for (;;) {
    rtems_bsdnet_event_receive( RTEMS_ALL_EVENTS,
                                RTEMS_WAIT | RTEMS_EVENT_ANY,
                                tmoTicks,
                                &events);

#ifdef DEBUG
    printk ("%s: RxThread, wake\n", DRIVER_PREFIX );
#endif

    for (i=0; i< NUM_ETHPIO_UNITS; i++) {
      ethPio = &gEthPio[i];

      if ( ethPio->iIsPresent ) {
        ifp = ethPio->iIfp;

        if ( (ifp->if_flags & IFF_RUNNING) != 0 ) {
          //if ( events & ethPio->iIoEvent ) { // also on tmos until interrupts work
            ethPioRxThreadSingle(ifp);
            //}
        }
        else {
          printk ("%s: RxThread, interface %s present but not running\n", DRIVER_PREFIX, ethPio->iUnitName );
          assert(0);
        }
      }
    }
  }
}

extern "C" int ethPio_driver_attach(struct rtems_bsdnet_ifconfig* aBsdConfig, int aDummy)
{
   struct ifnet*  ifp;
   int32_t        unit;
   char*          unitName;
   struct EthPio* ethPio;

   unit = rtems_bsdnet_parse_driver_name(aBsdConfig, &unitName);
   if (unit < 0)
   {
      printk ("%s: Interface not found (%d)\n", __func__, unit);
      return 0;
   }

   if (aBsdConfig->hardware_address == NULL)
   {
      printk ("%s: No MAC address given for interface '%s'\n", DRIVER_PREFIX, unitName );
      return 0;
   }

   ethPio = &gEthPio[ unit ];
   memset(ethPio, 0, sizeof(struct EthPio));

   ethPio->iIsPresent = 1;

   snprintf(ethPio->iUnitName, MAX_UNIT_BYTES, "%s%" PRId32, unitName, unit);

   ethPio->iIfp     = &(ethPio->iArpcom.ac_if);
   ifp              = &(ethPio->iArpcom.ac_if);
   ethPio->iIoEvent = gUnitSignals[ unit ];

   memcpy( ethPio->iArpcom.ac_enaddr, aBsdConfig->hardware_address, ETHER_ADDR_LEN);

   ifp->if_softc  = ethPio;
   ifp->if_unit   = unit;
   ifp->if_name   = unitName;
   ifp->if_mtu    = aBsdConfig->mtu ? aBsdConfig->mtu : ETHERMTU;
   ifp->if_init   = ethPioInit;
   ifp->if_ioctl  = ethPioIoctl;
   ifp->if_start  = ethPioSend;
   ifp->if_output = ether_output;

   ifp->if_flags  =  IFF_BROADCAST | IFF_SIMPLEX | IFF_MULTICAST;

   if (ifp->if_snd.ifq_maxlen == 0)  ifp->if_snd.ifq_maxlen = ifqmaxlen;

   if_attach(ifp);
   ether_ifattach(ifp);

   /* create shared rx & tx threads */
   if ( (gEthPioRxThread == 0) && (gEthPioTxThread == 0) )
   {
      gEthPioRxThread = rtems_bsdnet_newproc((char*)"xerx", 4096, ethPioRxThread, NULL );
      gEthPioTxThread = rtems_bsdnet_newproc((char*)"xetx", 4096, ethPioTxThread, NULL );
   }

   return 1;
}

  } // ethPio

} // driver
