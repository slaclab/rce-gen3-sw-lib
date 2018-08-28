/******************************************************************************
*
* (c) Copyright 2010-12 Xilinx, Inc. All rights reserved.
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
//----------------------------------------------------------------
//
// Design Name:         I2C Utilities
// Target Devices:      Zynq-7
//
// Tool versions:       ISE 14.1
//
// Description:         I2C Utilities.
//                      - I2C Multiplexer
//                      - HDMI
//						- Clock Synthesizer
//
//----------------------------------------------------------------

#ifndef __ZC702_I2C_UTILS_H__
#define __ZC702_I2C_UTILS_H__


#include "xiicps.h"


// PS I2C0 Init Parameters -> ZC702 via MIO
#define ZC702_IIC_DEVICE_ID  XPAR_XIICPS_0_DEVICE_ID
#define ZC702_IIC_SCLK_RATE  100000


// PS I2C1 Init Parameters -> FMC via EMIO
#define FMC_IIC_DEVICE_ID  XPAR_XIICPS_1_DEVICE_ID
#define FMC_IIC_SCLK_RATE  100000


// FMC slots
#define ZC702_FMC_SLOT1 1
#define ZC702_FMC_SLOT2 2


// ADV7511 video output format
#define RGB444   0
#define YCRCB422 1
#define ADV7511_VIDEO_OUT_FORMAT  RGB444


#ifdef USRCLK_SUPPORT
// Supported Video Resolutions
#define VIDEO_RESOLUTION_VGA      0
#define VIDEO_RESOLUTION_SVGA     1
#define VIDEO_RESOLUTION_XGA      2
#define VIDEO_RESOLUTION_HD720P   3
#define VIDEO_RESOLUTION_SXGA     4
#define VIDEO_RESOLUTION_WSXGA    5
#define VIDEO_RESOLUTION_HD1080P  6
#endif


// FRU Board Info Area
struct fru_area_board {
	u8  area_ver;
	u32 mfg_date_time;
	u8  lang;
	unsigned char * mfg;
	unsigned char * prod;
	unsigned char * serial;
	unsigned char * part;
	unsigned char * fru;
	u16 area_len;
	u8  area_data[120]; // for now, fixed length
};


// I2C Init Function
int iic_init( XIicPs *IicPs, u16 DeviceId, u32 ClkRate );

// ZC702 HDMIO Functions
int zc702_hdmi_init( XIicPs *IicPs );

// FMC IPMI Functions
int zc702_fmc_ipmi_detect( XIicPs *IicPs, u8 FmcSlot, struct fru_area_board *board_info );

// FMC IMAGEON HDMII Functions
void fmc_imageon_io_exp_init( XIicPs *IicPs );
void fmc_imageon_hdmii_init( XIicPs *IicPs );

// USRCLK Functions
#ifdef USRCLK_SUPPORT
void zc702_usrclk_init( XIicPs *IicPs, u8 Resolution );
#endif


#endif // __ZC702_I2C_UTILS_H__
