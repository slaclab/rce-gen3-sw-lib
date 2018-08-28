/**************************************************************************
*
*     XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
*     SOLELY FOR USE IN DEVELOPING PROGRAMS AND SOLUTIONS FOR
*     XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION
*     AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE, APPLICATION
*     OR STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS
*     IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
*     AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
*     FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
*     WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
*     IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
*     REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
*     INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*     FOR A PARTICULAR PURPOSE.
*
*     (c) Copyright 2010 Xilinx, Inc.
*     All rights reserved.
*
**************************************************************************/

#include "xil_printf.h"
#include "sleep.h"
#include "gpio_utils.h"


#define SLEEP_TIME 10000

#define ZC702_IIC_MUX_RST_B_GPIO_PIN  13  // MIO
#define FMC_IIC_MUX_RST_B_GPIO_PIN    60  // EMIO


int init_GpioPs(XGpioPs *GpioPs)
{
	int Status;
	XGpioPs_Config *GpioPs_Config;

	GpioPs_Config = XGpioPs_LookupConfig(GPIO_DEVICE_ID);
	if (GpioPs_Config == NULL) {
		xil_printf("No GPIO found for ID %d\n\r", GPIO_DEVICE_ID);
		return XST_FAILURE;
	}

	Status = XGpioPs_CfgInitialize(GpioPs, GpioPs_Config, GpioPs_Config->BaseAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("GPIO Initialization failed %d\n\r", Status);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}


int gpio_read(XGpioPs *GpioPs, int Pin, int Data)
{
   // Set the direction to input
   XGpioPs_SetDirectionPin(GpioPs, Pin, 0);

   // Read data from register
   return XGpioPs_ReadPin(GpioPs, Pin);
}


void gpio_write(XGpioPs *GpioPs, int Pin, int Data)
{
   // Set the direction to output and enable output
   XGpioPs_SetDirectionPin(GpioPs, Pin, 1);
   XGpioPs_SetOutputEnablePin(GpioPs, Pin, 1);

   // Write data to register.
   XGpioPs_WritePin(GpioPs, Pin, Data);
}


void fmc_iic_mux_reset(XGpioPs *GpioPs)
{
	// Toggle reset_b pin:  0 -> 1
	gpio_write(GpioPs, FMC_IIC_MUX_RST_B_GPIO_PIN, 0);
	usleep(SLEEP_TIME);
	gpio_write(GpioPs, FMC_IIC_MUX_RST_B_GPIO_PIN, 1);
}


void zc702_iic_mux_reset(XGpioPs *GpioPs)
{
	// Toggle reset_b pin:  0 -> 1
	gpio_write(GpioPs, ZC702_IIC_MUX_RST_B_GPIO_PIN, 0);
	usleep(SLEEP_TIME);
	gpio_write(GpioPs, ZC702_IIC_MUX_RST_B_GPIO_PIN, 1);
}
