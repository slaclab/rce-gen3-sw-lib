/* $Id: xpci.h 1948 2013-08-15 18:48:49Z claus $ */
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
* @file xpci.h
*
* This file contains the software API definition of the Xilinx PCI bridge
* (XPci) component. This component bridges between local bus IPIF and the
* Xilinx LogiCORE PCI64 Interface v3.0 core. It provides full bridge
* functionality between the local bus a 32 bit V2.2 compliant PCI
* bus.
*
* <b>Features</b>
*
* This driver allows the user to access the device's registers to perform PCI
* configuration read and write access, error detection and processing, and
* interrupt management.
*
* The Xilinx PCI bridge controller is a soft IP core designed for
* Xilinx FPGAs and contains the following features:
*   - Supports 32 bit OPB local bus
*   - PCI V2.2 Complient
*   - Robust error reporting and diagnostics
*   - DMA capable
*
* <b>Interrupt Management</b>
*
* The XPci component driver provides interrupt management functions.
* Implementation of callback handlers is left to the user. Refer to the provided
* PCI code fragments in the examples directory.
*
* The PCI bridge IP core uses the IPIF to manage interrupts from devices within
* it. Devices in this core include the PCI bridge itself and an optional DMA
* engine. To manage interrupts from these devices, a three layer approach is
* utilized and is modeled on the IPIF.
*
* Device specific interrupt control is at the lowest layer. This is where
* individual sources are managed. For example, PCI Master Abort or DMA complete
* interrupts are enabled/disabled/cleared here. The XPci function API that
* manages this layer is identified as XPci_InterruptPci<operation>(). DMA
* interrupts at this layer are managed by the XDma_Channel software component.
*
* The middle layer is utilized to manage interrupts at a device level. For
* example, enabling PCI interrupts at this layer allows any PCI device specific
* interrupt enabled at the lowest layer to be passed up to the highest layer.
* The XPCI function API that manages this layer is identified as
* XPci_Interrupt<operation>().
*
* The middle layer serves little purpose when there is no DMA engine and can
* largely be ignored. During initialization, use XPci_InterruptEnable(...,
* XPCI_IPIF_INT_PCI) to allow all PCI interrupts enabled at the lowest layer
* to pass through. After this operation, the middle layer can be forgotten.
*
* The highest layer is simply a global interrupt enable/disable switch that
* allows all or none of the enabled interrupts to be passed on to an interrupt
* controller. The XPci function API that manages this level is identified as
* XPci_InterruptGlobal<operation>().
*
* <b>DMA</b>
*
* The PCI bridge can include a DMA engine in HW. The XPci software driver can
* be used to query which type of DMA engine has been implemented and manage
* interrupts. The application is required to initialize an XDma_Channel
* component driver and provide an interrupt service routine to service DMA
* exceptions. Example DMA management code is provided in the examples directory.
*
* @note
* This driver is intended to be used to bridge across multiple types of
* buses (PLB or OPB). While the register set will remain the same for all buses,
* their bit definitions may change slightly from bus to bus. The differences
* that arise out of this are clearly documented in this file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ------------------------------------------------------
* 1.00a rmm  04/15/03 First release
* 1.00b rpm  11/05/03 Changed to use XIo_InSwap32 and XIo_OutSwap32 swap
*                     routines. This corresponds to the opb_pci core change
*                     that preserves byte address through the bridge.
* 1.00b rpm  05/11/04 Added IdselAddrBit to the configuration data structure.
*                     This allows _Initialize() to use correct PCI device
*                     ID when setting the PCI bar registers.
* 1.00c jhl  06/24/04 Added new IPIFBAR2PCIBAR and Host Bridge Device Number
*                     registers with functions to access them.
* 1.12a ecm  03/24/07 Removed IPIF driver, new coding standard updates.
* </pre>
*
*****************************************************************************/
#ifndef XPCI_H          /* prevent circular inclusions */
#define XPCI_H          /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/
#include "xbasic_types.h"
#include "xstatus.h"
#include "xpci_l.h"

