/***************************** Include Files *********************************/

#include "gpio.h"
#include "fsbl.h"
#include "xparameters.h"
#include "xil_exception.h"

/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define GPIO_DEVICE_ID  XPAR_XGPIOPS_0_DEVICE_ID

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions *******************/

/************************** Function Prototypes ****************************/

/************************** Variable Definitions *****************************/
/* Devcfg driver instance */
static XGpioPs GpioInstance;
XGpioPs *GpioInstPtr;

/******************************************************************************/
/**
*
* This function Initializes the GPIO driver.
*
* @param	bank The GPIO bank to address
* @param        pin The GPIO pin to address
*
* @return
*		- XST_SUCCESS if the gpio driver initialization is successful
*		- XST_FAILURE if the gpio driver initialization fails
*
* @note	 none
*
****************************************************************************/
int GpioInit(void)
{
	XGpioPs_Config *ConfigPtr;
	int Status = XST_SUCCESS;
	GpioInstPtr = &GpioInstance;

	/*
	 * Initialize the GPIO driver.
	 */
	ConfigPtr = XGpioPs_LookupConfig(GPIO_DEVICE_ID);

	Status = XGpioPs_CfgInitialize(GpioInstPtr, ConfigPtr,
                                       ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		fsbl_printf(DEBUG_INFO, "XGpioPs_CfgInitialize failed \n\r");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function clears a GPIO pin.
*
* @param	None.
*
* @return	- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		None.
*
****************************************************************************/
int GpioClear(unsigned bank, unsigned pin)
{
	/*
	 * Set the Mode as Normal Mode, set the direction for all signals to be
	 * outputs and Enable the Output enable for the Pin.
	 */
	XGpioPs_SetDirection(&GpioInstance, bank, 1 << pin);
	XGpioPs_SetOutputEnable(&GpioInstance, bank, 1 << pin);

        /*
         * Clear the GPIO Output.
         */
        XGpioPs_WritePin(&GpioInstance, pin, 0x0);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets a GPIO pin.
*
* @param	bank The GPIO bank to address
* @param        pin The GPIO pin to address
*
* @return	- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		None.
*
****************************************************************************/
int GpioSet(unsigned bank, unsigned pin)
{
	/*
	 * Set the Mode as Normal Mode, set the direction for all signals to be
	 * outputs and Enable the Output enable for the Pin.
	 */
	XGpioPs_SetDirection(&GpioInstance, bank, 1 << pin);
	XGpioPs_SetOutputEnable(&GpioInstance, bank, 1 << pin);

        /*
         * Set the GPIO Output to High.
         */
        XGpioPs_WritePin(&GpioInstance, pin, 0x1);

	return XST_SUCCESS;
}
