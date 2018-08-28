/* $Id: xpci_l.h 1948 2013-08-15 18:48:49Z claus $ */
/******************************************************************************
*
*       XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
*       AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
*       SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,
*       OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
*       APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION
*       THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
*       AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
*       FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
*       WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
*       IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
*       REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
*       INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*       FOR A PARTICULAR PURPOSE.
*
*       (c) Copyright 2007 Xilinx Inc.
*       All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xpci_l.h
*
* This header file contains identifiers and low-level driver functions (or
* macros) that can be used to access the device.  High-level driver functions
* are defined and more PCI documentation is in xpci.h.
*
* PCI configuration read/write macro functions can be changed so that data is
* swapped before being written to the configuration addess/data registers.
* As delivered in this file, these macros do the swap. Change the definitions
* of XIo_InPci() and XIo_OutPci() in this file to suit system needs.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- --------------------------------------------------------
* 1.12a ecm  03/24/07 First release - was xpci_l.h
* 1.12a ecm  05/07/07 added two new error bits in ISR
* </pre>
*
******************************************************************************/

#ifndef XPCI_L_H       /* prevent circular inclusions */
#define XPCI_L_H       /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#ifndef _ASMLANGUAGE

#include "xbasic_types.h"
#include "xio.h"

#endif /* _ASMLANGUAGE */

/************************** Constant Definitions *****************************/

/** @name Registers
 *
 * Register offsets for this device. Note that the following IPIF registers
 * are implemented. Macros are defined to specifically access these registers
 * without knowing which version of IPIF being used. Some of the registers
 * are configurable at hardware build time such that may or may not exist
 * in the hardware.
 * @{
 */
#define XPCI_PREOVRD_OFFSET     0x100 /**< Prefetch override */
#define XPCI_IAR_OFFSET         0x104 /**< PCI interrupt ack */
#define XPCI_SC_DATA_OFFSET     0x108 /**< Special cycle data */
#define XPCI_CAR_OFFSET         0x10C /**< Config addr reg (port) */
#define XPCI_CDR_OFFSET         0x110 /**< Config command data  */
#define XPCI_BUSNO_OFFSET       0x114 /**< bus/subordinate bus numbers */
#define XPCI_STATCMD_OFFSET     0x118 /**< PCI config status/command */
#define XPCI_STATV3_OFFSET      0x11C /**< V3 core transaction status */
#define XPCI_INHIBIT_OFFSET     0x120 /**< Inhibit transfers on errors */
#define XPCI_LMADDR_OFFSET      0x124 /**< Local bus master address definition */
#define XPCI_LMA_R_OFFSET       0x128 /**< Local bus master read error address */
#define XPCI_LMA_W_OFFSET      0x12C /**< Local bus master write error address */
#define XPCI_SERR_R_OFFSET     0x130 /**< PCI initiater read SERR address */
#define XPCI_SERR_W_OFFSET     0x134 /**< PCI initiater write SERR address */
#define XPCI_PIADDR_OFFSET     0x138 /**< PCI address definition */
#define XPCI_PIA_R_OFFSET      0x13C /**< PCI read error address */
#define XPCI_PIA_W_OFFSET      0x140 /**< PCI write error address */
#define XPCI_IPIF2PCI_0_OFFSET 0x180 /**< IPIFBAR to PCIBAR translation 0 */
#define XPCI_IPIF2PCI_1_OFFSET 0x184 /**< IPIFBAR to PCIBAR translation 1 */
#define XPCI_IPIF2PCI_2_OFFSET 0x188 /**< IPIFBAR to PCIBAR translation 2 */
#define XPCI_IPIF2PCI_3_OFFSET 0x18C /**< IPIFBAR to PCIBAR translation 3 */
#define XPCI_IPIF2PCI_4_OFFSET 0x190 /**< IPIFBAR to PCIBAR translation 4 */
#define XPCI_IPIF2PCI_5_OFFSET 0x194 /**< IPIFBAR to PCIBAR translation 5 */
#define XPCI_HBDN_OFFSET       0x198 /**< Host bridge device number */
/*@}*/


/** @name Interrupt Status and Enable Register bitmaps and masks
 *
 * Bit definitions for the interrupt status register and interrupt enable
 * registers.
 * @{
 */
#define XPCI_IR_MASK        0x00001FFF /**< Mask of all bits */
#define XPCI_IR_LM_SERR_R   0x00000001 /**< Local bus master read SERR */
#define XPCI_IR_LM_PERR_R   0x00000002 /**< Local bus master read PERR */
#define XPCI_IR_LM_TA_R     0x00000004 /**< Local bus master read target abort */
#define XPCI_IR_LM_SERR_W   0x00000008 /**< Local bus master write SERR */
#define XPCI_IR_LM_PERR_W   0x00000010 /**< Local bus master write PERR */
#define XPCI_IR_LM_TA_W     0x00000020 /**< Local bus master write target abort */
#define XPCI_IR_LM_MA_W     0x00000040 /**< Local bus master abort write */
#define XPCI_IR_LM_BR_W     0x00000080 /**< Local bus master burst write retry */
#define XPCI_IR_LM_BRD_W    0x00000100 /**< Local bus master burst write retry
                                            disconnect */
