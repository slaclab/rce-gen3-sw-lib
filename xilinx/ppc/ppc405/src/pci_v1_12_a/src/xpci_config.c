/* $Id: xpci_config.c 1948 2013-08-15 18:48:49Z claus $ */
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
* @file xpci_config.c
*
* Implements advanced PCI configuration functions for the XPci
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
******************************************************************************/

/****************************** Include Files ********************************/
#include "xpci.h"

/*************************** Constant Definitions ****************************/
#define INVALID_ADDRESS 0xFFFFFFFF	/* For use with FormatConfig(). The
					   constant must be an invalid config
					   address */

/***************************** Type Definitions ******************************/

/****************** Macros (Inline Functions) Definitions ********************/

/*************************** Variable Definitions ****************************/

/*************************** Function Prototypes *****************************/
static u32 FormatConfig(unsigned Bus, unsigned Device,
			unsigned Func, unsigned Offset);


/****************************************************************************/
/**
* Perform a 8 bit read transaction in PCI configuration space. Together,
* the Bus, Device, Func, & Offset form the address of the PCI target to
* access.
*
* @param InstancePtr is the PCI component to operate on.
* @param Bus is the target PCI Bus #.
* @param Device is the target device number.
* @param Func is the target device's function number.
* @param Offset is the target device's configuration space I/O offset to
*        address.
* @param Data is the data read from the target.
*
* @return
*
* - XST_SUCCESS Operation was successfull.
* - XST_PCI_INVALID_ADDRESS One of Bus, Device, Func, or Offset form an invalid
*   address.
*
* @note
*
* None
*
*****************************************************************************/
int XPci_ConfigIn8(XPci * InstancePtr, unsigned Bus, unsigned Device,
		   unsigned Func, unsigned Offset, u8 *Data)
{
	u32 ConfigAddress;
	u32 ConfigData;
	u32 DataPtr;

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/*
	 * Format the config address
	 */
	ConfigAddress = FormatConfig(Bus, Device, Func, Offset);
	if (ConfigAddress == INVALID_ADDRESS) {
		return (XST_PCI_INVALID_ADDRESS);
	}

	/*
	 * Perform the configuration read then return the correct byte
	 */
	XPci_mConfigIn(InstancePtr->RegBaseAddr, ConfigAddress, ConfigData);
	DataPtr = (u32) &ConfigData + 3 - (Offset & 3);
	*Data = *(u8 *) DataPtr;
	return (XST_SUCCESS);
}


/****************************************************************************/
/**
* Perform a 16 bit read transaction in PCI configuration space. Together,
* the Bus, Device, Func, & Offset form the address of the PCI target to
* access.
*
* @param InstancePtr is the PCI component to operate on.
* @param Bus is the target PCI Bus #.
* @param Device is the target device number.
* @param Func is the target device's function number.
* @param Offset is the target device's configuration space I/O offset to address.
* @param Data is the data read from the target.
*
* @return
*
* - XST_SUCCESS Operation was successfull.
* - XPCI_INVALID_ADDRESS One of Bus, Device, Func, or Offset form an invalid
*   address.
*
* @note
*
* None
*
*****************************************************************************/
int XPci_ConfigIn16(XPci * InstancePtr, unsigned Bus, unsigned Device,
		    unsigned Func, unsigned Offset, u16 *Data)
{
	u32 ConfigAddress;
	u32 ConfigData;
	u32 DataPtr;

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/*
	 * Format the config address
	 */
	ConfigAddress = FormatConfig(Bus, Device, Func, Offset);
	if (ConfigAddress == INVALID_ADDRESS) {
		return (XST_PCI_INVALID_ADDRESS);
	}

	/*
	 * Perform the configuration read then return the correct word
	 */
	XPci_mConfigIn(InstancePtr->RegBaseAddr, ConfigAddress, ConfigData);
	DataPtr = (u32) &ConfigData + 2 - (Offset & 2);
	*Data = *(u16 *) DataPtr;
	return (XST_SUCCESS);
}


/****************************************************************************/
/**
* Perform a 32 bit read transaction in PCI configuration space. Together,
* the Bus, Device, Func, & Offset form the address of the PCI target to
* access.
*
* @param InstancePtr is the PCI component to operate on.
* @param Bus is the target PCI Bus #.
* @param Device is the target device number.
* @param Func is the target device's function number.
* @param Offset is the target device's configuration space I/O offset to address.
* @param Data is the data read from the target.
*
* @return
*
* - XST_SUCCESS Operation was successfull.
* - XPCI_INVALID_ADDRESS One of Bus, Device, Func, or Offset form an invalid
*   address.
*
* @note
*
* None
*
*****************************************************************************/
int XPci_ConfigIn32(XPci * InstancePtr, unsigned Bus, unsigned Device,
		    unsigned Func, unsigned Offset, u32 *Data)
{
	u32 ConfigAddress;

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/*
	 * Format the config address
	 */
	ConfigAddress = FormatConfig(Bus, Device, Func, Offset);
	if (ConfigAddress == INVALID_ADDRESS) {
		return (XST_PCI_INVALID_ADDRESS);
	}

	/*
	 * Perform the configuration read and return
	 */
	XPci_mConfigIn(InstancePtr->RegBaseAddr, ConfigAddress, *Data);
	return (XST_SUCCESS);
}


