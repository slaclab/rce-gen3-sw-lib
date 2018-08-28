/* $Id: xpci_v3.c 1948 2013-08-15 18:48:49Z claus $ */
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
* @file xpci_v3.c
*
* Implements V3 core processing functions for the XPci component.
* See xpci.h for more information about the component.
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
******************************************************************************/

/****************************** Include Files ********************************/
#include "xpci.h"

/*************************** Constant Definitions ****************************/

/***************************** Type Definitions ******************************/

/****************** Macros (Inline Functions) Definitions ********************/

/*************************** Variable Definitions ****************************/

/*************************** Function Prototypes *****************************/

/****************************************************************************/
/**
* Read the contents of the V3 bridge's status & command register. This same
* register can be retrieved by a PCI configuration access. The register can
* be written only with a PCI configuration access.
*
* @param InstancePtr is the PCI component to operate on.
*
* @return Contents of the V3 bridge's status and command register
*
* @note
*
* None
*
*****************************************************************************/
u32 XPci_V3StatusCommandGet(XPci * InstancePtr)
{
	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* this register is not supported in HW yet */
#if 0
	return (XPci_mReadReg(InstancePtr->RegBaseAddr, XPCI_STATCMD_OFFSET));
#endif
	return (0);
}


/****************************************************************************/
/**
* Read the contents of the V3 bridge's transaction status register. The
* contents of this register can be decoded using XPCI_STATV3_* constants
* defined in xpci_l.h.
*
* @param InstancePtr is the PCI component to operate on.
*
* @return Contents of the V3 bridge's transaction status register.
*
* @note
*
* None
*
*****************************************************************************/
u32 XPci_V3TransactionStatusGet(XPci * InstancePtr)
{
	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* this register is not supported in HW yet */
#if 0
	return (XPci_mReadReg(InstancePtr->RegBaseAddr, XPCI_STATV3_OFFSET));
#endif
	return (0);
}


/****************************************************************************/
/**
* Clear status bits in the V3 bridge's transaction status register. The
* contents of this register can be decoded using XPCI_STATV3_* constants
* defined in xpci_l.h.
*
* @param InstancePtr is the PCI component to operate on.
* @param Data is the contents to write to the register. Or XPCI_STATV3_*
*        constants for those bits to be cleared. Bits in the register that are
*        read-only are not affected.
*
* @note
*
* None
*
*****************************************************************************/
void XPci_V3TransactionStatusClear(XPci * InstancePtr, u32 Data)
{
	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
	XPci_mWriteReg(InstancePtr->RegBaseAddr, XPCI_STATV3_OFFSET, Data);
}