#define XPCI_IR_LM_BRT_W    0x00000200 /**< Local bus master burst write retry
                                            timeout */
#define XPCI_IR_LM_PFT_R    0x00000400 /**< Local bus master prefetch timeout */
#define XPCI_IR_PI_SERR_R   0x00000800 /**< PCI initiator read SERR */
#define XPCI_IR_PI_SERR_W   0x00001000 /**< PCI initiator write SERR */
#define XPCI_IR_BAR_ORUN_R  0x00008000 /**< PLB Slave BAR Read Overrun */
#define XPCI_IR_BAR_ORUN_W  0x00010000 /**< PLB Slave BAR Write Overrun */

/*@}*/

/** @name Inhibit transfers on errors register bitmaps and masks.
 *
 * These bits contol whether subsequent PCI transactions are allowed after
 * an error occurs. Bits set to 1 inhibit further transactions, while bits
 * set to 0 allow further transactions after an error.
 * @{
 */
#define XPCI_INHIBIT_MASK        0x0000000F /**< Mask for all bits defined below */
#define XPCI_INHIBIT_LOCAL_BUS_R 0x00000001 /**< Local bus master reads */
#define XPCI_INHIBIT_LOCAL_BUS_W 0x00000002 /**< Local bus mater writes */
#define XPCI_INHIBIT_PCI_R       0x00000004 /**< PCI initiator reads */
#define XPCI_INHIBIT_PCI_W       0x00000008 /**< PCI initiator writes */
/*@}*/

/** @name PCI configuration status & command register bitmaps and masks.
 *
 * Bit definitions for the PCI configuration status & command register. The
 * definition of this register is standard for PCI devices.
 * @{
 */
#define XPCI_STATCMD_IO_EN        0x00000001 /**< I/O access enable */
#define XPCI_STATCMD_MEM_EN       0x00000002 /**< Memory access enable */
#define XPCI_STATCMD_BUSM_EN      0x00000004 /**< Bus master enable */
#define XPCI_STATCMD_SPECIALCYC   0x00000008 /**< Special cycles */
#define XPCI_STATCMD_MEMWR_INV_EN 0x00000010 /**< Memory write & invalidate */
#define XPCI_STATCMD_VGA_SNOOP_EN 0x00000020 /**< VGA palette snoop enable */
#define XPCI_STATCMD_PARITY       0x00000040 /**< Report parity errors */
#define XPCI_STATCMD_STEPPING     0x00000080 /**< Stepping control */
#define XPCI_STATCMD_SERR_EN      0x00000100 /**< SERR report enable */
#define XPCI_STATCMD_BACK_EN      0x00000200 /**< Fast back-to-back enable */
#define XPCI_STATCMD_INT_DISABLE  0x00000400 /**< Interrupt disable (PCI v2.3) */
#define XPCI_STATCMD_INT_STATUS   0x00100000 /**< Interrupt status (PCI v2.3) */
#define XPCI_STATCMD_66MHZ_CAP    0x00200000 /**< 66MHz capable */
#define XPCI_STATCMD_MPERR        0x01000000 /**< Master data PERR detected */
#define XPCI_STATCMD_DEVSEL_MSK   0x06000000 /**< Device select timing mask */
#define XPCI_STATCMD_DEVSEL_FAST  0x04000000 /**< Device select timing fast */
#define XPCI_STATCMD_DEVSEL_MED   0x02000000 /**< Device select timing medium */
#define XPCI_STATCMD_TGTABRT_SIG  0x08000000 /**< Signaled target abort */
#define XPCI_STATCMD_TGTABRT_RCV  0x10000000 /**< Received target abort */
#define XPCI_STATCMD_MSTABRT_RCV  0x20000000 /**< Received master abort */
#define XPCI_STATCMD_SERR_SIG     0x40000000 /**< Signaled SERR */
#define XPCI_STATCMD_PERR_DET     0x80000000 /**< Detected PERR */
#define XPCI_STATCMD_ERR_MASK     (XPCI_STATCMD_PERR_DET |    \
                                   XPCI_STATCMD_SERR_SIG |    \
                                   XPCI_STATCMD_MSTABRT_RCV | \
                                   XPCI_STATCMD_TGTABRT_RCV | \
                                   XPCI_STATCMD_TGTABRT_SIG | \
                                   XPCI_STATCMD_MPERR) /**< Error bits or'd together */
/*@}*/

/** @name V3 core transaction status register bitmaps and masks.
 *
 * Bit definitions for the V3 core transaction status register. This register
 * consists of status information on V3 core transactions.
 * @{
 */
