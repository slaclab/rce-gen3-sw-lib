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
* $Revision: 1242 $
*
* @verbatim                    Copyright 2012
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef _DRIVER_ETHPIO_ETHPIO_HH_
#define _DRIVER_ETHPIO_ETHPIO_HH_
#include <rtems/irq.h>


#define ETHPIO_DRIVER_PREFIX   "ethPio"

#define DRIVER_PREFIX ETHPIO_DRIVER_PREFIX


/** IPIF interrupt and reset registers
 */
#define XPE_DISR_OFFSET  0x00000000  /**< Device interrupt status */
#define XPE_DIPR_OFFSET  0x00000004  /**< Device interrupt pending */
#define XPE_DIER_OFFSET  0x00000008  /**< Device interrupt enable */
#define XPE_DIIR_OFFSET  0x00000018  /**< Device interrupt ID */
#define XPE_DGIE_OFFSET  0x0000001C  /**< Device global interrupt enable */
#define XPE_IPISR_OFFSET 0x00000020  /**< IP interrupt status */
#define XPE_IPIER_OFFSET 0x00000028  /**< IP interrupt enable */
#define XPE_DSR_OFFSET   0x00000040  /**< Device software reset (write) */

/** IPIF transmit fifo
 */
#define XPE_PFIFO_TX_BASE_OFFSET    0x00002000  /**< Packet FIFO Tx channel */
#define XPE_PFIFO_TX_VACANCY_OFFSET 0x00002004  /**< Packet Fifo Tx Vacancy */
#define XPE_PFIFO_TX_DATA_OFFSET    0x00002100  /**< IPIF Tx packet fifo port */

/** IPIF receive fifo
 */
#define XPE_PFIFO_RX_BASE_OFFSET    0x00002010  /**< Packet FIFO Rx channel */
#define XPE_PFIFO_RX_VACANCY_OFFSET 0x00002014  /**< Packet Fifo Rx Vacancy */
#define XPE_PFIFO_RX_DATA_OFFSET    0x00002200  /**< IPIF Rx packet fifo port */

/** IPIF fifo masks
 */
#define XPE_PFIFO_COUNT_MASK     0x00FFFFFF

/** IPIF transmit and recieve DMA offsets
 */
#define XPE_DMA_SEND_OFFSET      0x00002300  /**< DMA Tx channel */
#define XPE_DMA_RECV_OFFSET      0x00002340  /**< DMA Rx channel */

/** IPIF IPIC_TO_TEMAC Core Registers
 */
#define XPE_CR_OFFSET           0x00001000  /**< Control */
#define XPE_TPLR_OFFSET         0x00001004  /**< Tx packet length (FIFO) */
#define XPE_TSR_OFFSET          0x00001008  /**< Tx status (FIFO) */
#define XPE_RPLR_OFFSET         0x0000100C  /**< Rx packet length (FIFO) */
#define XPE_RSR_OFFSET          0x00001010  /**< Receive status */
#define XPE_IFGP_OFFSET         0x00001014  /**< Interframe gap */
#define XPE_TPPR_OFFSET         0x00001018  /**< Tx pause packet */

/** TEMAC Core Registers
 * These are registers defined within the device's hard core located in the
 * processor block. They are accessed with the host interface. These registers
 * are addressed offset by XPE_HOST_IPIF_OFFSET or by the DCR base address
 * if so configured.
 */
#define XPE_HOST_IPIF_OFFSET    0x00003000  /**< Offset of host registers when
                                                 memory mapped into IPIF */