/************************** Constant Definitions ****************************/


/*
 * XPCI_CLEAR_ALL_ERRORS is for use with XPci_ErrorClear()
 */
#define XPCI_CLEAR_ALL_ERRORS ((XPciError*)3)


/**************************** Type Definitions ******************************/

/*
 * This typedef contains configuration information for the device.
 */
typedef struct {
    u16 DeviceId;       /**< Unique ID of device */
    u32 RegBaseAddr;    /**< Register base address */
    u32 DmaBaseAddr;    /**< DMA register base address */
    u32 CfgBar0;        /**< PCI BAR 0 */
    u32 CfgBar1;        /**< PCI BAR 1 */
    u32 CfgBar2;        /**< PCI BAR 2 */
    u8 CfgPrefetch0;    /**< prefetchable setting for PCI BAR 0 */
    u8 CfgPrefetch1;    /**< prefetchable setting for PCI BAR 1 */
    u8 CfgPrefetch2;    /**< prefetchable setting for PCI BAR 2 */
    u8 CfgSpace0;       /**< IO or memory space for PCI BAR 0 */
    u8 CfgSpace1;       /**< IO or memory space for PCI BAR 1 */
    u8 CfgSpace2;       /**< IO or memory space for PCI BAR 2 */
    u8 DmaType;     /**< DMA type  */
    u8 IdselAddrBit;    /**< IDSEL Address Bit (ranges 16 to 31) */
    u8 IpifBarNum;      /**< The number of IPIF BARs in h/w */
    u8 IncludeBarOffsetReg;
                /**< BAR Offset registers are in the h/w */
    u8 IncludeDevNumReg;/**< Host bridge device num register in h/w */
} XPci_Config;

/**
 * The XPci driver instance data. The user is required to allocate a
 * variable of this type for every PCI device in the system that will be
 * using this API. A pointer to a variable of this type is passed to the driver
 * API functions defined here.
 */
typedef struct {
    u32 RegBaseAddr;
               /**< Base address of registers */
    u32 DmaRegBaseAddr;
               /**< Base address of DMA (if included) */
    u32 IsReady;   /**< Device is initialized and ready */
    u8 DmaType;    /**< Type of DMA (if enabled), see XPCI_DMA_TYPE
                                constants in xpci_l.h */
    XPci_Config *ConfigPtr;
               /**< Pointer to the configuration information */
} XPci;

/**
 * XPciError is used to retrieve a snapshot of the bridge's error state.
 * Most of the attributes of this structure are copies of various bridge
 * registers. See XPci_ErrorGet() and XPci_ErrorClear().
 */
typedef struct {
    int IsError;       /**< Global error indicator */
    u32 LocalBusReason; /**< Local bus master address definition */
    u32 PciReason;      /**< PCI address definition */
    u32 PciSerrReason;  /**< PCI System error definiton */
    u32 LocalBusReadAddr;
                /**< Local bus master read error address */
    u32 LocalBusWriteAddr;
                /**< Local bus master write error address */
    u32 PciReadAddr;    /**< PCI read error address */
    u32 PciWriteAddr;   /**< PCI write error address */
    u32 PciSerrReadAddr;/**< PCI initiater read SERR address */
    u32 PciSerrWriteAddr;
                /**< PCI initiater write SERR address */
} XPciError;

/***************** Macros (Inline Functions) Definitions ********************/


/************************** Function Prototypes *****************************/

/*
 * Initialization & raw PCI configuration functions.
 * This API is implemented in xpci.c
 */
int XPci_Initialize(XPci * InstancePtr, u16 DeviceId, int BusNo, int SubBusNo);
void XPci_Reset(XPci * InstancePtr);
u32 XPci_ConfigPack(unsigned Bus, unsigned Device, unsigned Function);
u32 XPci_ConfigIn(XPci * InstancePtr, u32 ConfigAddress, u8 Offset);
void XPci_ConfigOut(XPci * InstancePtr, u32 ConfigAddress, u8 Offset,
            u32 ConfigData);