#define XPCI_STATV3_MASK         0x000007FF /**< Mask of all bits */
#define XPCI_STATV3_DATA_XFER    0x00000001 /**< Data transfer. Read only */
#define XPCI_STATV3_TRANS_END    0x00000002 /**< Transaction end. Read only */
#define XPCI_STATV3_NORM_TERM    0x00000004 /**< Normal termination. Read only */
#define XPCI_STATV3_TGT_TERM     0x00000008 /**< Target termination. Read only */
#define XPCI_STATV3_DISC_WODATA  0x00000010 /**< Disconnect without data. Read only */
#define XPCI_STATV3_DISC_WDATA   0x00000020 /**< Disconnect with data. Read only */
#define XPCI_STATV3_TGT_ABRT     0x00000040 /**< Target abort. Read only */
#define XPCI_STATV3_MASTER_ABRT  0x00000080 /**< Master abort. Read only */
#define XPCI_STATV3_PCI_RETRY_R  0x00000100 /**< PCI retry on read. Read/write */
#define XPCI_STATV3_PCI_RETRY_W  0x00000200 /**< PCI retry on write. Read/write */
#define XPCI_STATV3_WRITE_BUSY   0x00000400 /**< Write busy. Read only */
/*@}*/


/** @name Bus number and subordinate bus number register bitmaps and masks
 *
 * @{
 */
#define XPCI_BUSNO_BUS_MASK      0x00FF0000 /**< Mask for bus number */
#define XPCI_BUSNO_SUBBUS_MASK   0x000000FF /**< Mask for subordinate bus no */
/*@}*/


/** @name Local bus master address register bitmaps and masks
 *
 * Bit definitions for the local bus master address definition register.
 * This register defines the meaning of the address stored in the local bus
 * master read (XPCI_LMA_R_OFFSET) and master write error (XPCI_LMA_W_OFFSET)
 * registers.
 * @{
 */
#define XPCI_LMADDR_MASK         0x000007FF /**< Mask of all bits */
#define XPCI_LMADDR_SERR_R       0x00000001 /**< Master read SERR */
#define XPCI_LMADDR_PERR_R       0x00000002 /**< Master read PERR */
#define XPCI_LMADDR_TA_R         0x00000004 /**< Master read target abort */
#define XPCI_LMADDR_SERR_W       0x00000008 /**< Master write SERR */
#define XPCI_LMADDR_PERR_W       0x00000010 /**< Master write PERR */
#define XPCI_LMADDR_TA_W         0x00000020 /**< Master write target abort */
#define XPCI_LMADDR_MA_W         0x00000040 /**< Master abort write */
#define XPCI_LMADDR_BR_W         0x00000080 /**< Master burst write retry */
#define XPCI_LMADDR_BRD_W        0x00000100 /**< Master burst write retry disconnect */
#define XPCI_LMADDR_BRT_W        0x00000200 /**< Master burst write retry timeout */
#define XPCI_LMADDR_PFT_R        0x00000400 /**< Master burst prefetch time out */
/*@}*/

/** @name PCI error address definition bitmaps and masks
 *
 * Bit definitions for the PCI address definition register. This register
 * defines the meaning of the address stored in the PCI read error address
 * (XPCI_PIA_R_OFFSET) and PCI write error address (XPCI_PIA_W_OFFSET) registers.
 * @{
 */
#define XPCI_PIADDR_MASK         0x0000001F /**< Mask of all bits */
#define XPCI_PIADDR_ERRACK_R     0x00000001 /**< PCI initiator read ErrAck */
#define XPCI_PIADDR_ERRACK_W     0x00000002 /**< PCI initiator write ErrAck */
#define XPCI_PIADDR_RETRY_W      0x00000004 /**< PCI initiator write retries */
#define XPCI_PIADDR_TIMEOUT_W    0x00000008 /**< PCI initiator write timeout */
#define XPCI_PIADDR_RANGE_W      0x00000010 /**< PCI initiator write range */
/*@}*/


/** @name PCI configuration header offsets
 *
 * Defines the offsets in the standard PCI configuration header
 * @{
 */
#define XPCI_HDR_VENDOR          0x00 /**< Vendor ID */
#define XPCI_HDR_DEVICE          0x02 /**< Device ID */
#define XPCI_HDR_COMMAND         0x04 /**< Command register */
#define XPCI_HDR_STATUS          0x06 /**< Status register */
#define XPCI_HDR_REVID           0x08 /**< Revision ID */
#define XPCI_HDR_CLASSCODE       0x09 /**< Class code */
#define XPCI_HDR_CACHE_LINE_SZ   0x0C /**< Cache line size */
#define XPCI_HDR_LATENCY         0x0D /**< Latency timer */
#define XPCI_HDR_TYPE            0x0E /**< Header type */
#define XPCI_HDR_BIST            0x0F /**< Built in self test */
#define XPCI_HDR_BAR0            0x10 /**< Base address 0 */
#define XPCI_HDR_BAR1            0x14 /**< Base address 1 */
#define XPCI_HDR_BAR2            0x18 /**< Base address 2 */
#define XPCI_HDR_BAR3            0x1C /**< Base address 3 */
#define XPCI_HDR_BAR4            0x20 /**< Base address 4 */
#define XPCI_HDR_BAR5            0x24 /**< Base address 5 */
#define XPCI_HDR_CARDBUS_PTR     0x28 /**< Cardbus CIS pointer */
#define XPCI_HDR_SUB_VENDOR      0x2C /**< Subsystem Vendor ID */
#define XPCI_HDR_SUB_DEVICE      0x2E /**< Subsystem ID */
#define XPCI_HDR_ROM_BASE        0x30 /**< Expansion ROM base address */
#define XPCI_HDR_CAP_PTR         0x34 /**< Capabilities pointer */
#define XPCI_HDR_INT_LINE        0x3C /**< Interrupt line */
#define XPCI_HDR_INT_PIN         0x3D /**< Interrupt pin */
#define XPCI_HDR_MIN_GNT         0x3E /**< Timeslice request */
#define XPCI_HDR_MAX_LAT         0x3F /**< Priority level request */
/*@}*/