#define XPE_ERXC0_OFFSET        (XPE_HOST_IPIF_OFFSET + 0x00000200)  /**< Rx configuration word 0 */
#define XPE_ERXC1_OFFSET        (XPE_HOST_IPIF_OFFSET + 0x00000240)  /**< Rx configuration word 1 */
#define XPE_ETXC_OFFSET         (XPE_HOST_IPIF_OFFSET + 0x00000280)  /**< Tx configuration */
#define XPE_EFCC_OFFSET         (XPE_HOST_IPIF_OFFSET + 0x000002C0)  /**< Flow control configuration */
#define XPE_ECFG_OFFSET         (XPE_HOST_IPIF_OFFSET + 0x00000300)  /**< EMAC configuration */
#define XPE_EGMIC_OFFSET        (XPE_HOST_IPIF_OFFSET + 0x00000320)  /**< RGMII/SGMII configuration */
#define XPE_EMC_OFFSET          (XPE_HOST_IPIF_OFFSET + 0x00000340)  /**< Management configuration */
#define XPE_EUAW0_OFFSET        (XPE_HOST_IPIF_OFFSET + 0x00000380)  /**< Unicast address word 0 */
#define XPE_EUAW1_OFFSET        (XPE_HOST_IPIF_OFFSET + 0x00000384)  /**< Unicast address word 1 */
#define XPE_EMAW0_OFFSET        (XPE_HOST_IPIF_OFFSET + 0x00000388)  /**< Multicast address word 0 */
#define XPE_EMAW1_OFFSET        (XPE_HOST_IPIF_OFFSET + 0x0000038C)  /**< Multicast address word 1 */
#define XPE_EAFM_OFFSET         (XPE_HOST_IPIF_OFFSET + 0x00000390)  /**< Promisciuous mode */
#define XPE_EIRS_OFFSET         (XPE_HOST_IPIF_OFFSET + 0x000003A0)  /**< IRstatus */
#define XPE_EIREN_OFFSET        (XPE_HOST_IPIF_OFFSET + 0x000003A4)  /**< IRenable */
#define XPE_EMIID_OFFSET        (XPE_HOST_IPIF_OFFSET + 0x000003B0)  /**< MIIMwrData */
#define XPE_EMIIC_OFFSET        (XPE_HOST_IPIF_OFFSET + 0x000003B4)  /**< MiiMcnt */

/* Register masks. The following constants define bit locations of various
 * control bits in the registers. Constants are not defined for those registers
 * that have a single bit field representing all 32 bits. For further
 * information on the meaning of the various bit masks, refer to the HW spec.
 */

/** Interrupt status bits for top level interrupts
 *  These bits are associated with the XPE_DISR_OFFSET, XPE_DIPR_OFFSET,
 *  and XPE_DIER_OFFSET registers.
 */
#define XPE_DXR_SEND_FIFO_MASK          0x00000040 /**< Send FIFO channel */
#define XPE_DXR_RECV_FIFO_MASK          0x00000020 /**< Receive FIFO channel */
#define XPE_DXR_RECV_DMA_MASK           0x00000010 /**< Receive DMA channel */
#define XPE_DXR_SEND_DMA_MASK           0x00000008 /**< Send DMA channel */
#define XPE_DXR_CORE_MASK               0x00000004 /**< Core */
#define XPE_DXR_DPTO_MASK               0x00000002 /**< Data phase timeout */
#define XPE_DXR_TERR_MASK               0x00000001 /**< Transaction error */

/** Interrupt status bits for MAC interrupts
 *  These bits are associated with XPE_IPISR_OFFSET and XPE_IPIER_OFFSET
 *  registers.
 */
#define XPE_IPXR_XMIT_DONE_MASK         0x00000001 /**< Tx complete */
#define XPE_IPXR_RECV_DONE_MASK         0x00000002 /**< Rx complete */
#define XPE_IPXR_AUTO_NEG_MASK          0x00000004 /**< Auto negotiation complete */
#define XPE_IPXR_RECV_REJECT_MASK       0x00000008 /**< Rx packet rejected */
#define XPE_IPXR_XMIT_SFIFO_EMPTY_MASK  0x00000010 /**< Tx status fifo empty */
#define XPE_IPXR_RECV_LFIFO_EMPTY_MASK  0x00000020 /**< Rx length fifo empty */
#define XPE_IPXR_XMIT_LFIFO_FULL_MASK   0x00000040 /**< Tx length fifo full */
#define XPE_IPXR_RECV_LFIFO_OVER_MASK   0x00000080 /**< Rx length fifo overrun
                                                        Note that this signal is
                                                        no longer asserted by HW
                                                        */
#define XPE_IPXR_RECV_LFIFO_UNDER_MASK  0x00000100 /**< Rx length fifo underrun */
#define XPE_IPXR_XMIT_SFIFO_OVER_MASK   0x00000200 /**< Tx status fifo overrun */
#define XPE_IPXR_XMIT_SFIFO_UNDER_MASK  0x00000400 /**< Tx status fifo underrun */
#define XPE_IPXR_XMIT_LFIFO_OVER_MASK   0x00000800 /**< Tx length fifo overrun */
#define XPE_IPXR_XMIT_LFIFO_UNDER_MASK  0x00001000 /**< Tx length fifo underrun */
#define XPE_IPXR_RECV_PFIFO_ABORT_MASK  0x00002000 /**< Rx packet rejected due to
                                                        full packet FIFO */
