/* $Id: xpci.c 1948 2013-08-15 18:48:49Z claus $ */
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
/******************************************************************************
*
* @file xpci.c
*
* Implements initialization and simple PCI configuration functions for the XPci
* component. See xpci.h for more information about the component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.00a rmm  03/25/02 Original code.
* 1.00b rpm  05/11/04 Use IdselAddrBit in the config data structure to
*                     determine correct PCI device number when setting the
*                     PCI bar registers in Initialize().
* 1.01a jhl  06/24/04 Added new IPIFBAR2PCIBAR and Host Bridge Device Number
*                     registers with functions to access them. Removed the
*                     configuration of the BARs from the intialize function
*                     as this will now be done by the application using the
*                     driver.
* 1.02a ecm  12/01/06 Added new XPCI_IR_LM_PFT_R error, driver is now designed
*                     all bus attachment methods of the PCI.
* 1.12a ecm  03/24/07 Removed IPIF driver, new coding standard updates.
* </pre>
*
******************************************************************************/

/****************************** Include Files ********************************/
#include "xpci.h"
#include "xparameters.h"

/*************************** Constant Definitions ****************************/

/*
 * IDSEL address bit in hardware determines PCI device number (e.g., address
 * bit 16 indicates PCI device number 0, bit 17 is device number 1, etc...).
 * Address bit ranges from 16 to 31.
 */
#define XPCI_IDSEL_ADDR_BIT_MIN     16	/* Minimum IDSEL address bit */
#define XPCI_IDSEL_ADDR_BIT_MAX     31	/* Maximum IDSEL address bit */

/***************************** Type Definitions ******************************/

/****************** Macros (Inline Functions) Definitions ********************/

/*************************** Variable Definitions ****************************/

/*************************** Function Prototypes *****************************/