/** @name PCI BAR register definitions
 *
 * Defines the masks and bits for the PCI XPAR_HDR_BARn registers
 * The bridge supports the first three BARs in the PCI configuration header
 * @{
 */
#define XPCI_HDR_NUM_BAR            6          /**< Number of BARs in the PCI header */
#define XPCI_HDR_BAR_ADDR_MASK      0xFFFFFFF0 /**< Base address mask */
#define XPCI_HDR_BAR_PREFETCH_YES   0x00000008 /**< Range is prefetchable */
#define XPCI_HDR_BAR_PREFETCH_NO    0x00000000 /**< Range is not prefetchable */
#define XPCI_HDR_BAR_TYPE_MASK      0x00000006 /**< Memory type mask */
#define XPCI_HDR_BAR_TYPE_BELOW_4GB 0x00000000 /**< Locate anywhere below 4GB */
#define XPCI_HDR_BAR_TYPE_BELOW_1MB 0x00000002 /**< Reserved in PCI 2.2 */
#define XPCI_HDR_BAR_TYPE_ABOVE_4GB 0x00000004 /**< Locate anywhere above 4GB */
#define XPCI_HDR_BAR_TYPE_RESERVED  0x00000006 /**< Reserved */
#define XPCI_HDR_BAR_SPACE_IO       0x00000001 /**< IO space indicator */
#define XPCI_HDR_BAR_SPACE_MEMORY   0x00000000 /**< Memory space indicator */
/*@}*/


/** @name DMA type constants
 *
 * Defines the types of DMA engines
 * @{
 */
#define XPCI_DMA_TYPE_NONE          9 /**< No DMA */
#define XPCI_DMA_TYPE_SIMPLE        0 /**< Simple DMA */
#define XPCI_DMA_TYPE_SG            1 /**< Scatter-gather DMA */
/*@}*/


/**
 * <pre>
 * The following IPIF registers are implemented.
 *
 *   XPCI_IPIF_DISR_OFFSET       Device interrupt status
 *   XPCI_IPIF_DIPR_OFFSET       Device interrupt pending
 *   XPCI_IPIF_DIER_OFFSET       Device interrupt enable
 *   XPCI_IPIF_DIIR_OFFSET       Device interrupt ID
 *   XPCI_IPIF_DGIER_OFFSET      Device global interrupt enable
 *   XPCI_IPIF_IISR_OFFSET       Bridge Interrupt status
 *   XPCI_IPIF_IIER_OFFSET       Bridge Interrupt enable
 *   XPCI_IPIF_RESETR_OFFSET     Reset (1)
 *
 * (1) This version of the PCI core places the RESETR at a nonstandard location
 * of 0x80 instead of 0x40.
 * </pre>
 */

/* IPIF Specific Defines */

#define XPCI_IPIF_DISR_OFFSET     0UL  /**< device interrupt status register */
#define XPCI_IPIF_DIPR_OFFSET     4UL  /**< device interrupt pending register */
#define XPCI_IPIF_DIER_OFFSET     8UL  /**< device interrupt enable register */
#define XPCI_IPIF_DIIR_OFFSET     24UL /**< device interrupt ID register */
#define XPCI_IPIF_DGIER_OFFSET    28UL /**< device global interrupt enable register */
#define XPCI_IPIF_IISR_OFFSET     32UL /**< IP interrupt status register */
#define XPCI_IPIF_IIER_OFFSET     40UL /**< IP interrupt enable register */
#define XPCI_IPIF_RESETR_OFFSET   0x80UL /**< reset register */


/*
 * Default IPIF Device interrupt enable (XPCI_IPIF_IIER_OFFSET),
 * status registers (XPCI_IPIF_IISR_OFFSET), and pending register
 * (XPCI_IPIF_IIPR_OFFSET)
 */
#define XPCI_IPIF_INT_PCI           0x00000004  /* PCI core interrupt */
#define XPCI_IPIF_INT_DMA           0x00000008  /* DMA interrupt */

/* Define IPIF interrupt mask */
#define XPCI_IPIF_INT_MASK (XPCI_IPIF_INT_PCI | XPCI_IPIF_INT_DMA)

/* Define value of IPIF DIIR register when no interrupt is pending */
#define XPCI_IPIF_INT_NONE_PENDING  128  /**< no interrupts are pending */

#define XPCI_IPIF_GINTR_ENABLE_MASK      0x80000000UL
#define XPCI_IPIF_RESET_MASK             0xAUL