#define XPE_IPXR_RECV_LFIFO_ABORT_MASK  0x00004000 /**< Rx packet rejected due to
                                                        full length FIFO */

#define XPE_IPXR_RECV_DROPPED_MASK                                      \
    (XPE_IPXR_RECV_REJECT_MASK |                                        \
     XPE_IPXR_RECV_PFIFO_ABORT_MASK |                                   \
     XPE_IPXR_RECV_LFIFO_ABORT_MASK)    /**< IPXR bits that indicate a dropped
                                             receive frame */
#define XPE_IPXR_XMIT_ERROR_MASK                                        \
    (XPE_IPXR_XMIT_SFIFO_OVER_MASK |                                    \
     XPE_IPXR_XMIT_SFIFO_UNDER_MASK |                                   \
     XPE_IPXR_XMIT_LFIFO_OVER_MASK |                                    \
     XPE_IPXR_XMIT_LFIFO_UNDER_MASK)    /**< IPXR bits that indicate transmit
                                             errors */

#define XPE_IPXR_RECV_ERROR_MASK                                        \
    (XPE_IPXR_RECV_DROPPED_MASK |                                       \
     XPE_IPXR_RECV_LFIFO_UNDER_MASK)    /**< IPXR bits that indicate receive
                                             errors */

#define XPE_IPXR_FIFO_FATAL_ERROR_MASK          \
    (XPE_IPXR_XMIT_SFIFO_OVER_MASK |            \
     XPE_IPXR_XMIT_SFIFO_UNDER_MASK |           \
     XPE_IPXR_XMIT_LFIFO_OVER_MASK |            \
     XPE_IPXR_XMIT_LFIFO_UNDER_MASK |           \
     XPE_IPXR_RECV_LFIFO_UNDER_MASK)    /**< IPXR bits that indicate errors with
                                             one of the length or status FIFOs
                                             that is fatal in nature. These bits
                                             can only be cleared by a device
                                             reset */

/** Software reset register (DSR)
 */
#define XPE_DSR_RESET_MASK      0x0000000A  /**< Write this value to DSR to
                                                 reset entire core */


/** Global interrupt enable register (DGIE)
 */
#define XPE_DGIE_ENABLE_MASK    0x80000000  /**< Write this value to DGIE to
                                                 enable interrupts from this
                                                 device */

/** Control Register (CR)
 */
#define XPE_CR_HTRST_MASK       0x00000008   /**< Reset hard temac */
#define XPE_CR_BCREJ_MASK       0x00000004   /**< Disable broadcast address
                                                  filtering */
#define XPE_CR_MCREJ_MASK       0x00000002   /**< Disable multicast address
                                                  filtering */
#define XPE_CR_HDUPLEX_MASK     0x00000001   /**< Enable half duplex operation */


/** Transmit Packet Length Register (TPLR)
 */
#define XPE_TPLR_TXPL_MASK      0x00003FFF   /**< Tx packet length in bytes */


/** Transmit Status Register (TSR)
 */
#define XPE_TSR_TXED_MASK       0x80000000   /**< Excess deferral error */
#define XPE_TSR_PFIFOU_MASK     0x40000000   /**< Packet FIFO underrun */
#define XPE_TSR_TXA_MASK        0x3E000000   /**< Transmission attempts */
#define XPE_TSR_TXLC_MASK       0x01000000   /**< Late collision error */
#define XPE_TSR_TPCF_MASK       0x00000001   /**< Transmit packet complete
                                                  flag */

#define XPE_TSR_ERROR_MASK                 \
    (XPE_TSR_TXED_MASK |                   \
     XPE_TSR_PFIFOU_MASK |                 \
     XPE_TSR_TXLC_MASK)                      /**< TSR bits that indicate an
                                                  error */


/** Receive Packet Length Register (RPLR)
 */
#define XPE_RPLR_RXPL_MASK      0x00003FFF   /**< Rx packet length in bytes */


/** Receive Status Register (RSR)
 */
#define XPE_RSR_RPCF_MASK       0x00000001   /**< Receive packet complete
                                                  flag */

/** Interframe Gap Register (IFG)
 */
#define XPE_IFG_IFGD_MASK       0x000000FF   /**< IFG delay */