/****************************************************************************/
/**
* Perform a 8 bit write transaction in PCI configuration space. Together,
* the Bus, Device, Func, & Offset form the address of the PCI target to
* access.
*
* @param InstancePtr is the PCI component to operate on.
* @param Bus is the target PCI Bus #.
* @param Device is the target device number.
* @param Func is the target device's function number.
* @param Offset is the target device's configuration space I/O offset to address.
* @param Data is the data to write
*
* @return
*
* - XST_SUCCESS Operation was successfull.
* - XPCI_INVALID_ADDRESS One of Bus, Device, Func, or Offset form an invalid
*   address.
*
* @note
*
* None
*
*****************************************************************************/
int XPci_ConfigOut8(XPci * InstancePtr, unsigned Bus, unsigned Device,
		    unsigned Func, unsigned Offset, u8 Data)
{
	u32 ConfigAddress;
	u32 ConfigData;
	u32 DataPtr;

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/*
	 * Format the config address
	 */
	ConfigAddress = FormatConfig(Bus, Device, Func, Offset);
	if (ConfigAddress == INVALID_ADDRESS) {
		return (XST_PCI_INVALID_ADDRESS);
	}

	/*
	 * Perform configuration read. Merge in the Data. Then perform a
	 * configuration write.
	 */
	XPci_mConfigIn(InstancePtr->RegBaseAddr, ConfigAddress, ConfigData);
	DataPtr = (u32) &ConfigData + 3 - (Offset & 3);
	*(u8 *) DataPtr = Data;
	XPci_mConfigOut(InstancePtr->RegBaseAddr, ConfigAddress, ConfigData);
	return (XST_SUCCESS);
}


/****************************************************************************/
/**
* Perform a 16 bit write transaction in PCI configuration space. Together,
* the Bus, Device, Func, & Offset form the address of the PCI target to
* access.
*
* @param InstancePtr is the PCI component to operate on.
* @param Bus is the target PCI Bus #.
* @param Device is the target device number.
* @param Func is the target device's function number.
* @param Offset is the target device's configuration space I/O offset to address.
* @param Data is the data to write
*
* @return
*
* - XST_SUCCESS Operation was successfull.
* - XPCI_INVALID_ADDRESS One of Bus, Device, Func, or Offset form an invalid
*   address.
*
* @note
*
* None
*
*****************************************************************************/
int XPci_ConfigOut16(XPci * InstancePtr, unsigned Bus, unsigned Device,
		     unsigned Func, unsigned Offset, u16 Data)
{
	u32 ConfigAddress;
	u32 ConfigData;
	u32 DataPtr;

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/*
	 * Format the config address
	 */
	ConfigAddress = FormatConfig(Bus, Device, Func, Offset);
	if (ConfigAddress == INVALID_ADDRESS) {
		return (XST_PCI_INVALID_ADDRESS);
	}

	/*
	 * Perform configuration read. Merge in the Data. Then perform a configuration
	 * write.
	 */
	XPci_mConfigIn(InstancePtr->RegBaseAddr, ConfigAddress, ConfigData);
	DataPtr = (u32) &ConfigData + 2 - (Offset & 2);
	*(u16 *) DataPtr = Data;
	XPci_mConfigOut(InstancePtr->RegBaseAddr, ConfigAddress, ConfigData);
	return (XST_SUCCESS);
}


/****************************************************************************/
/**
* Perform a 32 bit write transaction in PCI configuration space. Together,
* the Bus, Device, Func, & Offset form the address of the PCI target to
* access.
*
* @param InstancePtr is the PCI component to operate on.
* @param Bus is the target PCI Bus #.
* @param Device is the target device number.
* @param Func is the target device's function number.
* @param Offset is the target device's configuration space I/O offset to address.
* @param Data is the data to write
*
* @return
*
* - XST_SUCCESS Operation was successfull.
* - XPCI_INVALID_ADDRESS One of Bus, Device, Func, or Offset form an invalid
*   address.
*
* @note
*
* None
*
*****************************************************************************/
int XPci_ConfigOut32(XPci * InstancePtr, unsigned Bus, unsigned Device,
		     unsigned Func, unsigned Offset, u32 Data)
{
	u32 ConfigAddress;

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/*
	 * Format the config address
	 */
	ConfigAddress = FormatConfig(Bus, Device, Func, Offset);
	if (ConfigAddress == INVALID_ADDRESS) {
		return (XST_PCI_INVALID_ADDRESS);
	}

	/*
	 * Perform the configuration write.
	 */
	XPci_mConfigOut(InstancePtr->RegBaseAddr, ConfigAddress, Data);
	return (XST_SUCCESS);
}


/****************************************************************************/
/**
* Validate configuration data. If valid, format address into something that
* can be written to the PCI configuration access register (CAR). The valid
* ranges for Bus, Device, Function, and Offset are set by the PCI standard.
*
* @param Bus is the target PCI Bus #.
* @param Device is the target device number.
* @param Func is the target device's function number.
* @param Offset is the target device's configuration space I/O offset to address.
*
* @return
*
* - Formatted 32 bit value suitable for writing in the configuration address
*   register (CAR)
* - INVALID_ADDRESS One of Bus, Device, Func, or Offset form an invalid
*   address.
* @note
*
* None
*
*****************************************************************************/
static u32 FormatConfig(unsigned Bus, unsigned Device,
			unsigned Func, unsigned Offset)
{
	u32 CarData;

	if ((Bus > 255) || (Device > 31) || (Func > 7) || (Offset > 255)) {
		return (INVALID_ADDRESS);
	}

	CarData = XPci_ConfigPack(Bus, Device, Func);

	/* Add Offset value and configuration enable bit to ConfigAddress */
	CarData |= (0x80000000 | (Offset & 0xFC));
	return (CarData);
}