/*
 * These macros define how IO access to the CAR/CDR are made
 * Use XIo_In/Out32 normally or XIo_InSwap/OutSwap32 if these registers
 * present data in an endianess opposite processor's.
 */
#define XIo_InPci(InAddress)         XIo_InSwap32(InAddress)
#define XIo_OutPci(OutAddress, Data) XIo_OutSwap32(OutAddress, Data)

/* IPIF Specific Register Access Functions */

#define XPCI_IPIF_READ_DIER(RegBaseAddress) \
    XIo_In32((RegBaseAddress) + XPCI_IPIF_DIER_OFFSET)

#define XPCI_IPIF_WRITE_DIER(RegBaseAddress, Enable) \
    XIo_Out32((RegBaseAddress) + XPCI_IPIF_DIER_OFFSET, (Enable))

#define XPCI_IPIF_READ_DISR(RegBaseAddress) \
    XIo_In32((RegBaseAddress) + XPCI_IPIF_DISR_OFFSET)

#define XPCI_IPIF_WRITE_DISR(RegBaseAddress, Enable) \
    XIo_Out32((RegBaseAddress) + XPCI_IPIF_DISR_OFFSET, (Enable))

#define XPCI_IPIF_READ_DIIR(RegBaseAddress) \
    XIo_In32((RegBaseAddress) + XPCI_IPIF_DIIR_OFFSET)

#define XPCI_IPIF_READ_DIPR(RegBaseAddress) \
    XIo_In32((RegBaseAddress) + XPCI_IPIF_DIPR_OFFSET)

#define XPCI_IPIF_WRITE_DIPR(RegBaseAddress, Enable) \
    XIo_Out32((RegBaseAddress) + XPCI_IPIF_DIPR_OFFSET, (Enable))

#define XPCI_IPIF_WRITE_IIER(RegBaseAddress, Enable) \
    XIo_Out32((RegBaseAddress) + XPCI_IPIF_IIER_OFFSET, (Enable))

#define XPCI_IPIF_READ_IIER(RegBaseAddress) \
    XIo_In32((RegBaseAddress) + XPCI_IPIF_IIER_OFFSET)

#define XPCI_IPIF_READ_IISR(RegBaseAddress) \
    XIo_In32((RegBaseAddress) + XPCI_IPIF_IISR_OFFSET)

#define XPCI_IPIF_WRITE_IISR(RegBaseAddress, Status) \
    XIo_Out32((RegBaseAddress) + XPCI_IPIF_IISR_OFFSET, (Status))

/******************************************************************************/
/**
* IPIF Low level PCI reset function. Reset the V3 core.
*
* C-style signature:
*    void XPci_mReset(u32 BaseAddress)
*
* @param BaseAddress is the base address of the PCI component.
*
* @return None
*
* @note The IPIF RESETR register is located at (base + 0x80) instead of
* (base + 0x40) where the IPIF component driver expects it. This macro adjusts
* for this difference.
*******************************************************************************/
#define XPci_mReset(BaseAddress) \
      XIo_Out32(BaseAddress + XPCI_IPIF_RESETR_OFFSET, XPCI_IPIF_RESET_MASK)


/******************************************************************************/
/**
* Global interrupt enable. Must be called before any interupts enabled by
* XPci_mIntrEnable() or XPci_mIntrPciEnable() have any effect.
*
* C-style signature:
*    void XPci_mIntrGlobalEnable(u32 BaseAddress)
*
* @param BaseAddress is the base address of the PCI component.
*
* @return None
*
* @note None
*******************************************************************************/
#define XPci_mIntrGlobalEnable(BaseAddress) \
    XIo_Out32((BaseAddress) + XPCI_IPIF_DGIER_OFFSET, \
               XPCI_IPIF_GINTR_ENABLE_MASK)


/******************************************************************************/
/**
* Global interrupt disable. Disable all interrupts from this core. Any
* interrupts enabled by XPci_mIntrEnable() or XPci_mIntrPciEnable()
* are disabled, however their settings remain unchanged.
*
* C-style signature:
*    void XPci_mIntrGlobalDisable(u32 BaseAddress)
*
* @param BaseAddress is the base address of the PCI component.
*
* @return None
*
* @note None
*******************************************************************************/
#define XPci_mIntrGlobalDisable(BaseAddress) \
    XIo_Out32((BaseAddress) + XPCI_IPIF_DGIER_OFFSET, 0)


/******************************************************************************/
/**
* Enable interrupts in the device interrupt enable register (DIER)
*
* C-style signature:
*    void XPci_mIntrEnable(u32 BaseAddress, Mask)
*
* @param BaseAddress is the base address of the PCI component.
* @param Mask is the group of interrupts to enable. Use a logical OR of
*        constants in XPCI_IPIF_INT_MASK. Bits set to 1 are enabled,
*        bits set to 0 are not affected.
*
* @return None
*
* @note None
*******************************************************************************/
#define XPci_mIntrEnable(BaseAddress, Mask) \
{ \
   u32 Temp; \
   Temp = XPCI_IPIF_READ_DIER((BaseAddress)); \
   Temp |= (Mask); \
   XPCI_IPIF_WRITE_DIER((BaseAddress), (Temp)); \
}