/** Transmit Pause Packet Register (TPPR)
 */
#define XPE_TPPR_TPPD_MASK      0x0000FFFF   /**< Tx pause packet data */


/** Receiver Configuration Word 1 (ERXC1)
 */
#define XPE_ERXC1_RXRST_MASK    0x80000000   /**< Receiver reset */
#define XPE_ERXC1_RXJMBO_MASK   0x40000000   /**< Jumbo frame enable */
#define XPE_ERXC1_RXFCS_MASK    0x20000000   /**< FCS not stripped */
#define XPE_ERXC1_RXEN_MASK     0x10000000   /**< Receiver enable */
#define XPE_ERXC1_RXVLAN_MASK   0x08000000   /**< VLAN enable */
#define XPE_ERXC1_RXHD_MASK     0x04000000   /**< Half duplex */
#define XPE_ERXC1_RXLT_MASK     0x02000000   /**< Length/type check disable */
#define XPE_ERXC1_ERXC1_MASK    0x0000FFFF   /**< Pause frame source address
                                                  bits [47:32]. Bits [31:0]
                                                  are stored in register
                                                  ERXC0 */


/** Transmitter Configuration (ETXC)
 */
#define XPE_ETXC_TXRST_MASK     0x80000000   /**< Transmitter reset */
#define XPE_ETXC_TXJMBO_MASK    0x40000000   /**< Jumbo frame enable */
#define XPE_ETXC_TXFCS_MASK     0x20000000   /**< Generate FCS */
#define XPE_ETXC_TXEN_MASK      0x10000000   /**< Transmitter enable */
#define XPE_ETXC_TXVLAN_MASK    0x08000000   /**< VLAN enable */
#define XPE_ETXC_TXHD_MASK      0x04000000   /**< Half duplex */
#define XPE_ETXC_TXIFG_MASK     0x02000000   /**< IFG adjust enable */


/** Flow Control Configuration (EFCC)
 */
#define XPE_EFCC_TXFLO_MASK     0x40000000   /**< Tx flow control enable */
#define XPE_EFCC_RXFLO_MASK     0x20000000   /**< Rx flow control enable */


/** EMAC Configuration (ECFG)
 */
#define XPE_ECFG_LINKSPD_MASK   0xC0000000   /**< Link speed */
#define XPE_ECFG_RGMII_MASK     0x20000000   /**< RGMII mode enable */
#define XPE_ECFG_SGMII_MASK     0x10000000   /**< SGMII mode enable */
#define XPE_ECFG_1000BASEX_MASK 0x08000000   /**< 1000BaseX mode enable */
#define XPE_ECFG_HOSTEN_MASK    0x04000000   /**< Host interface enable */
#define XPE_ECFG_TX16BIT        0x02000000   /**< 16 bit Tx client enable */
#define XPE_ECFG_RX16BIT        0x01000000   /**< 16 bit Rx client enable */

#define XPE_ECFG_LINKSPD_10     0x00000000   /**< XPE_ECFG_LINKSPD_MASK for
                                                  10 Mbit */
#define XPE_ECFG_LINKSPD_100    0x40000000   /**< XPE_ECFG_LINKSPD_MASK for
                                                  100 Mbit */
#define XPE_ECFG_LINKSPD_1000   0x80000000   /**< XPE_ECFG_LINKSPD_MASK for
                                                  1000 Mbit */

/** EMAC RGMII/SGMII Configuration (EGMIC)
 */
#define XPE_EGMIC_RGLINKSPD_MASK    0xC0000000   /**< RGMII link speed */
#define XPE_EGMIC_SGLINKSPD_MASK    0x0000000C   /**< SGMII link speed */
#define XPE_EGMIC_RGSTATUS_MASK     0x00000002   /**< RGMII link status */
#define XPE_EGMIC_RGHALFDUPLEX_MASK 0x00000001   /**< RGMII half duplex */

#define XPE_EGMIC_RGLINKSPD_10      0x00000000   /**< XPE_EGMIC_RGLINKSPD_MASK
                                                      for 10 Mbit */
#define XPE_EGMIC_RGLINKSPD_100     0x40000000   /**< XPE_EGMIC_RGLINKSPD_MASK
                                                      for 100 Mbit */
#define XPE_EGMIC_RGLINKSPD_1000    0x80000000   /**< XPE_EGMIC_RGLINKSPD_MASK
                                                      for 1000 Mbit */
