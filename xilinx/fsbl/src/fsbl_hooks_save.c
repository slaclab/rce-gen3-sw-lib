/******************************************************************************
*
* (c) Copyright 2011-2012 Xilinx, Inc. All rights reserved.
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
*******************************************************************************/

/*****************************************************************************
*
* @file fsbl_hooks.c
*
* This file provides functions that serve as user hooks.  The user can add the
* additional functionality required into these routines.  This would help retain
* the normal FSBL flow unchanged.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 3.00a np   08/03/12 Initial release
* </pre>
*
* @note
*
******************************************************************************/


#include "fsbl.h"
#include "xstatus.h"
#include "gpio_utils.h"
#include "i2c_utils.h"


/************************** Function Prototypes ******************************/



/******************************************************************************
* This function is the hook which will be called  before the bitstream download.
* The user can add all the customized code required to be executed before the
* bitstream download to this routine.
*
* @param None
*
* @return
*		- XST_SUCCESS to indicate success
*		- XST_FAILURE.to indicate failure
*
****************************************************************************/
u32 FsblHookBeforeBitstreamDload(void)
{
	u32 Status;

	Status = XST_SUCCESS;

	/* User logic to be added here. Errors to be stored in the status variable
	 * and returned */
	fsbl_printf(DEBUG_INFO,"In FsblHookBeforeBitstreamDload function \r\n");
	return (Status);
} /* End of FsblHookBeforeBitstreamDload */

/******************************************************************************
* This function is the hook which will be called  after the bitstream download.
* The user can add all the customized code required to be executed after the
* bitstream download to this routine.
*
* @param None
*
* @return
*		- XST_SUCCESS to indicate success
*		- XST_FAILURE.to indicate failure
*
****************************************************************************/
u32 FsblHookAfterBitstreamDload(void)
{
	u32 Status;
	XIicPs zc702_IicPs;
	XIicPs fmc_IicPs;
	XGpioPs GpioPs;
	struct fru_area_board board_info;

	/* User logic to be added here. Errors to be stored in the status variable
	 * and returned */
	fsbl_printf(DEBUG_INFO," In FsblHookAfterBitstreamDload function \r\n");

	/* Initialize PS I2C0 */
	fsbl_printf(DEBUG_INFO, "Initialize PS I2C0\n\r");
	Status = iic_init(&zc702_IicPs, ZC702_IIC_DEVICE_ID, ZC702_IIC_SCLK_RATE);
	if (Status == XST_FAILURE){
		/* Error Handling here */
		OutputStatus(FSBL_AFTER_BSTREAM_HOOK_FAIL);
		FsblFallback();
	}

	/* Initialize ADV7511 HDMI */
	fsbl_printf(DEBUG_INFO, "Initialize HDMI OUT\n\r");
	Status = zc702_hdmi_init(&zc702_IicPs);
	if (Status == XST_FAILURE){
		fsbl_printf(DEBUG_GENERAL, "No monitor detected on HDMI OUT\r\n");
	}

#ifdef USRCLK_SUPPORT
	/* Initialize SI570 Clock Synthesizer */
	fsbl_printf(DEBUG_INFO, "Initialize clock synthesizer\n\r");
	zc702_usrclk_init(&zc702_IicPs, VIDEO_RESOLUTION_HD1080P);
#endif

	/* Initialize PS I2C1 */
	fsbl_printf(DEBUG_INFO, "Initialize PS I2C1\n\r");
	Status = iic_init(&fmc_IicPs, FMC_IIC_DEVICE_ID, FMC_IIC_SCLK_RATE);
	if (Status == XST_FAILURE){
		/* Error Handling here */
		OutputStatus(FSBL_AFTER_BSTREAM_HOOK_FAIL);
		FsblFallback();
	}

	/* Initialize PS GPIO */
	fsbl_printf(DEBUG_INFO, "Initialize PS GPIO\n\r");
	Status = init_GpioPs(&GpioPs);
	if (Status == XST_FAILURE){
		/* Error Handling here */
		OutputStatus(FSBL_AFTER_BSTREAM_HOOK_FAIL);
		FsblFallback();
	}

	/* Initialize FMC card on FMC2 */
	Status = zc702_fmc_ipmi_detect(&zc702_IicPs, ZC702_FMC_SLOT2, &board_info);
	if ( Status != XST_SUCCESS ) {
		fsbl_printf(DEBUG_GENERAL, "No FMC card detected on FMC2\n\r");
	} else {
		if (strcmp((char *) board_info.mfg, "Avnet") == 0 && strcmp((char *) board_info.prod, "FMC-IMAGEON") == 0) {
			fsbl_printf(DEBUG_GENERAL, "%s %s detected on FMC2\n\r", board_info.mfg, board_info.prod);
			// initialization for ADV7511
			fsbl_printf(DEBUG_INFO, "Initialize HDMI IN\n\r");
//			debug_xil_printf("Reset FMC-IMAGEON IIC MUX\n\r");
			fmc_iic_mux_reset(&GpioPs);
//			debug_xil_printf("Initialize FMC-IMAGEON I/O Expander\n\r");
			fmc_imageon_io_exp_init(&fmc_IicPs);
//			debug_xil_printf("Initialize FMC-IMAGEON ADV7611\n\r");
			fmc_imageon_hdmii_init(&fmc_IicPs);
		}
//		else if (strcmp((char *) board_info.mfg, "Avnet") == 0 && strcmp((char *) board_info.prod, "FMC-DVI/DP") == 0) {
//			fsbl_printf(DEBUG_GENERAL, "%s %s detected on FMC2\n\r", board_info.mfg, board_info.prod);
//			// no initialization required for TFP403
//		}
		else {
			fsbl_printf(DEBUG_GENERAL, "Unsupported FMC card detected on FMC2 (%s %s)\n\r", board_info.mfg, board_info.prod);
		}
	}

	return XST_SUCCESS;
} /* End of FsblHookAfterBitstreamDload */

/******************************************************************************
* This function is the hook which will be called  before the FSBL does a handoff
* to the application. The user can add all the customized code required to be
* executed before the handoff to this routine.
*
* @param None
*
* @return
*		- XST_SUCCESS to indicate success
*		- XST_FAILURE.to indicate failure
*
****************************************************************************/
u32 FsblHookBeforeHandoff(void)
{
	u32 Status;

	Status = XST_SUCCESS;

	/* User logic to be added here. Errors to be stored in the status variable
	 * and returned */
	fsbl_printf(DEBUG_INFO,"In FsblHookBeforeHandoff function \r\n");
	return (Status);
} /* End of FsblHookBeforeHandoff */