void XPci_ErrorGet(XPci * InstancePtr, XPciError * ErrorDataPtr);
void XPci_ErrorClear(XPci * InstancePtr, XPciError * ErrorDataPtr);
void XPci_InhibitAfterError(XPci * InstancePtr, u32 Mask);
void XPci_SetBusNumber(XPci * InstancePtr, int BusNo, int SubBusNo);
void XPci_GetBusNumber(XPci * InstancePtr, int *BusNoPtr, int *SubBusNoPtr);
void XPci_GetDmaImplementation(XPci * InstancePtr, u32 *BaseAddr, u8 *DmaType);
u32 XPci_GetIpifBar2PciBar(XPci * InstancePtr, u32 BarNumber);
void XPci_SetIpifBar2PciBar(XPci * InstancePtr, u32 BarNumber, u32 ValueToSet);
u32 XPci_GetHostBridgeDevice(XPci * InstancePtr);
void XPci_SetHostBridgeDevice(XPci * InstancePtr, u32 DeviceNumber);

XPci_Config *XPci_LookupConfig(u16 DeviceId);

/*
 * PCI bus configuration functions.
 * This API is implemented in xpci_config.c
 */
int XPci_ConfigIn8(XPci * InstancePtr, unsigned Bus, unsigned Device,
           unsigned Func, unsigned Offset, u8 *Data);
int XPci_ConfigIn16(XPci * InstancePtr, unsigned Bus, unsigned Device,
            unsigned Func, unsigned Offset, u16 *Data);
int XPci_ConfigIn32(XPci * InstancePtr, unsigned Bus, unsigned Device,
            unsigned Func, unsigned Offset, u32 *Data);

int XPci_ConfigOut8(XPci * InstancePtr, unsigned Bus, unsigned Device,
            unsigned Func, unsigned Offset, u8 Data);
int XPci_ConfigOut16(XPci * InstancePtr, unsigned Bus, unsigned Device,
             unsigned Func, unsigned Offset, u16 Data);
int XPci_ConfigOut32(XPci * InstancePtr, unsigned Bus, unsigned Device,
             unsigned Func, unsigned Offset, u32 Data);

/*
 * Interrupt processing and special cycle functions
 * This API is implemented in xpci_intr.c
 */
void XPci_InterruptGlobalEnable(XPci * InstancePtr);
void XPci_InterruptGlobalDisable(XPci * InstancePtr);

void XPci_InterruptEnable(XPci * InstancePtr, u32 Mask);
void XPci_InterruptDisable(XPci * InstancePtr, u32 Mask);
void XPci_InterruptClear(XPci * InstancePtr, u32 Mask);
u32 XPci_InterruptGetEnabled(XPci * InstancePtr);
u32 XPci_InterruptGetStatus(XPci * InstancePtr);
u32 XPci_InterruptGetPending(XPci * InstancePtr);
u32 XPci_InterruptGetHighestPending(XPci * InstancePtr);

void XPci_InterruptPciEnable(XPci * InstancePtr, u32 Mask);
void XPci_InterruptPciDisable(XPci * InstancePtr, u32 Mask);
void XPci_InterruptPciClear(XPci * InstancePtr, u32 Mask);
u32 XPci_InterruptPciGetEnabled(XPci * InstancePtr);
u32 XPci_InterruptPciGetStatus(XPci * InstancePtr);

void XPci_AckSend(XPci * InstancePtr, u32 Vector);
u32 XPci_AckRead(XPci * InstancePtr);
void XPci_SpecialCycle(XPci * InstancePtr, u32 Data);

/*
 * V3 core access functions
 * This API implementedin xpci_v3.c
 */
u32 XPci_V3StatusCommandGet(XPci * InstancePtr);
u32 XPci_V3TransactionStatusGet(XPci * InstancePtr);
void XPci_V3TransactionStatusClear(XPci * InstancePtr, u32 Data);

/*
 * Selftest
 * This API is implemented in xpci_selftest.c
 */
int XPci_SelfTest(XPci * InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