#define XPE_EGMIC_SGLINKSPD_10      0x00000000   /**< XPE_SGMIC_RGLINKSPD_MASK
                                                      for 10 Mbit */
#define XPE_EGMIC_SGLINKSPD_100     0x00000004   /**< XPE_SGMIC_RGLINKSPD_MASK
                                                      for 100 Mbit */
#define XPE_EGMIC_SGLINKSPD_1000    0x00000008   /**< XPE_SGMIC_RGLINKSPD_MASK
                                                      for 1000 Mbit */

/** EMAC Management Configuration (EMC)
 */
#define XPE_EMC_MDIO_MASK       0x00000040   /**< MII management enable */
#define XPE_EMC_CLK_DVD_MAX     0x3F         /**< Maximum MDIO divisor */


/** EMAC Unicast Address Register Word 1 (EUAW1)
 */
#define XPE_EUAW1_MASK          0x0000FFFF   /**< Station address bits [47:32]
                                                  Station address bits [31:0]
                                                  are stored in register
                                                  EUAW0 */


/** EMAC Multicast Address Register Word 1 (EMAW1)
 */
#define XPE_EMAW1_CAMRNW_MASK   0x00800000   /**< CAM read/write control */
#define XPE_EMAW1_CAMADDR_MASK  0x00030000   /**< CAM address mask */
#define XPE_EMAW1_MASK          0x0000FFFF   /**< Multicast address bits [47:32]
                                                  Multicast address bits [31:0]
                                                  are stored in register
                                                  EMAW1 */
#define XPE_EMAW1_CAMMADDR_SHIFT_MASK 16     /**< Number of bits to shift right
                                                  to align with
                                                  XPE_EMAW1_CAMADDR_MASK */


/** EMAC Address Filter Mode (EAFM)
 */
#define XPE_EAFM_EPPRM_MASK         0x80000000   /**< Promiscuous mode enable */


/** EMAC MII Management Write Data (EMIID)
 */
#define XPE_EMIID_MIIMWRDATA_MASK   0x0000FFFF   /**< Data port */


/** EMAC MII Management Control (EMIIC)
 */
#define XPE_EMIID_MIIMDECADDR_MASK  0x0000FFFF   /**< Address port */

/** Control and response opcodes
 */
#define XPE_CTL_OPCODE_XMIT           (5 << 28)
#define XPE_RSP_OPCODE_RECV           (7 << 28)
#define XPE_RSP_OPCODE_RBAD           (6 << 28)
#define XPE_RSP_OPCODE_TACK           (5 << 28)

/** Depth of the two outbound FIFOs
 */
#define XPE_TX_FIFO_DEPTH                 2048
#define XPE_CMD_FIFO_DEPTH                1024
#define XPE_RSP_FIFO_DEPTH                1024
#define XPE_RX_FIFO_DEPTH                 8192

/** Response FIFO condition code values
 */
#define XPE_RSP_FIFO_EMPTY                   (0x1 << 0)
#define XPE_RSP_FIFO_ALMOST_EMPTY            (0x1 << 1)


namespace driver {

  namespace ethPio {

struct EthPioStats
{
  uint32_t iBadOpcode;
  uint32_t iRxOpCode[16];

  uint32_t iRxNoMbufs;
  uint32_t iRxOkay;
  uint32_t iRxBad;
  uint32_t iRxTxAcks;
  uint32_t iRxMaxDrained;
  uint32_t iRxStrayEvents;

  uint32_t iTxMaxDrained;
  uint32_t iTxFifoFull;
  uint32_t iTxUnderflow;
  uint32_t iTxOverflow;

  uint32_t iCmdFifoFull;
  uint32_t iCmdUnderflow;
  uint32_t iCmdOverflow;
};

#define MAX_UNIT_BYTES 50

struct EthPio
{
  struct arpcom         iArpcom;
  struct EthPioStats    iStats;
  struct ifnet*         iIfp;

  char                  iUnitName[MAX_UNIT_BYTES];

  rtems_event_set       iIoEvent;

  int                   iIsPresent;
  uint32_t              txSeq;
  uint32_t              rxSeq;
  int32_t               cmdFifoDepth;
  int32_t               txFifoDepth;

  bool                  pendingSend;
};

  } // ethPio

} // driver


#endif /* _DRIVER_ETHPIO_ETHPIO_HH_*/