/******************************************************************************/
/**
* Disable interrupts in the device interrupt enable register (DIER)
*
* C-style signature:
*    void XPci_mIntrDisable(u32 BaseAddress, Mask)
*
* @param BaseAddress is the base address of the PCI component.
* @param Mask is the group of interrupts to disable. Use a logical OR of
*        constants in XPCI_IPIF_INT_MASK. Bits set to 1 are disabled,
*        bits set to 0 are not affected.
*
* @return None
*
* @note None
*******************************************************************************/
#define XPci_mIntrDisable(BaseAddress, Mask) \
{ \
   u32 Temp; \
   Temp = XPCI_IPIF_READ_DIER((BaseAddress)); \
   Temp &= ~(Mask); \
   XPCI_IPIF_WRITE_DIER((BaseAddress), (Temp)); \
}


/******************************************************************************/
/**
* Clear pending interrupts in the device interrupt status register (DISR)
* This is a toggle on write register.
*
* C-style signature:
*    void XPci_mIntrClear(u32 BaseAddress, u32 Mask)
*
* @param BaseAddress is the base address of the PCI component.
* @param Mask is the group of interrupts to clear. Use a logical OR of
*        constants in XPCI_IPIF_INT_MASK. Bits set to 1 are cleared,
*        bits set to 0 are not affected.
*
* @return None
*
* @note None
*******************************************************************************/
#define XPci_mIntrClear(BaseAddress, Mask) \
   XPCI_IPIF_WRITE_DISR((BaseAddress), \
                         XPCI_IPIF_READ_DISR((BaseAddress)) & (Mask))


/******************************************************************************/
/**
* Read the contents of the device interrupt enable register (DIER)
*
* C-style signature:
*    u32 XPci_mIntrReadIER(u32 BaseAddress)
*
* @param BaseAddress is the base address of the PCI component.
*
* @return Value of the register.
*
* @note None
*******************************************************************************/
#define XPci_mIntrReadIER(BaseAddress) \
  XPCI_IPIF_READ_DIER((BaseAddress))


/******************************************************************************/
/**
* Read the contents of the device interrupt status register (DISR)
*
* C-style signature:
*    u32 XPci_mIntrReadISR(u32 BaseAddress)
*
* @param BaseAddress is the base address of the PCI component.
*
* @return Value of the register.
*
* @note None
*******************************************************************************/
#define XPci_mIntrReadISR(BaseAddress) \
  XPCI_IPIF_READ_DISR((BaseAddress))

/******************************************************************************/
/**
* Write to the device interrupt status register (DISR)
*
* C-style signature:
*    void XPci_mIntrWriteISR(u32 BaseAddress, u32 Mask)
*
* @param BaseAddress is the base address of the PCI component.
* @param Mask is the value to write to the register and is assumed to be
*        bits or'd together from the XPCI_IPIF_INT_MASK.
*
* @return None
*
* @note None
*******************************************************************************/
#define XPci_mIntrWriteISR(BaseAddress, Mask) \
   XPCI_IPIF_WRITE_DISR((BaseAddress), (Mask))


/******************************************************************************/
/**
* Read the contents of the device interrupt pending register (DIPR).
*
* C-style signature:
*    u32 XPci_mIntrReadIPR(u32 BaseAddress)
*
* @param BaseAddress is the base address of the PCI component.
*
* @return Value of the register.
*
* @note None
*******************************************************************************/
#define XPci_mIntrReadIPR(BaseAddress) \
  XPCI_IPIF_READ_DIPR((BaseAddress))


/******************************************************************************/
/**
* Read the contents of the device interrupt ID register (DIIR).
*
* C-style signature:
*    u32 XPci_mIntrReadID(u32 BaseAddress)
*
* @param BaseAddress is the base address of the PCI component.
*
* @return Value of the register.
*
* @note None
*******************************************************************************/
#define XPci_mIntrReadID(BaseAddress) \
  XPCI_IPIF_READ_DIIR((BaseAddress))


/******************************************************************************/
/**
* Enable PCI specific interrupt sources in the PCI interrupt enable register
* (IIER)
*
* C-style signature:
*    void XPci_mIntrPciEnable(u32 BaseAddress, u32 Mask)
*
* @param BaseAddress is the base address of the PCI component.
* @param Mask is the group of interrupts to enable. Bits set to 1 are enabled,
*        bits set to 0 are not affected. The mask is made up by OR'ing bits
*        from XPCI_IR_MASK.
*
* @return None
*
* @note None
*******************************************************************************/
#define XPci_mIntrPciEnable(BaseAddress, Mask) \
{ \
   u32 Temp; \
   Temp = XPCI_IPIF_READ_IIER((BaseAddress)); \
   Temp |= (Mask); \
   XPCI_IPIF_WRITE_IIER((BaseAddress), (Temp)); \
}


