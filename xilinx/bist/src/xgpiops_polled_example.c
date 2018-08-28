/* $Id: xgpiops_polled_example.c 1947 2013-08-15 18:42:54Z claus $ */
/******************************************************************************
*
* (c) Copyright 2010 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file xgpiops_polled_example.c
*
* This file contains an example for using GPIO hardware and driver.
*
* @note		This example assumes that there is a Uart device in the HW
*		design.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sv   01/18/10 First Release
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xparameters.h"
#include "xgpiops.h"
#include "xstatus.h"
#include <xil_printf.h>

/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define GPIO_DEVICE_ID  	XPAR_XGPIOPS_0_DEVICE_ID

/*
 * The following constant is used to wait after an LED is turned on to make
 * sure that it is visible to the human eye.  This constant might need to be
 * tuned for faster or slower processor speeds.
 */
#define LED_DELAY		20000000

/*
 * Following constant is used to determine which Bank of the GPIO is
 * used for the Input and Output pins.
 */
#define INPUT_BANK		XGPIOPS_BANK0	/* Bank to be used for input */
#define OUTPUT_BANK		XGPIOPS_BANK0	/* Bank to be used for LEDS */

#define LED0			0xA	/* GPIO pin number of first LED */
#define OUTPUT_BANK_WIDTH	0x1	/* 32 pins are connected */

#define LED_MAX_BLINK		0x06	/* Number of times the LED Blinks */

#define printf			xil_printf	/* Smalller foot-print printf */

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions *******************/

/************************** Function Prototypes ****************************/

static int GpioOutputExample(void);
static int GpioInputExample(u32 *DataRead);
int GpioPolledInputExample(u16 DeviceId, u32 *DataRead);
int GpioPolledOutputExample(u16 DeviceId, u32 *DataRead);


/************************** Variable Definitions **************************/

/*
 * The following are declared globally so they are zeroed and can be
 * easily accessible from a debugger.
 */
XGpioPs Gpio;	/* The driver instance for GPIO Device. */

/*****************************************************************************/
/**
*
* Main function to call the example.
*
* @param	None
*
* @return	- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		None
*
******************************************************************************/
int hello_led(void)
{
	int Status;
	u32 InputData;

	xil_printf("\n\r********************************************************");
	xil_printf("\n\r********************************************************");
	xil_printf("\n\r**     ZC702 - GPIO LED Test                          **");
	xil_printf("\n\r********************************************************");
	xil_printf("\n\r********************************************************\r\n");
	xil_printf("Watch the LED\r\n");

	Status = GpioPolledOutputExample(GPIO_DEVICE_ID, &InputData);
	if (Status != XST_SUCCESS) {
		printf("GPIO Polled Mode Example Test Failed\r\n");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Main function to call the example.
*
* @param	None
*
* @return	- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		None
*
******************************************************************************/
int hello_switch(void)
{
	int Status;
	u32 InputData;
	u32 SW13_data;
	u32 SW14_data;
	
	xil_printf("\n\r********************************************************");
	xil_printf("\n\r********************************************************");
	xil_printf("\n\r**     ZC702 - GPIO Switch Test                       **");
	xil_printf("\n\r********************************************************");
	xil_printf("\n\r********************************************************\r\n");

	Status = GpioPolledInputExample(GPIO_DEVICE_ID, &InputData);
	if (Status != XST_SUCCESS) {
		printf("GPIO Switch Test Failed\r\n");
		return XST_FAILURE;
	}
  SW13_data = InputData << 17;
  SW13_data = SW13_data >> 31;
  SW14_data = InputData << 19;
  SW14_data = SW14_data >> 31;
	printf("Data read from SW13 Input is  0x%x \n\r", (int)SW13_data);
	printf("Data read from SW14 Input is  0x%x \n\r", (int)SW14_data);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to illustrate how to use the GPIO driver to
* turn on/off an LED.
*
* @param	DeviceId is the XPAR_<GPIO_instance>_DEVICE_ID value from
*		xparameters.h
* @param	DataRead is the pointer where the data read from GPIO Input is
*		returned.
*
* @return	- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		This function will not return if the test is running.
*
******************************************************************************/
int GpioPolledOutputExample(u16 DeviceId, u32 *DataRead)
{
	int Status;
	XGpioPs_Config *ConfigPtr;

	/*
	 * Initialize the GPIO driver.
	 */
	ConfigPtr = XGpioPs_LookupConfig(GPIO_DEVICE_ID);
	Status = XGpioPs_CfgInitialize(&Gpio, ConfigPtr,
					ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Run the Output Example.
	 */
	Status = GpioOutputExample();
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to illustrate how to use the GPIO driver to
* read the inputs.
*
* @param	DeviceId is the XPAR_<GPIO_instance>_DEVICE_ID value from
*		xparameters.h
* @param	DataRead is the pointer where the data read from GPIO Input is
*		returned.
*
* @return	- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		This function will not return if the test is running.
*
******************************************************************************/
int GpioPolledInputExample(u16 DeviceId, u32 *DataRead)
{
	int Status;
	XGpioPs_Config *ConfigPtr;

	/*
	 * Initialize the GPIO driver.
	 */
	ConfigPtr = XGpioPs_LookupConfig(GPIO_DEVICE_ID);
	Status = XGpioPs_CfgInitialize(&Gpio, ConfigPtr,
					ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Run the Input Example.
	 */
	Status = GpioInputExample(DataRead);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the GPIO device configured as OUTPUT.
*
* @param	None.
*
* @return	- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		None.
*
****************************************************************************/
static int GpioOutputExample(void)
{
	volatile int Delay;
	u32 LedBit;
	u32 LedLoop;

	/*
	 * Set the Mode as Normal Mode, set the direction for all signals to be
	 * outputs and Enable the Output enable for the LED Pins.
	 */
	XGpioPs_SetDirection(&Gpio, OUTPUT_BANK, 0x400);
	XGpioPs_SetOutputEnable(&Gpio, OUTPUT_BANK, 0x400);


	for (LedBit = LED0; LedBit < OUTPUT_BANK_WIDTH + LED0; LedBit++) {

		for (LedLoop = 0; LedLoop < LED_MAX_BLINK; LedLoop++) {

			/*
			 * Clear the GPIO Output.
			 */
			XGpioPs_WritePin(&Gpio, LedBit, 0x0);

			/*
			 * Wait a small amount of time so the LED is visible.
			 */
			for (Delay = 0; Delay < LED_DELAY; Delay++);

			/*
			 * Set the GPIO Output to High.
			 */
			XGpioPs_WritePin(&Gpio, LedBit, 0x1);

			/*
			 * Wait a small amount of time so the LED is visible.
			 */
			for (Delay = 0; Delay < LED_DELAY; Delay++);
		}

	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
*
* This function performs a test on the GPIO driver/device with the GPIO
* configured as INPUT.
*
* @param	DataRead is the pointer where the data read from GPIO Input is
*		returned
*
* @return	- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		None.
*
******************************************************************************/
static int GpioInputExample(u32 *DataRead)
{

	/*
	 * Set the Mode as Normal Mode, set the direction for all signals to be
	 * inputs in the specified BANK.
	 */
	XGpioPs_SetDirection(&Gpio, INPUT_BANK, 0x0);

	/*
	 * Read the state of the data so that it can be  verified.
	 */
	*DataRead = XGpioPs_Read(&Gpio, INPUT_BANK);

	return XST_SUCCESS;
}
