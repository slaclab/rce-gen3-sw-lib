/* $Id: xpci_intr.c 1948 2013-08-15 18:48:49Z claus $ */
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
* @file xpci_intr.c
*
* Implements PCI interrupt processing functions for the XPci
* component. See xpci.h for more information about the component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a rmm  03/25/02 Original code
* 1.12a ecm  03/24/07 Removed IPIF driver, new coding standard updates.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xpci.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/


/****************************************************************************/
/**
* Enable the core's interrupt output signal. Interrupts enabled through
* XPci_InterruptEnable() and XPci_InterruptPciEnable() will not be passed
* through until the IPIF global enable bit is set by this function.
*
* @param InstancePtr is the PCI component to operate on.
*
* @note
*
* None
*
*****************************************************************************/
void XPci_InterruptGlobalEnable(XPci * InstancePtr)
{
	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
	XPci_mIntrGlobalEnable(InstancePtr->RegBaseAddr);
}


/****************************************************************************/
/**
* Disable the core's interrupt output signal. Interrupts enabled through
* XPci_InterruptEnable() and XPci_InterruptPciEnable() will no longer be
* passed through until the IPIF global enable bit is set by
* XPci_InterruptGlobalEnable().
*
* @param InstancePtr is the PCI component to operate on.
*
* @note
*
* None
*
*****************************************************************************/
void XPci_InterruptGlobalDisable(XPci * InstancePtr)
{
	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
	XPci_mIntrGlobalDisable(InstancePtr->RegBaseAddr);
}


/****************************************************************************/
/**
* Enable device interrupts. Device interrupts must be enabled by this function
* before component interrupts enabled by XPci_InterruptPciEnable() and/or the
* DMA driver have any effect.
*
* @param InstancePtr is the PCI component to operate on.
* @param Mask is the mask to enable. Bit positions of 1 are enabled. The mask
*        is formed by OR'ing bits from XPCI_IPIF_INT_MASK.
*
* @note
*
* None
*
*****************************************************************************/
void XPci_InterruptEnable(XPci * InstancePtr, u32 Mask)
{
	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
	XPci_mIntrEnable(InstancePtr->RegBaseAddr, Mask);
}


/****************************************************************************/
/**
* Disable device interrupts. Any component interrupts enabled through
* XPci_InterruptPciEnable() and/or the DMA driver will no longer have any
* effect. The component interrupt settings will be retained however.
*
* @param InstancePtr is the PCI component to operate on.
* @param Mask is the mask to disable. Bits set to 1 are disabled. The mask
*        is formed by OR'ing bits from XPCI_IPIF_INT_MASK
*
* @note
*
* None
*
*****************************************************************************/
void XPci_InterruptDisable(XPci * InstancePtr, u32 Mask)
{
	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
	XPci_mIntrDisable(InstancePtr->RegBaseAddr, Mask);
}


/****************************************************************************/
/**
* Clear device level pending interrupts with the provided mask.
*
* @param InstancePtr is the PCI component to operate on.
* @param Mask is the mask to clear pending interrupts for. Bit positions of 1
*        are cleared. This mask is formed by OR'ing bits from
*        XPCI_IPIF_INT_MASK
*
* @note
*
* None
*
*****************************************************************************/
void XPci_InterruptClear(XPci * InstancePtr, u32 Mask)
{
	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
	XPci_mIntrClear(InstancePtr->RegBaseAddr, Mask);
}

/****************************************************************************/
/**
* Returns the device level interrupt enable mask as set by
* XPci_InterruptEnable().
*
* @param InstancePtr is the PCI component to operate on.
*
* @return Mask of bits made from XPCI_IPIF_INT_MASK.
*
* @note
*
* None
*
*****************************************************************************/
u32 XPci_InterruptGetEnabled(XPci * InstancePtr)
{
	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
	return (XPci_mIntrReadIER(InstancePtr->RegBaseAddr));
}


/****************************************************************************/
/**
* Returns the status of device level interrupt signals. Any bit in the mask
* set to 1 indicates that the given component has asserted an interrupt
* condition.
*
* @param InstancePtr is the PCI component to operate on.
*
* @return Mask of bits made from XPCI_IPIF_INT_MASK.
*
* @note
*
* The interrupt status indicates the status of the device irregardless if
* the interrupts from the devices have been enabled or not through
* XPci_InterruptEnable().
*
*****************************************************************************/
u32 XPci_InterruptGetStatus(XPci * InstancePtr)
{
	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
	return (XPci_mIntrReadISR(InstancePtr->RegBaseAddr));
}


/****************************************************************************/
/**
* Returns the pending status of device level interrupt signals that have been
* enabled by XPci_InterruptEnable(). Any bit in the mask set to 1 indicates
* that an interrupt is pending from the given component
*
* @param InstancePtr is the PCI component to operate on.
*
* @return Mask of bits made from XPCI_IPIF_INT_MASK or zero if no interrupts
*         are pending.
*
* @note
*
* None
*
*****************************************************************************/
u32 XPci_InterruptGetPending(XPci * InstancePtr)
{
	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
	return (XPci_mIntrReadIPR(InstancePtr->RegBaseAddr));
}