/******************************************************************************/
/**
* Disable PCI specific interrupt sources in the PCI interrupt enable register
* (IIER)
*
* C-style signature:
*    void XPci_mIntrPciDisable(u32 BaseAddress, u32 Mask)
*
* @param BaseAddress is the base address of the PCI component.
* @param Mask is the group of interrupts to disable. Bits set to 1 are disabled,
*        bits set to 0 are not affected. The mask is made up by OR'ing bits
*        from XPCI_IR_MASK.
*
* @return None
*
* @note None
*******************************************************************************/
#define XPci_mIntrPciDisable(BaseAddress, Mask) \
{ \
   u32 Temp; \
   Temp = XPCI_IPIF_READ_IIER((BaseAddress)); \
   Temp &= ~(Mask); \
   XPCI_IPIF_WRITE_IIER((BaseAddress), (Temp)); \
}


/******************************************************************************/
/**
* Clear PCI specific interrupts in the interrupt status register (IISR).
* This is a toggle on write register.
*
* C-style signature:
*    void XPci_mIntrPciClear(u32 BaseAddress, u32 Mask)
*
* @param BaseAddress is the base address of the PCI component.
* @param Mask is the group of interrupts to clear. Use a logical OR of
*        constants in XPCI_IR_MASK. Bits set to 1 are cleared,
*        bits set to 0 are not affected.
*
* @return None
*
* @note None
*******************************************************************************/
#define XPci_mIntrPciClear(BaseAddress, Mask) \
   XPCI_IPIF_WRITE_IISR((BaseAddress), \
                         XPCI_IPIF_READ_IISR((BaseAddress)) & (Mask))


/******************************************************************************/
/**
* Read the contents of the PCI specific interrupt enable register (IIER)
*
* C-style signature:
*    u32 XPci_mIntrPciReadIER(u32 BaseAddress)
*
* @param BaseAddress is the base address of the PCI component.
*
* @return Contents of the pending interrupt register. The mask is made up of
*         bits defined in XPCI_IR_MASK.
*
* @note None
*******************************************************************************/
#define XPci_mIntrPciReadIER(BaseAddress) \
   XPCI_IPIF_READ_IIER((BaseAddress))


/******************************************************************************/
/**
* Read the contents of the PCI specific interrupt status register (IISR)
*
* C-style signature:
*    u32 XPci_mIntrPciReadISR(u32 BaseAddress)
*
* @param BaseAddress is the base address of the PCI component.
*
* @return
* Contents of the pending interrupt register. The mask is made up of
* bits defined in XPCI_IR_MASK.
*
* @note None
*******************************************************************************/
#define XPci_mIntrPciReadISR(BaseAddress) \
   XPCI_IPIF_READ_IISR((BaseAddress))


/******************************************************************************/
/**
* Write to the PCI interrupt status register (IISR)
*
* C-style signature:
*    void XPci_mIntrPciWriteISR(u32 BaseAddress, u32 Mask)
*
* @param BaseAddress is the base address of the PCI component.
* @param Mask is the value to write to the register and is assumed to be
*        bits or'd together from the XPCI_IR_MASK.
*
* @return None
*
* @note None
*******************************************************************************/
#define XPci_mIntrPciWriteISR(BaseAddress, Mask) \
   XPCI_IPIF_WRITE_IISR((BaseAddress), (Mask))


/******************************************************************************/
/**
* Low level register read function.
*
* C-style signature:
*    u32 XPci_mReadReg(u32 BaseAddress, u32 RegOffset)
*
* @param BaseAddress is the base address of the PCI component.
* @param RegOffset is the register offset.
*
* @return Value of the register.
*
* @note None
*******************************************************************************/
#define XPci_mReadReg(BaseAddress, RegOffset) \
  XIo_In32((BaseAddress) + (RegOffset))


/******************************************************************************/
/**
* Low level register write function.
*
* C-style signature:
*    void XPci_mWriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
* @param BaseAddress is the base address of the PCI component.
* @param RegOffset is the register offset.
* @param Data is the data to write.
*
* @return None
*
* @note None
*******************************************************************************/
#define XPci_mWriteReg(BaseAddress, RegOffset, Data) \
  XIo_Out32((BaseAddress) + (RegOffset), (Data))


/******************************************************************************/
/**
* Low level PCI configuration read
*
* C-style signature:
*    void XPci_mConfigIn(u32 BaseAddress, u32 ConfigAddress,
*                        u32 ConfigData)
*
* @param BaseAddress is the base address of the PCI component.
* @param ConfigAddress is the PCI configuration space address in a packed
*        format.
* @param ConfigData is the data read from the ConfigAddress
*
* @return Data from configuration address
*
* @note None
*******************************************************************************/
#define XPci_mConfigIn(BaseAddress, ConfigAddress, ConfigData) \
  XIo_OutPci((BaseAddress) + XPCI_CAR_OFFSET, (ConfigAddress)); \
  (ConfigData) = XIo_InPci((BaseAddress) + XPCI_CDR_OFFSET)