/****************************************************************************/
/**
* Initialize the XPci instance provided by the caller based on the
* given DeviceID.
*
* Initialization includes setting up the bar registers in the bridge's PCI
* header to match the IPIF settings. Not performing this step will cause the
* the IPIF to not respond to PCI bus hits.
*
* @param InstancePtr is a pointer to an XPci instance. The memory the pointer
* references must be pre-allocated by the caller. Further calls to manipulate
* the component through the XPci API must be made with this pointer.
*
* @param DeviceId is the unique id of the device controlled by this XPci
* component.  Passing in a device id associates the generic XPci
* instance to a specific device, as chosen by the caller or application
* developer.
*
* @param BusNo is the initial PCI bus number to assign to the host bridge. This
* value can be changed later with a call to XPci_SetBusNumber()
*
* @param SubBusNo is the initial PCI sub-bus number to assign to the host bridge
* This value can be changed later with a call to XPci_SetBusNumber()
*
* @return
*
* - XST_SUCCESS Initialization was successfull.
* - XST_DEVICE_NOT_FOUND Device configuration data was not found for a device
*   with the supplied device ID.
*
* @note
*
* None
*
*****************************************************************************/
int XPci_Initialize(XPci * InstancePtr, u16 DeviceId, int BusNo, int SubBusNo)
{
	XPci_Config *ConfigPtr;

	/* Assert arguments */
	XASSERT_NONVOID(InstancePtr != NULL);

	/*
	 * Lookup configuration data in the device configuration table.
	 * Use this configuration info down below when initializing this component.
	 */
	ConfigPtr = XPci_LookupConfig(DeviceId);
	if (ConfigPtr == (XPci_Config *) NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	InstancePtr->ConfigPtr = ConfigPtr;

	/* Set some default values */
	InstancePtr->RegBaseAddr = ConfigPtr->RegBaseAddr;
	InstancePtr->DmaRegBaseAddr = ConfigPtr->DmaBaseAddr;
	InstancePtr->DmaType = ConfigPtr->DmaType;

	/* Disable interrupts at the global level */
	XPci_mIntrGlobalDisable(InstancePtr->RegBaseAddr);

	/* Disable and clear all pending interrupts at the device level */
	XPci_mIntrDisable(InstancePtr->RegBaseAddr, XPCI_IPIF_INT_MASK);
	XPci_mIntrClear(InstancePtr->RegBaseAddr, XPCI_IPIF_INT_MASK);

	/* Disable and clear all pending interrupts at the PCI level */
	XPci_mIntrPciDisable(InstancePtr->RegBaseAddr, XPCI_IR_MASK);
	XPci_mIntrPciClear(InstancePtr->RegBaseAddr, XPCI_IR_MASK);

	/*
	 * Set the bus number
	 * Indicate the instance is now ready to use, initialized without error
	 */
	XPci_SetBusNumber(InstancePtr, BusNo, SubBusNo);
	InstancePtr->IsReady = XCOMPONENT_IS_READY;

	return (XST_SUCCESS);
}

/****************************************************************************/
/**
* Lookup the device configuration based on the unique device ID.  The table
* ConfigTable contains the configuration info for each device in the system.
*
* @param DeviceId is the device identifier to lookup.
*
* @return
*
* - XEmc configuration structure pointer if DeviceID is found.
* - NULL if DeviceID is not found.
*
*****************************************************************************/
XPci_Config *XPci_LookupConfig(u16 DeviceId)
{
	extern XPci_Config XPci_ConfigTable[];
	XPci_Config *CfgPtr = NULL;

	int i;

	for (i = 0; i < XPAR_XPCI_NUM_INSTANCES; i++) {
		if (XPci_ConfigTable[i].DeviceId == DeviceId) {
			CfgPtr = &XPci_ConfigTable[i];
			break;
		}
	}

	return (CfgPtr);
}


/****************************************************************************/
/**
* Reset the PCI IP core. This is a destructive operation that could cause
* loss of data, local bus errors, or PCI bus errors if reset occurs while
* a transaction is pending.
*
* @param InstancePtr is the PCI component to operate on.
*
* @note
*
* None
*
*****************************************************************************/
void XPci_Reset(XPci * InstancePtr)
{
	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
	XPci_mReset(InstancePtr->RegBaseAddr);
}


/****************************************************************************/
/**
* Pack configuration address data.
*
* @param Bus is the PCI bus number. Valid range 0..255.
* @param Device is the PCI device number. Valid range 0..31.
* @param Function is the PCI function number. Valid range 0..7.
*
* @return Encoded Bus, Device & Function formatted to be written to
*         PCI configuration address register.
*
* @note
*
* None
*
*****************************************************************************/
u32 XPci_ConfigPack(unsigned Bus, unsigned Device, unsigned Function)
{
	u32 CarData;

	/* Encode to PCI configuration access specifications */
	CarData = (Bus << 16) & 0x00FF0000;
	CarData |= (Device << 11) & 0x0000F800;
	CarData |= (Function << 8) & 0x00000700;
	return (CarData);
}

/****************************************************************************/
/**
* Perform a 32 bit configuration read transaction.
*
* @param InstancePtr is the PCI component to operate on.
* @param ConfigAddress contains the address of the PCI device to access.
*        It should be properly formatted for writing to the PCI configuration
*        access port. (see XPci_ConfigPack())
* @param Offset is the register offset within the PCI device being accessed.
*
* @return 32 bit data word from addressed device
*
* @note
*
* This function performs the same type of operation that XPci_ConfigIn32,
* does except the user must format the ConfigAddress
*
*****************************************************************************/
u32 XPci_ConfigIn(XPci * InstancePtr, u32 ConfigAddress, u8 Offset)
{
	u32 Data;

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* Add Offset value and configuration enable bit to ConfigAddress */
	ConfigAddress |= (0x80000000 | (Offset & 0xFC));
	XPci_mConfigIn(InstancePtr->RegBaseAddr, ConfigAddress, Data);
	return (Data);
}

/****************************************************************************/
/**
* Perform a 32 bit configuration write transaction.
*
* @param InstancePtr is the PCI component to operate on.
*
* @param ConfigAddress contains the address of the PCI device to access.
*        It should be properly formatted for writing to the PCI configuration
*        access port. (see XPci_ConfigPack())
* @param Offset is the register offset within the PCI device being accessed.
* @param ConfigData is the data to write to the addressed device.
*
* @note
*
* This function performs the same type of operation that XPci_ConfigOutWord,
* does except the user must format the Car.
*
*****************************************************************************/
void XPci_ConfigOut(XPci * InstancePtr, u32 ConfigAddress, u8 Offset,
		    u32 ConfigData)
{
	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* Add Offset value and configuration enable bit to ConfigAddress */
	ConfigAddress |= (0x80000000 | (Offset & 0xFC));
	XPci_mConfigOut(InstancePtr->RegBaseAddr, ConfigAddress, ConfigData);
}


/****************************************************************************/
/**
* Get a snapshot of the PCI bridge's error state, summarize and place results
* in an XPciError structure. Several bridge registers are read and their
* contents placed into the structure as follows. Register definitions and their
* bitmaps are located in xpci_l.h:
*
* <pre>
*    Attribute            Source Register
*    -------------------  -----------------
*    LocalBusReason       XPCI_LMADDR_OFFSET
*    PciReason            XPCI_PIADDR_OFFSET
*    PciSerrReason        IPIF IISR
*    LocalBusReadAddr     XPCI_LMA_R_OFFSET
*    LocalBusWriteAddr    XPCI_LMA_W_OFFSET
*    PciSerrReadAddr      XPCI_SERR_R_OFFSET
*    PciSerrWriteAddr     XPCI_SERR_W_OFFSET
*    PciReadAddr          XPCI_PIA_R_OFFSET
*    PciWriteAddr         XPCI_PIA_W_OFFSET
* </pre>
*
* LocalBusReadAddr, LocalBusWriteAddr, PciSerrReadAddr, PciSerrWriteAddr,
* PCIReadAddr, and PciWriteAddr are all error addresses whose contents are
* latched at the time of the error.
*
* LocalBusReason and PciReason are present to allow the caller
* to precicely determine the source of the error. The summary below
* indicates which bits cause the associated error address to become valid and
* which interrupt bits from interrupt status register are the cause if
* the error was reported via an interrupt.
*
* <pre>
* LocalBusReason:
*    Bit                     Error addr is valid  Associated Interrupt bit
*    ----------------------- -------------------  ------------------------
*    XPCI_LMADDR_SERR_R      LocalBusReadAddr     XPCI_IR_LM_SERR_R
*    XPCI_LMADDR_PERR_R      LocalBusReadAddr     XPCI_IR_LM_PERR_R
*    XPCI_LMADDR_TA_R        LocalBusReadAddr     XPCI_IR_LM_TA_R
*    XPCI_LMADDR_PFT_R       LocalBusReadAddr     XPCI_IR_LM_PFT_R
*    XPCI_LMADDR_SERR_W      LocalBusWriteAddr    XPCI_IR_LM_SERR_W
*    XPCI_LMADDR_PERR_W      LocalBusWriteAddr    XPCI_IR_LM_PERR_W
*    XPCI_LMADDR_TA_W        LocalBusWriteAddr    XPCI_IR_LM_TA_W
*    XPCI_LMADDR_MA_W        LocalBusWriteAddr    XPCI_IR_LM_MA_W
*    XPCI_LMADDR_BR_W        LocalBusWriteAddr    XPCI_IR_LM_BR_W
*    XPCI_LMADDR_BRD_W       LocalBusWriteAddr    XPCI_IR_LM_BRD_W
*    XPCI_LMADDR_BRT_W       LocalBusWriteAddr    XPCI_IR_LM_BRT_W
*
* PciReason:
*    Bit                     Error addr is valid  Associated Interrupt bit
*    ----------------------- -------------------  ------------------------
*    XPCI_PIADDR_ERRACK_R    PciReadAddr          N/A
*    XPCI_PIADDR_ERRACK_W    PciWriteAddr         N/A
*    XPCI_PIADDR_RETRY_W     PciWriteAddr         N/A
*    XPCI_PIADDR_TIMEOUT_W   PciWriteAddr         N/A
*    XPCI_PIADDR_RANGE_W     PciWriteAddr         N/A
*
* PciReasonSerr:
*    Bit                     Error addr is valid  Associated Interrupt bit
*    ----------------------- -------------------  ------------------------
*    XPCI_IR_PI_SERR_R       PciSerrReadAddr      XPCI_IR_PI_SERR_R
*    XPCI_IR_PI_SERR_W       PciSerrWriteAddr     XPCI_IR_PI_SERR_W
*    XPCI_IR_BAR_ORUN_R      N/A                  XPCI_IR_BAR_ORUN_R
*    XPCI_IR_BAR_ORUN_W      N/A                  XPCI_IR_BAR_ORUN_W
* </pre>
*
* If any of the above mentioned error reason bits are set, then attribute
* IsError is set to TRUE. If no errors are detected, then it is set to FALSE.
*
* @param InstancePtr is the PCI component to operate on.
* @param ErrorDataPtr is the error snapshot data returned from the PCI bridge.
*
* @note
*
* None
*
*****************************************************************************/
void XPci_ErrorGet(XPci * InstancePtr, XPciError * ErrorDataPtr)
{
	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
	XASSERT_VOID(ErrorDataPtr != NULL);

	/* Read all pertinent registers into the ErrorDataPtr structure */
	ErrorDataPtr->LocalBusReason =
		XPci_mReadReg(InstancePtr->RegBaseAddr, XPCI_LMADDR_OFFSET);
	ErrorDataPtr->LocalBusReadAddr =
		XPci_mReadReg(InstancePtr->RegBaseAddr, XPCI_LMA_R_OFFSET);
	ErrorDataPtr->LocalBusWriteAddr =
		XPci_mReadReg(InstancePtr->RegBaseAddr, XPCI_LMA_W_OFFSET);

	/* these registers are not supported in HW yet */
#if 0
	ErrorDataPtr->PciReason =
		XPci_mReadReg(InstancePtr->RegBaseAddr, XPCI_PIADDR_OFFSET);
	ErrorDataPtr->PciReadAddr =
		XPci_mReadReg(InstancePtr->RegBaseAddr, XPCI_PIA_R_OFFSET);
	ErrorDataPtr->PciWriteAddr =
		XPci_mReadReg(InstancePtr->RegBaseAddr, XPCI_PIA_W_OFFSET);
	ErrorDataPtr->PciSerrReadAddr =
		XPci_mReadReg(InstancePtr->RegBaseAddr, XPCI_SERR_R_OFFSET);
	ErrorDataPtr->PciSerrWriteAddr =
		XPci_mReadReg(InstancePtr->RegBaseAddr, XPCI_SERR_W_OFFSET);
#endif

	ErrorDataPtr->PciReason = 0;
	ErrorDataPtr->PciReadAddr = 0;
	ErrorDataPtr->PciWriteAddr = 0;
	ErrorDataPtr->PciSerrReadAddr = 0;
	ErrorDataPtr->PciSerrWriteAddr = 0;


	/* SERR is determined by two bits in the interrupt status register */
	ErrorDataPtr->PciSerrReason =
		XPci_mIntrPciReadISR(InstancePtr->RegBaseAddr);
	ErrorDataPtr->PciSerrReason &= (XPCI_IR_PI_SERR_R | XPCI_IR_PI_SERR_W);

	/* Calculate summary */
	if (ErrorDataPtr->LocalBusReason ||
	    ErrorDataPtr->PciReason || ErrorDataPtr->PciSerrReason) {
		ErrorDataPtr->IsError = TRUE;
	}
	else {
		ErrorDataPtr->IsError = FALSE;
	}
}

/****************************************************************************/
/**
* Clear errors associated with the PCI bridge. Which errors are cleared depend
* on the Reason attributes of the ErrorData parameter. For every bit set, that
* corresponding error is cleared.
*
* XPci_ErrorGet() and XPci_ErrorClear() are designed to be used in tandem.
* Use ErrorGet to retrieve the errors, then ErrorClear to clear the error
* state.
*
*   XPci_ErrorGet(ThisInstance, &Errors)
*   if (Errors->IsError)
*   {
*      // Handle error
*      XPci_ErrorClear(ThisInstance, &Errors);
*   }
*
* If it is desired to clear some but not all errors, or a specific set of
* errors, then prepare ErrorData Bitmap attributes appropriately. If it is
* desired to clear all errors indiscriminately, then use XPCI_CLEAR_ALL_ERRORS.
* This has the advantage of not requiring the caller to explicitly setup an
* XPciError structure.
*
* @param InstancePtr is the PCI component to operate on.
*
* @param ErrorDataPtr is used to determine which error conditions to clear. Only
*        the Bitmap attributes are used. Addr attributes of this structure are
*        ignored. If this parameter is set to XPCI_CLEAR_ALL_ERRORS then all
*        errors are cleared.
*
* @note
*
* If PciSerrReason attribute is set or XPCI_CLEAR_ALL_ERRORS is passed, then
* the IPIF interrupt status register bits associated with SERR are cleared.
* This has the same effect as acknowledging an interrupt. If you don't intend
* on doing this, then clear PciSerrReason before calling XPci_ErrorClear.
*
*****************************************************************************/
void XPci_ErrorClear(XPci * InstancePtr, XPciError * ErrorDataPtr)
{
	u32 PciClear;
	u32 LocalClear;
	u32 SerrClear;
	u32 Temp;

	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
	XASSERT_VOID(ErrorDataPtr != NULL);

	/*
	 * Clear all errors if XPCI_CLEAR_ALL_ERRORS is passed instead
	 * of an ErrorData structure
	 */
	if (ErrorDataPtr == XPCI_CLEAR_ALL_ERRORS) {
		PciClear = XPCI_PIADDR_MASK;
		LocalClear = XPCI_LMADDR_MASK;
		SerrClear = XPCI_IR_PI_SERR_R | XPCI_IR_PI_SERR_W;
	}

	/*
	 * Clear errors specified by the ErrorData structure
	 */
	else {
		PciClear = ErrorDataPtr->PciReason;
		LocalClear = ErrorDataPtr->LocalBusReason;
		SerrClear = ErrorDataPtr->PciSerrReason &
			(XPCI_IR_PI_SERR_R | XPCI_IR_PI_SERR_W);
	}

	/* Clear local bus reason code */
	Temp = XPci_mReadReg(InstancePtr->RegBaseAddr, XPCI_LMADDR_OFFSET);
	XPci_mWriteReg(InstancePtr->RegBaseAddr, XPCI_LMADDR_OFFSET,
		       Temp & LocalClear);

	/* Clear pci bus reason code */
	/* Note: This function is not yet supported in HW */
#if 0
	Temp = XPci_mReadReg(InstancePtr->RegBaseAddr, XPCI_PIADDR_OFFSET);
	XPci_mWriteReg(InstancePtr->RegBaseAddr, XPCI_PIADDR_OFFSET,
		       Temp & PciClear);
#endif

	/* Clear interrupt status */
	XPci_mIntrPciClear(InstancePtr->RegBaseAddr, SerrClear);
}

/****************************************************************************/
/**
* Change how the bridge handles subsequent PCI transactions after errors
* occur. Transactions can be prohibited once an error occurs then allowed
* again once the error is cleared. Or transactions are be allowed to continue
* despite an error condition.
*
* @param InstancePtr is the PCI component to operate on.
*
* @param Mask defines the type of transactions affected. OR together bits
* from XPCI_INHIBIT_* to form the mask. Bits set to 1 will cause transactions
* to be inhibited when an error exists. Bits set to 0 will allow transactions
* to proceed.
*
* @note
*
* None
*
*****************************************************************************/
void XPci_InhibitAfterError(XPci * InstancePtr, u32 Mask)
{
	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* this register is not supported in HW yet */
#if 0
	XPci_mWriteReg(InstancePtr->RegBaseAddr, XPCI_INHIBIT_OFFSET,
		       (XPCI_INHIBIT_MASK & Mask));
#endif
}


/****************************************************************************/
/**
* Set the bus number and subordinate bus number of the pci bridge. This
* function has effect only if the PCI bridge is configured as a PCI host
* bridge.
*
* @param InstancePtr is the PCI component to operate on.
* @param BusNo is the bus number to set
* @param SubBusNo is the subordinate bus number to set
*
* @note
*
* None
*
*****************************************************************************/
void XPci_SetBusNumber(XPci * InstancePtr, int BusNo, int SubBusNo)
{
	u32 Data;

	XASSERT_VOID(InstancePtr != NULL);

	Data = (BusNo << 16) | SubBusNo;
	Data &= (XPCI_BUSNO_BUS_MASK | XPCI_BUSNO_SUBBUS_MASK);

	XPci_mWriteReg(InstancePtr->RegBaseAddr, XPCI_BUSNO_OFFSET, Data);
}


/****************************************************************************/
/**
* Get the bus number and subordinate bus number of the pci bridge.
*
* @param InstancePtr is the PCI component to operate on
* @param BusNoPtr is storage to place the bus number
* @param SubBusNoPtr is storage to place the subordinate bus number
*
* @note
*
* None
*
*****************************************************************************/
void XPci_GetBusNumber(XPci * InstancePtr, int *BusNoPtr, int *SubBusNoPtr)
{
	u32 Data;

	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(BusNoPtr != NULL);
	XASSERT_VOID(SubBusNoPtr != NULL);

	Data = XPci_mReadReg(InstancePtr->RegBaseAddr, XPCI_BUSNO_OFFSET);
	*BusNoPtr = Data >> 16;
	*SubBusNoPtr = Data & XPCI_BUSNO_SUBBUS_MASK;
}


/****************************************************************************/
/**
* Get the DMA engine implementation information for this instance.
*
* @param InstancePtr is the PCI component to operate on.
*
* @param BaseAddr is a return value indicating the base address of the
* DMA registers.
*
* @param DmaType is a return value indicating the type of DMA implemented.
* The possible types are XPCI_DMA_TYPE_NONE for no DMA, XPCI_DMA_TYPE_SIMPLE
* for simple DMA, and XPCI_DMA_TYPE_SG for scatter-gather DMA.
*
* @note
*
* None
*
*****************************************************************************/
void XPci_GetDmaImplementation(XPci * InstancePtr, u32 *BaseAddr, u8 *DmaType)
{
	/* check for parameter errors */
	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(BaseAddr != NULL);
	XASSERT_VOID(DmaType != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* set output parameters and return */
	*BaseAddr = InstancePtr->DmaRegBaseAddr;
	*DmaType = InstancePtr->DmaType;
}



/****************************************************************************/
/**
* Get a IPIF BAR to PCI BAR register from the PCI bridge. The register
* determines the address translation between the corresponding IPIF Base
* Address (BAR) and the PCI Base Address. The translation occurs by
* substituting the M highest order address bits of IPIFBAR_N with the
* the register value.
*
* The width of each register is given by the number of fixed bits that define
* the complete address range of the associated IPIFBAR_N. For example, if
* C_IPIFBAR_0 = 0xFE000000 and C_IPIF_HIGHADDR_0 = 0xFFFFFFFF, then
* the register would be M = 7 bits wide. The number of registers that are
* present is given by the number of IPIF BARs configured.
*
* These registers are included in the PCI bridge via parameter
* C_INCLUDE_BAROFFSET_REG and when parameter C_INCLUDE_OPB_MST2PCI_TARG = 1.
*
* @param InstancePtr is the PCI component to operate on.
* @param BarNumber is the IPIFBAR2PCIBAR register within the PCI device
* being accessed and is zero based with a range of 0 - 5 being valid.
*
* @return 32 bit data word from register
*
* @note
*
* This function asserts if the specified register in the PCI Bridge does not
* exist. It does not check the C_INCLUDE_OPB_MST2PCI_TARG and assumes this
* is done when building the system.
*
*****************************************************************************/
u32 XPci_GetIpifBar2PciBar(XPci * InstancePtr, u32 BarNumber)
{
	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(BarNumber < InstancePtr->ConfigPtr->IpifBarNum);
	XASSERT_NONVOID(InstancePtr->ConfigPtr->IncludeBarOffsetReg != FALSE);

	return XPci_mReadReg(InstancePtr->RegBaseAddr,
			     XPCI_IPIF2PCI_0_OFFSET +
			     (BarNumber * sizeof(u32)));
}

/****************************************************************************/
/**
* Set a IPIF BAR to PCI BAR register in the PCI bridge. The register
* determines the address translation between the corresponding IPIF Base
* Address (BAR) and the PCI Base Address. The translation occurs by
* substituting the M highest order address bits of IPIFBAR_N with the
* the register value.
*
* The width of each register is given by the number of fixed bits that define
* the complete address range of the associated IPIFBAR_N. For example, if
* C_IPIFBAR_0 = 0xFE000000 and C_IPIF_HIGHADDR_0 = 0xFFFFFFFF, then
* the register would be M = 7 bits wide. The number of registers that are
* present is given by the number of IPIF BARs configured.
*
* These registers are included in the PCI bridge via parameter
* C_INCLUDE_BAROFFSET_REG and when parameter C_INCLUDE_OPB_MST2PCI_TARG = 1.
*
* @param InstancePtr is the PCI component to operate on.
* @param BarNumber is the IPIFBAR2PCIBAR register within the PCI device
* being accessed and is zero based with a range of 0 - 5 being valid.
*
* @note
*
* This function asserts if the specified register in the PCI Bridge does not
* exist. It does not check the C_INCLUDE_OPB_MST2PCI_TARG and assumes this
* is done when building the system.
*
*****************************************************************************/
void XPci_SetIpifBar2PciBar(XPci * InstancePtr, u32 BarNumber, u32 ValueToSet)
{
	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(BarNumber < InstancePtr->ConfigPtr->IpifBarNum);
	XASSERT_VOID(InstancePtr->ConfigPtr->IncludeBarOffsetReg != FALSE);

	XPci_mWriteReg(InstancePtr->RegBaseAddr,
		       XPCI_IPIF2PCI_0_OFFSET + (BarNumber * sizeof(u32)),
		       ValueToSet);
}

/****************************************************************************/
/**
* Set the host bridge device number in the PCI bridge.  This register is
* included in the PCI bridge when parameter C_INCLUDE_DEVNUM_REG and \
* parameter C_INCLUDE_PCI_CONFIG = 1.
*
* If the device number register has been built into the hardware, this
* register must be initialized before any configuration accesses are done.
*
* @param InstancePtr is the PCI component to operate on.
* @param DeviceNumber is the Host Bridge Device Number used by the PCI Bridge.
*
* @note
*
* This function asserts if the register in the PCI Bridge does not
* exist. It does not check the C_INCLUDE_PCI_CONFIG and assumes this
* is done when building the system.
*
*****************************************************************************/
void XPci_SetHostBridgeDevice(XPci * InstancePtr, u32 DeviceNumber)
{
	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->ConfigPtr->IncludeDevNumReg != FALSE);

	XPci_mWriteReg(InstancePtr->RegBaseAddr, XPCI_HBDN_OFFSET,
		       DeviceNumber);
}

/****************************************************************************/
/**
* Get the host bridge device number from the PCI bridge.  This register is
* included in the PCI bridge when parameter C_INCLUDE_DEVNUM_REG and \
* parameter C_INCLUDE_PCI_CONFIG = 1.
*
* @param InstancePtr is the PCI component to operate on.
*
* @return DeviceNumber is the Host Bridge Device Number from the PCI Bridge.
*
* @note
*
* This function asserts if the register in the PCI Bridge does not
* exist. It does not check the C_INCLUDE_PCI_CONFIG and assumes this
* is done when building the system.
*
*****************************************************************************/
u32 XPci_GetHostBridgeDevice(XPci * InstancePtr)
{
	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->ConfigPtr->IncludeDevNumReg != FALSE);

	return XPci_mReadReg(InstancePtr->RegBaseAddr, XPCI_HBDN_OFFSET);
}