/****************************************************************************/
/**
* Returns the highest priority pending device interrupt that has been
* enabled by XPci_InterruptEnable().
*
* @param InstancePtr is the PCI component to operate on.
*
* @return Mask is one set bit made from XPCI_IPIF_INT_MASK or zero if no
*         interrupts are pending.
*
* @note
*
* None
*
*****************************************************************************/
u32 XPci_InterruptGetHighestPending(XPci * InstancePtr)
{
	u32 Data;

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	Data = XPci_mIntrReadID(InstancePtr->RegBaseAddr);
	if (Data == XPCI_IPIF_INT_NONE_PENDING) {
		Data = 0;
	}
	return (Data);
}


/****************************************************************************/
/**
* Enable PCI bridge specific interrupts. Before this function has any effect
* in generating interrupts, the function XPci_InterruptEnable() must be
* invoked with the XPCI_IPIF_INT_PCI bit set.
*
* @param InstancePtr is the PCI component to operate on.
* @param Mask is the mask to enable. Bit positions of 1 are enabled. The mask
*        is formed by OR'ing bits from XPCI_IR_MASK.
*
* @note
*
* None
*
*****************************************************************************/
void XPci_InterruptPciEnable(XPci * InstancePtr, u32 Mask)
{
	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
	XPci_mIntrPciEnable(InstancePtr->RegBaseAddr, Mask);
}


/****************************************************************************/
/**
* Disable PCI bridge specific interrupts.
*
* @param InstancePtr is the PCI component to operate on.
* @param Mask is the mask to disable. Bits set to 1 are disabled. The mask
*        is formed by OR'ing bits from XPCI_IR_MASK
*
* @note
*
* None
*
*****************************************************************************/
void XPci_InterruptPciDisable(XPci * InstancePtr, u32 Mask)
{
	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
	XPci_mIntrPciDisable(InstancePtr->RegBaseAddr, Mask);
}


/****************************************************************************/
/**
* Clear PCI bridge specific interrupt status bits with the provided mask.
*
* @param InstancePtr is the PCI component to operate on.
* @param Mask is the mask to clear pending interrupts for. Bit positions of 1
*        are cleared. This mask is formed by OR'ing bits from
*        XPCI_IR_MASK
*
* @note
*
* None
*
*****************************************************************************/
void XPci_InterruptPciClear(XPci * InstancePtr, u32 Mask)
{
	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
	XPci_mIntrPciClear(InstancePtr->RegBaseAddr, Mask);
}


/****************************************************************************/
/**
* Get the PCI bridge specific interrupts enabled through
* XPci_InterruptPciEnable(). Bits set to 1 mean that interrupt source is
* enabled.
*
* @param InstancePtr is the PCI component to operate on.
*
* @return Mask of enabled bits made from XPCI_IR_MASK.
*
* @note
*
* None
*
*****************************************************************************/
u32 XPci_InterruptPciGetEnabled(XPci * InstancePtr)
{
	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
	return (XPci_mIntrPciReadIER(InstancePtr->RegBaseAddr));
}


/****************************************************************************/
/**
* Get the status of PCI bridge specific interrupts that have been asserted
* Bits set to 1 are in an asserted state. Bits may be set to 1 irregardless
* of whether they have been enabled or not though XPci_InterruptPciEnable().
* To get the pending interrupts, AND the results of this function with
* XPci_InterruptPciGetEnabled().
*
* @param InstancePtr is the PCI component to operate on.
*
* @return Mask of enabled bits made from XPCI_IR_MASK.
*
* @note
*
* None
*
*****************************************************************************/
u32 XPci_InterruptPciGetStatus(XPci * InstancePtr)
{
	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
	return (XPci_mIntrPciReadISR(InstancePtr->RegBaseAddr));
}


/****************************************************************************/
/**
* Generate a PCI interrupt acknowledge bus cycle with the given vector.
*
* @param InstancePtr is the PCI component to operate on.
* @param Vector is a system dependent interrupt vector to place on the bus.
*
* @note
*
* None
*
*****************************************************************************/
void XPci_AckSend(XPci * InstancePtr, u32 Vector)
{
	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
	XPci_mAckSend(InstancePtr->RegBaseAddr, Vector);
}


/****************************************************************************/
/**
* Read the contents of the PCI interrupt acknowledge vector register.
*
* @param InstancePtr is the PCI component to operate on.
*
* @return System dependent interrupt vector.
*
* @note
*
* None
*
*****************************************************************************/
u32 XPci_AckRead(XPci * InstancePtr)
{
	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
	return (XPci_mAckRead(InstancePtr->RegBaseAddr));
}


/****************************************************************************/
/**
* Broadcasts a message to all listening PCI targets.
*
* @param InstancePtr is the PCI component to operate on.
* @param Data is the data to broadcast.
*
* @note
*
* None
*
*****************************************************************************/
void XPci_SpecialCycle(XPci * InstancePtr, u32 Data)
{
	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
	XPci_mSpecialCycle(InstancePtr->RegBaseAddr, Data);
}