/******************************************************************************/
/**
* Low level PCI configuration write
*
* C-style signature:
*    void XPci_mConfigOut(u32 BaseAddress, u32 ConfigAddress,
*                         u32 ConfigData)
*
* @param BaseAddress is the base address of the PCI component.
* @param ConfigAddress is the PCI configuration space address in a packed
*        format.
* @param ConfigData is the data to write at the ConfigAddress
*
* @return None
*
* @note None
*******************************************************************************/
#define XPci_mConfigOut(BaseAddress, ConfigAddress, ConfigData) \
  XIo_OutPci((BaseAddress) + XPCI_CAR_OFFSET, (ConfigAddress)); \
  XIo_OutPci((BaseAddress) + XPCI_CDR_OFFSET, (ConfigData))


/******************************************************************************/
/**
* Generate a PCI interrupt ackowledge bus cycle with the given vector.
*
* C-style signature:
*    void XPci_mAckSend(u32 BaseAddress, u32 Vector)
*
* @param BaseAddress is the base address of the PCI component.
* @param Vector is the interrupt vector to place on the PCI bus.
*
* @return None
*
* @note None
*******************************************************************************/
#define XPci_mAckSend(BaseAddress, Vector) \
   XPci_mWriteReg((BaseAddress), XPCI_IAR_OFFSET, (Vector))


/******************************************************************************/
/**
* Read the contents of the PCI interrupt acknowledge vector register.
*
* C-style signature:
*    u32 XPci_mAckRead(u32 BaseAddress)
*
* @param BaseAddress is the base address of the PCI component.
*
* @return System dependent interrupt vector.
*
* @note None
*******************************************************************************/
#define XPci_mAckRead(BaseAddress) \
   XPci_mReadReg((BaseAddress), XPCI_IAR_OFFSET)


/******************************************************************************/
/**
* Broadcasts a message to all listening PCI targets.
*
* C-style signature:
*    u32 XPci_mSpecialCycle(u32 BaseAddress, u32 Data)
*
* @param BaseAddress is the base address of the PCI component.
* @param Data is the data to broadcast.
*
* @return None
*
* @note None
*******************************************************************************/
#define XPci_mSpecialCycle(BaseAddress, Data) \
   XPci_mWriteReg((BaseAddress), XPCI_SC_DATA_OFFSET, (Data))


/******************************************************************************/
/**
* Convert local bus address to a PCI address
*
* C-style signature:
*    u32 XPci_mLocal2Pci(u32 LocalAddr,
*                            u32 LocalBaseAddr,
*                            u32 PciBaseAddr)
*
* @param LocalAddr is the local address to find the equivalent PCI address
*        for.
* @param LocalBaseAddr is the base address of the address range containing
*        the local address.
* @param PciBaseAddr is the base address of the PCI address range that the
*        local address range maps to.
*
* @return Address in PCI space
*
* @note IPIFBAR_n, IPIFHIGHADDR_n, and IPIFBAR2PCIBAR_n, defined in
* xparameters.h, are defined for each BAR.  To make a proper conversion,
* LocalAddr must fall within range of a IPIFBAR_n and IPIFHIGHADDR_n pair
* and PciBaseAddr must be the matching IPIFBAR2PCIBAR_n.
*
* Example: pciAddr = XPci_mLocal2Pci(XPAR_PCI_IPIFBAR_0,
*                                    XPAR_PCI_IPIFBAR_0,
*                                    XPAR_PCI_IPIFBAR2PCIBAR_0)
*
* finds the PCI equivalent address for the local address named by
* XPAR_PCI_IPIFBAR_0.
*******************************************************************************/
#define XPci_mLocal2Pci(LocalAddr, LocalBaseAddr, PciBaseAddr) \
    ((u32)(LocalAddr) + ((u32)(PciBaseAddr) - (u32)(LocalBaseAddr)))

/******************************************************************************/
/**
* Convert PCI address to a local bus address
*
* C-style signature:
*    u32 XPci_mPci2Local(u32 PciAddr, u32 PciBaseAddr,
*                            u32 LocalBaseAddr)
*
* @param PciAddr is the PCI address to find the equivalent local address
*        for.
* @param PciBaseAddr is the base address of the PCI address range containing
*        PciAddr.
* @param LocalBaseAddr is the base address of the local bus address range
*        which the PCI address maps to.
*
* @return Address in local space
*
* @note PCIBAR_n, PCIBAR_LEN_n, and PCIBAR2IPIFBAR_n, defined in xparameters.h,
* are defined for each BAR.  To make a proper conversion, PciAddr must
* fall within range of a PCIBAR_n and PCIBAR_LEN_n pair and LocalBaseAddr
* specified must be the matching XPAR_PCI_PCIBAR2IPIFBAR_n.
*
* Example: localAddr = XPci_mPci2Local(XPAR_PCI_PCIBAR_0,
*                                      XPAR_PCI_PCIBAR_0,
*                                      XPAR_PCI_PCIBAR2IPIFBAR_0)
*
* finds the local address that corresponds to XPAR_PCI_PCIBAR_0 on the PCI bus.
* Note that PCIBAR_LEN is expressed as a power of 2.
*******************************************************************************/
#define XPci_mPci2Local(PciAddr, PciBaseAddr, LocalBaseAddr) \
    ((u32)(PciAddr) + ((u32)(LocalBaseAddr) - (u32)(PciBaseAddr)))

#ifdef __cplusplus
}
#endif

#endif /* XPCI_L_H */
