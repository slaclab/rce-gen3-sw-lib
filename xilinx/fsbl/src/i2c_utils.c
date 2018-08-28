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

#include <stdlib.h>

#include "xil_printf.h"
#include "xstatus.h"
#include "i2c_utils.h"


#ifdef IIC_DEBUG
#define  iic_debug_xil_printf                       xil_printf
#else
#define  iic_debug_xil_printf(msg, args...) do {  } while (0)
#endif


// FMC IMAGEON Mux Selections
#define FMC_IMAGEON_I2C_SELECT_DDCEDID    0x01
#define FMC_IMAGEON_I2C_SELECT_HDMI_OUT   0x02
#define FMC_IMAGEON_I2C_SELECT_HDMI_IN    0x04
#define FMC_IMAGEON_I2C_SELECT_IO_EXP     0x08
#define FMC_IMAGEON_I2C_SELECT_VID_CLK    0x08

// FMC IMAGEON I2C Addresses
#define FMC_IMAGEON_I2C_MUX_ADDR   0x70 // (PCA9546A)
#define FMC_IMAGEON_DDCEDID_ADDR   0x50
#define FMC_IMAGEON_HDMI_OUT_ADDR  0x39 // (ADV7511)
#define FMC_IMAGEON_HDMI_IN_ADDR   0x4C // (ADV7611)
#define FMC_IMAGEON_IO_EXP_ADDR    0x20 // (PCA9534)
#define FMC_IMAGEON_VID_CLK_ADDR   0x65 // (CDCE913)

// FMC IMAGEON Programmable ADV7611 I2C addresses
#define FMC_IMAGEON_ADV7611_IO_ADDR          FMC_IMAGEON_HDMI_IN_ADDR
#define FMC_IMAGEON_ADV7611_CEC_ADDR         0x40
#define FMC_IMAGEON_ADV7611_INFOFRAME_ADDR   0x35
#define FMC_IMAGEON_ADV7611_DPLL_ADDR        0x26
#define FMC_IMAGEON_ADV7611_KSV_ADDR         0x32
#define FMC_IMAGEON_ADV7611_EDID_ADDR        0x36
#define FMC_IMAGEON_ADV7611_HDMI_ADDR        0x34
#define FMC_IMAGEON_ADV7611_CP_ADDR          0x22

// ZC702 Mux Selections
#define ZC702_I2C_SELECT_USRCLK    0x01
#define ZC702_I2C_SELECT_HDMI      0x02
#define ZC702_I2C_SELECT_EEPROM    0x04
#define ZC702_I2C_SELECT_EXPANDER  0x08
#define ZC702_I2C_SELECT_RTC       0x10
#define ZC702_I2C_SELECT_FMC1      0x20
#define ZC702_I2C_SELECT_FMC2      0x40
#define ZC702_I2C_SELECT_PMBUS     0x80

// ZC702 I2C Addresses
#define ZC702_I2C_MUX_ADDR   0x74 // (PCA9458)
#define ZC702_USRCLK_ADDR    0x5D // (SI570)
#define ZC702_HDMI_ADDR      0x39 // (ADV7511)
#define ZC702_PMBUS0_ADDR    0x34 // (UCD9248)
#define ZC702_PMBUS1_ADDR    0x35 // (UCD9248)
#define ZC702_PMBUS2_ADDR    0x36 // (UCD9248)
#define ZC702_FMC_IPMI_ADDR  0x50 // (24LC02)


// Function Prototypes
static int iic_read1( XIicPs *IicPs, u8 Address, u8 *Data );
static int iic_read2( XIicPs *IicPs, u8 Address, u8 Register, u8 *Data, int ByteCount );


// I2C Config Struct
typedef struct {
	u8 Reg;
	u8 Data;
	u8 Init;
} ZC702_I2C_CONFIG;


// I2C Config Struct
typedef struct {
	u8 Addr;
	u8 Reg;
	u8 Data;
	u8 Init;
} ZC702_I2C_CONFIG2;


// Common FRU Header
struct fru_header {
	u8 version;
	struct	{
		u8 internal;
		u8 chassis;
		u8 board;
		u8 product;
		u8 multi;
	} offset;
	u8 pad;
	u8 checksum;
};


// EDID for ADV7611 HDMI receiver
static u8 edid_data[256] =
{
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
	0x06, 0xD4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x16, 0x01, 0x03, 0x81, 0x46, 0x27, 0x78,
	0x0A, 0x32, 0x30, 0xA1, 0x54, 0x52, 0x9E, 0x26,
	0x0A, 0x49, 0x4B, 0xA3, 0x08, 0x00, 0x81, 0xC0,
	0x81, 0x00, 0x81, 0x0F, 0x81, 0x40, 0x81, 0x80,
	0x95, 0x00, 0xB3, 0x00, 0x01, 0x01, 0x02, 0x3A,
	0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C,
	0x45, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x1E,
	0xA9, 0x1A, 0x00, 0xA0, 0x50, 0x00, 0x16, 0x30,
	0x30, 0x20, 0x37, 0x00, 0xC4, 0x8E, 0x21, 0x00,
	0x00, 0x1A, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x46,
	0x4D, 0x43, 0x2D, 0x49, 0x4D, 0x41, 0x47, 0x45,
	0x4F, 0x4E, 0x0A, 0x20, 0x00, 0x00, 0x00, 0xFD,
	0x00, 0x38, 0x4B, 0x20, 0x44, 0x11, 0x00, 0x0A,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x54,
	0x02, 0x03, 0x1F, 0x71, 0x4B, 0x90, 0x03, 0x04,
	0x05, 0x12, 0x13, 0x14, 0x1F, 0x20, 0x07, 0x16,
	0x26, 0x15, 0x07, 0x50, 0x09, 0x07, 0x01, 0x67,
	0x03, 0x0C, 0x00, 0x10, 0x00, 0x00, 0x1E, 0x01,
	0x1D, 0x00, 0x72, 0x51, 0xD0, 0x1E, 0x20, 0x6E,
	0x28, 0x55, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00,
	0x1E, 0x01, 0x1D, 0x80, 0x18, 0x71, 0x1C, 0x16,
	0x20, 0x58, 0x2C, 0x25, 0x00, 0xC4, 0x8E, 0x21,
	0x00, 0x00, 0x9E, 0x8C, 0x0A, 0xD0, 0x8A, 0x20,
	0xE0, 0x2D, 0x10, 0x10, 0x3E, 0x96, 0x00, 0xC4,
	0x8E, 0x21, 0x00, 0x00, 0x18, 0x01, 0x1D, 0x80,
	0x3E, 0x73, 0x38, 0x2D, 0x40, 0x7E, 0x2C, 0x45,
	0x80, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x1E, 0x1A,
	0x36, 0x80, 0xA0, 0x70, 0x38, 0x1F, 0x40, 0x30,
	0x20, 0x25, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00,
	0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
};

#if (ADV7511_VIDEO_OUT_FORMAT == RGB444)

/*
 * The video input format of the ADV7511 is set to YCbCr, 16-bit, 4:2:2,
 * ID 1 (separate syncs), Style 3. The video output format is set to
 * RGB, 24-bit, 4:4:4, DVI mode.
 *
 * CSC coefficients (registers 0x18 - 0x2F) are set to convert HDTV YCrCb
 * (0 to 255 or full range) to RGB (0 to 255 or full range).
 */
#define ZC702_HDMI_CONFIG_LEN  40
ZC702_I2C_CONFIG zc702_hdmi_config[ZC702_HDMI_CONFIG_LEN] =
{
    {0x41, 0x00, 0x10}, // Power Down Control
                        //    R0x41[  6] = PowerDown = 0 (power-up)
    {0xD6, 0x00, 0xC0}, // HPD Control
                        //    R0xD6[7:6] = HPD Control = 11 (always high)
    {0x15, 0x00, 0x01}, // Input YCbCr 4:2:2 with separate syncs
    {0x16, 0x00, 0x3C}, // Output format 444, Input Color Depth = 8
                        //    R0x16[  7] = Output Video Format = 0 (444)
                        //    R0x16[5:4] = Input Video Color Depth = 11 (8 bits/color)
                        //    R0x16[3:2] = Input Video Style = 11 (style 3)
                        //    R0x16[  1] = DDR Input Edge = 0 (falling edge)
                        //    R0x16[  0] = Output Color Space = 0 (RGB)
    {0x18, 0x00, 0xAB}, // Color Space Conversion
                        //    R0x18[  7] = CSC enable = 1 (CSC enabled)
                        //    R0x18[6:5] = CSC Scaling Factor = 01 (+/- 2.0, -8192 - 8190)
                        //    R0x18[4:0] = CSC coefficient A1[12:8]
    {0x19, 0x00, 0x37}, //    R0x19[7:0] = CSC coefficient A1[ 7:0]
    {0x1A, 0x00, 0x08}, //    R0x1A[  5] = CSC coefficient update
                        //    R0x1A[4:0] = CSC coefficient A2[12:8]
    {0x1B, 0x00, 0x00}, //    R0x1B[7:0] = CSC coefficient A2[ 7:0]
    {0x1C, 0x00, 0x00}, //    R0x1C[4:0] = CSC coefficient A3[12:8]
    {0x1D, 0x00, 0x00}, //    R0x1D[7:0] = CSC coefficient A3[ 7:0]
    {0x1E, 0x00, 0x1A}, //    R0x1E[4:0] = CSC coefficient A4[12:8]
    {0x1F, 0x00, 0x86}, //    R0x1F[7:0] = CSC coefficient A4[ 7:0]
    {0x20, 0x00, 0x1A}, //    R0x20[4:0] = CSC coefficient B1[12:8]
    {0x21, 0x00, 0x49}, //    R0x21[7:0] = CSC coefficient B1[ 7:0]
    {0x22, 0x00, 0x08}, //    R0x22[4:0] = CSC coefficient B2[12:8]
    {0x23, 0x00, 0x00}, //    R0x23[7:0] = CSC coefficient B2[ 7:0]
    {0x24, 0x00, 0x1D}, //    R0x24[4:0] = CSC coefficient B3[12:8]
    {0x25, 0x00, 0x3F}, //    R0x25[7:0] = CSC coefficient B3[ 7:0]
    {0x26, 0x00, 0x04}, //    R0x26[4:0] = CSC coefficient B4[12:8]
    {0x27, 0x00, 0x22}, //    R0x27[7:0] = CSC coefficient B4[ 7:0]
    {0x28, 0x00, 0x00}, //    R0x28[4:0] = CSC coefficient C1[12:8]
    {0x29, 0x00, 0x00}, //    R0x29[7:0] = CSC coefficient C1[ 7:0]
    {0x2A, 0x00, 0x08}, //    R0x2A[4:0] = CSC coefficient C2[12:8]
    {0x2B, 0x00, 0x00}, //    R0x2B[7:0] = CSC coefficient C2[ 7:0]
    {0x2C, 0x00, 0x0E}, //    R0x2C[4:0] = CSC coefficient C3[12:8]
    {0x2D, 0x00, 0x2D}, //    R0x2D[7:0] = CSC coefficient C3[ 7:0]
    {0x2E, 0x00, 0x19}, //    R0x2E[4:0] = CSC coefficient C4[12:8]
    {0x2F, 0x00, 0x14}, //    R0x2F[7:0] = CSC coefficient C4[ 7:0]
    {0x48, 0x00, 0x08}, // Video Input Justification
                        //    R0x48[4:3] = Video Input Justification = 01 (right justified)
    {0x55, 0x00, 0x00}, // Set RGB in AVinfo Frame
                        //    R0x55[6:5] = Output Format = 00 (RGB)
    {0x56, 0x00, 0x28}, // Aspect Ratio
                        //    R0x56[5:4] = Picture Aspect Ratio = 10 (16:9)
                        //    R0x56[3:0] = Active Format Aspect Ratio = 1000 (Same as Aspect Ratio)
    {0x98, 0x00, 0x03}, // ADI Recommended Write
    {0x9A, 0x00, 0xE0}, // ADI Recommended Write
    {0x9C, 0x00, 0x30}, // PLL Filter R1 Value
    {0x9D, 0x00, 0x61}, // Set clock divide
    {0xA2, 0x00, 0xA4}, // ADI Recommended Write
    {0xA3, 0x00, 0xA4}, // ADI Recommended Write
    {0xAF, 0x00, 0x04}, // HDMI/DVI Modes
                        //    R0xAF[  7] = HDCP Enable = 0 (HDCP disabled)
                        //    R0xAF[  4] = Frame Encryption = 0 (Current frame NOT HDCP encrypted)
                        //    R0xAF[3:2] = 01 (fixed)
                        //    R0xAF[  1] = HDMI/DVI Mode Select = 0 (DVI Mode)
    {0xE0, 0x00, 0xD0}, // Must be set to 0xD0 for proper operation
    {0xF9, 0x00, 0x00}  // Fixed I2C Address (This should be set to a non-conflicting I2C address)
};

#else // YCBCR422

/*
 * The video input format of the ADV7511 is set to YCbCr, 16-bit, 4:2:2,
 * ID 1 (separate syncs), Style 3. The video output format is set to
 * YCbCr, 16-bit, 4:2:2, HDMI mode.
 */
#define ZC702_HDMI_CONFIG_LEN  16
ZC702_I2C_CONFIG zc702_hdmi_config[ZC702_HDMI_CONFIG_LEN] =
{
    {0x41, 0x00, 0x10}, // Power Down Control
                        //    R0x41[  6] = PowerDown = 0 (power-up)
    {0xD6, 0x00, 0xC0}, // HPD Control
                        //    R0xD6[7:6] = HPD Control = 11 (always high)
    {0x15, 0x00, 0x01}, // Input YCbCr 4:2:2 with separate syncs
    {0x16, 0x00, 0xBD}, // Output format 4:2:2, Input Color Depth = 8
                        //    R0x16[  7] = Output Video Format = 1 (4:2:2)
                        //    R0x16[5:4] = Input Video Color Depth = 11 (8 bits/color)
                        //    R0x16[3:2] = Input Video Style = 11 (style 3)
                        //    R0x16[  1] = DDR Input Edge = 0 (falling edge)
                        //    R0x16[  0] = Output Color Space = 1 (YCbCr)
    {0x48, 0x00, 0x08}, // Video Input Justification
                        //    R0x48[4:3] = Video Input Justification = 01 (right justified)
    {0x55, 0x00, 0x20}, // Set RGB in AVinfo Frame
                        //    R0x55[6:5] = Output Format = 01 (YCbCr)
    {0x56, 0x00, 0x28}, // Aspect Ratio
                        //    R0x56[5:4] = Picture Aspect Ratio = 10 (16:9)
                        //    R0x56[3:0] = Active Format Aspect Ratio = 1000 (Same as Aspect Ratio)
    {0x98, 0x00, 0x03}, // ADI Recommended Write
    {0x9A, 0x00, 0xE0}, // ADI Recommended Write
    {0x9C, 0x00, 0x30}, // PLL Filter R1 Value
    {0x9D, 0x00, 0x61}, // Set clock divide
    {0xA2, 0x00, 0xA4}, // ADI Recommended Write
    {0xA3, 0x00, 0xA4}, // ADI Recommended Write
    {0xAF, 0x00, 0x06}, // HDMI/DVI Modes
                        //    R0xAF[  7] = HDCP Enable = 0 (HDCP disabled)
                        //    R0xAF[  4] = Frame Encryption = 0 (Current frame NOT HDCP encrypted)
                        //    R0xAF[3:2] = 01 (fixed)
                        //    R0xAF[  1] = HDMI/DVI Mode Select = 2 (HDMI Mode)
    {0xE0, 0x00, 0xD0}, // Must be set to 0xD0 for proper operation
    {0xF9, 0x00, 0x00}  // Fixed I2C Address (This should be set to a non-conflicting I2C address)
};

#endif


#define FMC_IMAGEON_HDMI_IN_MAPPING_LEN  7
static ZC702_I2C_CONFIG2 fmc_imageon_hdmi_in_mapping[FMC_IMAGEON_HDMI_IN_MAPPING_LEN] =
{
	{FMC_IMAGEON_ADV7611_IO_ADDR, 0xF4, 0x00, FMC_IMAGEON_ADV7611_CEC_ADDR<<1},       // CEC
	{FMC_IMAGEON_ADV7611_IO_ADDR, 0xF5, 0x00, FMC_IMAGEON_ADV7611_INFOFRAME_ADDR<<1}, // INFOFRAME
	{FMC_IMAGEON_ADV7611_IO_ADDR, 0xF8, 0x00, FMC_IMAGEON_ADV7611_DPLL_ADDR<<1},      // DPLL
	{FMC_IMAGEON_ADV7611_IO_ADDR, 0xF9, 0x00, FMC_IMAGEON_ADV7611_KSV_ADDR<<1},       // KSV (Repeater)
	{FMC_IMAGEON_ADV7611_IO_ADDR, 0xFA, 0x00, FMC_IMAGEON_ADV7611_EDID_ADDR<<1},      // EDID
	{FMC_IMAGEON_ADV7611_IO_ADDR, 0xFB, 0x00, FMC_IMAGEON_ADV7611_HDMI_ADDR<<1},      // HDMI
	{FMC_IMAGEON_ADV7611_IO_ADDR, 0xFD, 0x00, FMC_IMAGEON_ADV7611_CP_ADDR<<1}         // CP
};


#define FMC_IMAGEON_HDMI_IN_EDID_POST_LEN  5
static ZC702_I2C_CONFIG2 fmc_imageon_hdmi_in_edid_post[FMC_IMAGEON_HDMI_IN_EDID_POST_LEN] =
{
	{FMC_IMAGEON_ADV7611_KSV_ADDR, 0x77, 0x00, 0x00}, // Set the Most Significant Bit of the SPA location to 0
	{FMC_IMAGEON_ADV7611_KSV_ADDR, 0x52, 0x00, 0x20}, // Set the SPA for port B.
	{FMC_IMAGEON_ADV7611_KSV_ADDR, 0x53, 0x00, 0x00}, // Set the SPA for port B.
	{FMC_IMAGEON_ADV7611_KSV_ADDR, 0x70, 0x00, 0x9E}, // Set the Least Significant Byte of the SPA location
	{FMC_IMAGEON_ADV7611_KSV_ADDR, 0x74, 0x00, 0x03}  // Enable the Internal EDID for Ports
};


#define FMC_IMAGEON_HDMI_IN_CONFIG_LEN  42
static ZC702_I2C_CONFIG2 fmc_imageon_hdmi_in_config[FMC_IMAGEON_HDMI_IN_CONFIG_LEN] =
{
	{FMC_IMAGEON_ADV7611_IO_ADDR,   0x01, 0x00, 0x06}, // Prim_Mode = 110b HDMI-GR
	{FMC_IMAGEON_ADV7611_IO_ADDR,   0x02, 0x00, 0xF5}, // Auto CSC, YCrCb out, Set op_656 bit
	{FMC_IMAGEON_ADV7611_IO_ADDR,   0x03, 0x00, 0x80}, // 16-Bit SDR ITU-R BT.656 4:2:2 Mode 0
	{FMC_IMAGEON_ADV7611_IO_ADDR,   0x04, 0x00, 0x62}, // OP_CH_SEL[2:0] = 011b - (P[15:8] Y, P[7:0] CrCb), XTAL_FREQ[1:0] = 01b (28.63636 MHz)
	{FMC_IMAGEON_ADV7611_IO_ADDR,   0x05, 0x00, 0x2C}, // AV Codes on

	{FMC_IMAGEON_ADV7611_CP_ADDR,   0x7B, 0x00, 0x05},

	{FMC_IMAGEON_ADV7611_IO_ADDR, 0x0B, 0x00, 0x44}, // Power up part
	{FMC_IMAGEON_ADV7611_IO_ADDR, 0x0C, 0x00, 0x42}, // Power up part
	{FMC_IMAGEON_ADV7611_IO_ADDR, 0x14, 0x00, 0x7F}, // Max Drive Strength
	{FMC_IMAGEON_ADV7611_IO_ADDR, 0x15, 0x00, 0x80}, // Disable Tristate of Pins
    {FMC_IMAGEON_ADV7611_IO_ADDR, 0x06, 0x00, 0xA1}, // LLC polarity (INV_LLC_POL = 1)
    {FMC_IMAGEON_ADV7611_IO_ADDR, 0x19, 0x00, 0x80}, // LLC DLL phase (delay = 0)
	{FMC_IMAGEON_ADV7611_IO_ADDR, 0x33, 0x00, 0x40}, // LLC DLL enable

	{FMC_IMAGEON_ADV7611_CP_ADDR  , 0xBA, 0x00, 0x01}, // Set HDMI FreeRun

	{FMC_IMAGEON_ADV7611_KSV_ADDR , 0x40, 0x00, 0x81}, // Disable HDCP 1.1 features

	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0x9B, 0x00, 0x03}, // ADI recommended setting
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0xC1, 0x00, 0x01}, // ADI recommended setting
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0xC2, 0x00, 0x01}, // ADI recommended setting
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0xC3, 0x00, 0x01}, // ADI recommended setting
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0xC4, 0x00, 0x01}, // ADI recommended setting
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0xC5, 0x00, 0x01}, // ADI recommended setting
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0xC6, 0x00, 0x01}, // ADI recommended setting
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0xC7, 0x00, 0x01}, // ADI recommended setting
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0xC8, 0x00, 0x01}, // ADI recommended setting
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0xC9, 0x00, 0x01}, // ADI recommended setting
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0xCA, 0x00, 0x01}, // ADI recommended setting
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0xCB, 0x00, 0x01}, // ADI recommended setting
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0xCC, 0x00, 0x01}, // ADI recommended setting
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0x00, 0x00, 0x08}, // Set HDMI Input Port A  (BG_MEAS_PORT_SEL = 001b)
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0x02, 0x00, 0x03}, // Enable Ports A & B in background mode
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0x83, 0x00, 0xFC}, // Enable clock terminators for port A & B
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0x6F, 0x00, 0x0C}, // ADI recommended setting
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0x85, 0x00, 0x1F}, // ADI recommended setting
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0x87, 0x00, 0x70}, // ADI recommended setting
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0x8D, 0x00, 0x04}, // LFG Port A
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0x8E, 0x00, 0x1E}, // HFG Port A
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0x1A, 0x00, 0x8A}, // Unmute audio
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0x57, 0x00, 0xDA}, // ADI recommended setting
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0x58, 0x00, 0x01}, // ADI recommended setting
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0x75, 0x00, 0x10}, // DDC drive strength
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0x90, 0x00, 0x04}, // LFG Port B
	{FMC_IMAGEON_ADV7611_HDMI_ADDR, 0x91, 0x00, 0x1E}  // HFG Port B
};


// Turn on/off SI570 clock synthesizer support
#ifdef USRCLK_SUPPORT
#define ZC702_USRCLK_CONFIG_LEN  7
ZC702_I2C_CONFIG zc702_usrclk_config_HD1080p[ZC702_USRCLK_CONFIG_LEN] =
{
	{0x07, 0x00, 0xA0},
	{0x08, 0x00, 0xC2},
	{0x09, 0x00, 0xEC},
	{0x0A, 0x00, 0x70},
	{0x0B, 0x00, 0x22},
	{0x0C, 0x00, 0xC7},
	{0x87, 0x00, 0x40}
};

ZC702_I2C_CONFIG zc702_usrclk_config_WSXGA[ZC702_USRCLK_CONFIG_LEN] =
{
	{0x07, 0x00, 0xA0},
	{0x08, 0x00, 0xC2},
	{0x09, 0x00, 0xE1},
	{0x0A, 0x00, 0xBA},
	{0x0B, 0x00, 0x62},
	{0x0C, 0x00, 0x11},
	{0x87, 0x00, 0x40}
};

ZC702_I2C_CONFIG zc702_usrclk_config_SXGA[ZC702_USRCLK_CONFIG_LEN] =
{
	{0x07, 0x00, 0x22},
	{0x08, 0x00, 0x43},
	{0x09, 0x00, 0x01},
	{0x0A, 0x00, 0xFF},
	{0x0B, 0x00, 0x7B},
	{0x0C, 0x00, 0x38},
	{0x87, 0x00, 0x40}
};

ZC702_I2C_CONFIG zc702_usrclk_config_HD720p[ZC702_USRCLK_CONFIG_LEN] =
{
	{0x07, 0x00, 0x62},
	{0x08, 0x00, 0x42},
	{0x09, 0x00, 0xD7},
	{0x0A, 0x00, 0xA5},
	{0x0B, 0x00, 0xE8},
	{0x0C, 0x00, 0xEC},
	{0x87, 0x00, 0x40}
};

ZC702_I2C_CONFIG zc702_usrclk_config_XGA[ZC702_USRCLK_CONFIG_LEN] =
{
	{0x07, 0x00, 0x23},
	{0x08, 0x00, 0xC2},
	{0x09, 0x00, 0xD7},
	{0x0A, 0x00, 0xFF},
	{0x0B, 0x00, 0x82},
	{0x0C, 0x00, 0x76},
	{0x87, 0x00, 0x40}
};

ZC702_I2C_CONFIG zc702_usrclk_config_SVGA[ZC702_USRCLK_CONFIG_LEN] =
{
	{0x07, 0x00, 0xE2},
	{0x08, 0x00, 0xC2},
	{0x09, 0x00, 0xE3},
	{0x0A, 0x00, 0x32},
	{0x0B, 0x00, 0xB3},
	{0x0C, 0x00, 0xBB},
	{0x87, 0x00, 0x40}
};

ZC702_I2C_CONFIG zc702_usrclk_config_VGA[ZC702_USRCLK_CONFIG_LEN] =
{
	{0x07, 0x00, 0x66},
	{0x08, 0x00, 0xC2},
	{0x09, 0x00, 0xB2},
	{0x0A, 0x00, 0xCC},
	{0x0B, 0x00, 0xD8},
	{0x0C, 0x00, 0xC0},
	{0x87, 0x00, 0x40}
};
#endif

int iic_init( XIicPs *IicPs, u16 DeviceId, u32 ClkRate )
{
	int Status;
	XIicPs_Config *IicPs_Config;

	/*
	 * Initialize the IIC driver.
	 */
	IicPs_Config = XIicPs_LookupConfig(DeviceId);
	if (IicPs_Config == NULL) {
		iic_debug_xil_printf("No XIicPs instance found for ID %d\n\r", DeviceId);
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(IicPs, IicPs_Config, IicPs_Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		iic_debug_xil_printf("XIicPs Initialization failed for ID %d\n\r", DeviceId);
		return XST_FAILURE;
	}

	/*
	 * Set the IIC serial clock rate.
	 */
	Status = XIicPs_SetSClk(IicPs, ClkRate);
	if (Status != XST_SUCCESS) {
		iic_debug_xil_printf("Setting XIicPs clock rate failed for ID %d\n\r", DeviceId);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}


static int iic_write1( XIicPs *IicPs, u8 Address, u8 Data )
{
	int Status;

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(IicPs)) {
		/* NOP */
	}

	/*
	 * Send the buffer using the IIC and check for errors.
	 */
	Status = XIicPs_MasterSendPolled(IicPs, &Data, 1, Address);
	if (Status != XST_SUCCESS) {
		iic_debug_xil_printf("XIicPs_MasterSendPolled error!\n\r");
		return XST_FAILURE;
	}

#ifdef FSBL_DEBUG
	u8 read_data;
	iic_read1(IicPs, Address, &read_data);
#endif

	return XST_SUCCESS;
}


static int iic_write2( XIicPs *IicPs, u8 Address, u8 Register, u8 *Data, int ByteCount )
{
	u8 *WriteBuffer = (u8 *) malloc((ByteCount+1) * sizeof(u8));
	int Status;
	int i;

	/*
	 * A temporary write buffer must be used which contains both the address
	 * and the data to be written, put the address in first
	 */
	WriteBuffer[0] = Register;
	for (i = 0; i < ByteCount; i++) {
		WriteBuffer[i+1] = Data[i];
	}

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(IicPs)) {
		/* NOP */
	}

	/*
	 * Send the buffer using the IIC and check for errors.
	 */
	Status = XIicPs_MasterSendPolled(IicPs, WriteBuffer, ByteCount+1, Address);
	free(WriteBuffer);
	if (Status != XST_SUCCESS) {
		iic_debug_xil_printf("XIicPs_MasterSendPolled error!\n\r");
		return XST_FAILURE;
	}

#ifdef FSBL_DEBUG
	u8 *read_data = (u8 *) malloc((ByteCount+1) * sizeof(u8));
	iic_read2(IicPs, Address, Register, read_data, ByteCount);
	free(read_data);
#endif

	return XST_SUCCESS;
}


static void iic_writex( XIicPs *IicPs, u8 Address, ZC702_I2C_CONFIG Config[], u32 Length )
{
   int i;

   for ( i = 0; i < Length; i++ )
   {
      iic_write2(IicPs, Address, Config[i].Reg, &Config[i].Init, 1);
   }
}


static void iic_writex2( XIicPs *IicPs, ZC702_I2C_CONFIG2 Config[], u32 Length )
{
   int i;

   for ( i = 0; i < Length; i++ )
   {
      iic_write2(IicPs, Config[i].Addr, Config[i].Reg, &Config[i].Init, 1);
   }
}


static int iic_read1( XIicPs *IicPs, u8 Address, u8 *Data )
{
	int Status;

	/*
	* Wait until bus is idle to start another transfer.
	*/
	while (XIicPs_BusIsBusy(IicPs)) {
		/* NOP */
	}

	/*
	* Receive the data.
	*/
	Status = XIicPs_MasterRecvPolled(IicPs, Data, 1, Address);
	if (Status != XST_SUCCESS) {
		iic_debug_xil_printf("XIicPs_MasterRecvPolled error!\n\r");
		return XST_FAILURE;
	}

	iic_debug_xil_printf("[iic_read1] 0x%02X=0x%02X\n\r", Address, *Data);

	return XST_SUCCESS;
}


static int iic_read2( XIicPs *IicPs, u8 Address, u8 Register, u8 *Data, int ByteCount )
{
	int Status;
	int i;

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(IicPs)) {
		/* NOP */
	}

	/*
	 * Set the IIC Repeated Start option.
	 */
	Status = XIicPs_SetOptions(IicPs, XIICPS_REP_START_OPTION);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Send the buffer using the IIC and check for errors.
	 */
	Status = XIicPs_MasterSendPolled(IicPs, &Register, 1, Address);
	if (Status != XST_SUCCESS) {
		iic_debug_xil_printf("XIicPs_MasterSendPolled error!\n\r");
		return XST_FAILURE;
	}

	/*
	 * Receive the data.
	 */
	Status = XIicPs_MasterRecvPolled(IicPs, Data, ByteCount, Address);
	if (Status != XST_SUCCESS) {
		iic_debug_xil_printf("XIicPs_MasterRecvPolled error!\n\r");
		return XST_FAILURE;
	}

	/*
	 * Clear the IIC Repeated Start option.
	 */
	Status = XIicPs_ClearOptions(IicPs, XIICPS_REP_START_OPTION);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	iic_debug_xil_printf("[iic_read2] 0x%02X(0x%02X)={ ", Address, Register);
	for (i=0; i < ByteCount; i++) {
		iic_debug_xil_printf("0x%02X ", Data[i]);
	}
	iic_debug_xil_printf("}\n\r");

	return XST_SUCCESS;
}


static void iic_readx( XIicPs *IicPs, u8 Address, ZC702_I2C_CONFIG Config[], u32 Length )
{
   int i;

   for ( i = 0; i < Length; i++ ) {
      iic_read2(IicPs, Address, Config[i].Reg, &Config[i].Data, 1);
   }
}


//static void iic_readx2( XIicPs *IicPs, ZC702_I2C_CONFIG2 Config[], u32 Length )
//{
//   int i;
//
//   for ( i = 0; i < Length; i++ ) {
//      iic_read2(IicPs, Config[i].Addr, Config[i].Reg, &Config[i].Data, 1);
//   }
//}


static void zc702_iic_mux( XIicPs *IicPs, u8 MuxSelect )
{
   iic_write1(IicPs, ZC702_I2C_MUX_ADDR, MuxSelect);
}


int zc702_hdmi_init( XIicPs *IicPs )
{
	u8 data = 0x00;
	u8 hpd_ctrl_mask = 0x40; // bit 6 = state of HPD

	// set IIC MUX
	zc702_iic_mux( IicPs, ZC702_I2C_SELECT_HDMI );

	// check HPD state
	iic_read2( IicPs, ZC702_HDMI_ADDR, 0x42, &data, 1);
	if((data & hpd_ctrl_mask) != hpd_ctrl_mask) {
		iic_debug_xil_printf("HPD state is 0x%02X!\r\n", data);
		return XST_FAILURE;
	}

	// write ADV7511 configuration
	iic_writex( IicPs, ZC702_HDMI_ADDR, zc702_hdmi_config, ZC702_HDMI_CONFIG_LEN );

	// read back video input and output configuration
	iic_readx( IicPs, ZC702_HDMI_ADDR, zc702_hdmi_config, ZC702_HDMI_CONFIG_LEN );

	return XST_SUCCESS;
}


static void fmc_imageon_iic_mux( XIicPs *IicPs, u8 MuxSelect )
{
   iic_write1(IicPs, FMC_IMAGEON_I2C_MUX_ADDR, MuxSelect);
}


void fmc_imageon_io_exp_init( XIicPs *IicPs )
{
	u8 reg_addr;
	u8 reg_data;

	fmc_imageon_iic_mux( IicPs, FMC_IMAGEON_I2C_SELECT_IO_EXP );

	// Configure direction of I/O Expander's Px GPIO pins
	reg_addr = 0x03;
	reg_data = 0xEA; // P0 => HDMII_RST#  = 0 (output)
					 // P1 => HDMII_INT1  = 1 (input)
					 // P2 => HDMII_HPD   = 0 (output)
					 // P3 => HDMIO_HPD   = 1 (input)
					 // P4 => HDMIO_PD    = 0 (output)
					 // P5 => unused      = 1 (input)
					 // P6 => unused      = 1 (input)
					 // P7 => unused      = 1 (input)

	iic_write2( IicPs, FMC_IMAGEON_IO_EXP_ADDR, reg_addr, &reg_data, 1 );

	// Configure output values of I/O Expander's Px GPIO pins
	reg_addr = 0x01;
	reg_data = 0x01; // P0 => HDMII_RST#  = 1 (output)
					 // P1 => HDMII_INT1  = x (input)
					 // P2 => HDMII_HPD   = 0 (output)
					 // P3 => HDMIO_HPD   = x (input)
					 // P4 => HDMIO_PD    = 0 (output)
					 // P5 => unused      = x (input)
					 // P6 => unused      = x (input)
					 // P7 => unused      = x (input)

	iic_write2( IicPs, FMC_IMAGEON_IO_EXP_ADDR, reg_addr, &reg_data, 1 );
}


static void fmc_imageon_hdmii_set_hpd( XIicPs *IicPs, u32 HotPlugStatus )
{
   u8 reg_addr;
   u8 reg_data;

   fmc_imageon_iic_mux( IicPs, FMC_IMAGEON_I2C_SELECT_IO_EXP );

   // Configure output values of I/O Expander's P2 GPIO pins
   reg_addr = 0x01;

   iic_read2( IicPs, FMC_IMAGEON_IO_EXP_ADDR, reg_addr, &reg_data, 1 );

   if ( HotPlugStatus ) {
      reg_data |=  0x04; // // P2 => HDMII_HPD   = 1 (output)
   } else {
      reg_data &= ~0x04; // // P2 => HDMII_HPD   = 0 (output)
   }

   iic_write2( IicPs, FMC_IMAGEON_IO_EXP_ADDR, reg_addr, &reg_data, 1 );
}


static void fmc_imageon_hdmii_set_rst( XIicPs *IicPs, u32 Reset )
{
   u8 reg_addr;
   u8 reg_data;

   fmc_imageon_iic_mux( IicPs, FMC_IMAGEON_I2C_SELECT_IO_EXP );

   // Configure output values of I/O Expander's P0 GPIO pins
   reg_addr = 0x01;

   iic_read2( IicPs, FMC_IMAGEON_IO_EXP_ADDR, reg_addr, &reg_data, 1 );

   if ( Reset ) {
      reg_data &= ~0x01; // // P0 => HDMII_RST#  = 0 (output)
   } else {
      reg_data |=  0x01; // // P0 => HDMII_RST#  = 1 (output)
   }

   iic_write2( IicPs, FMC_IMAGEON_IO_EXP_ADDR, reg_addr, &reg_data, 1 );
}


void fmc_imageon_hdmii_init( XIicPs *IicPs )
{
   u8 reg_addr;
   u8 reg_data;

	// Do not allow HDMI source do "see" the HDMI receiver until the EDID has been programmed
	fmc_imageon_hdmii_set_hpd( IicPs, 0 );
//	usleep(100000);

	fmc_imageon_hdmii_set_rst( IicPs, 0 );
//	usleep(100000);

	fmc_imageon_iic_mux( IicPs, FMC_IMAGEON_I2C_SELECT_HDMI_IN );

	iic_writex2( IicPs, fmc_imageon_hdmi_in_mapping, FMC_IMAGEON_HDMI_IN_MAPPING_LEN );
	//xil_printf("iic_writex2: I2C address mapping done \r\n");

	reg_addr = 0x77;
	reg_data = 0x00;
	iic_write2( IicPs, FMC_IMAGEON_ADV7611_KSV_ADDR, reg_addr, &reg_data, 1 );
	//xil_printf("iic_write2: EDID pre done \r\n");

	iic_write2( IicPs, FMC_IMAGEON_ADV7611_EDID_ADDR, 0x00, edid_data, sizeof(edid_data) );
	//xil_printf("iic_write2: EDID write done \r\n");

	iic_writex2( IicPs, fmc_imageon_hdmi_in_edid_post, FMC_IMAGEON_HDMI_IN_EDID_POST_LEN );
	//xil_printf("iic_writex2: EDID post done \r\n");

	// Now that the EDID has been programmed, allow HDMI source to "see" the HDMI receiver
	fmc_imageon_hdmii_set_hpd( IicPs, 1 );
//	usleep(100000);

	fmc_imageon_iic_mux( IicPs, FMC_IMAGEON_I2C_SELECT_HDMI_IN );

	iic_writex2( IicPs, fmc_imageon_hdmi_in_config, FMC_IMAGEON_HDMI_IN_CONFIG_LEN );
	//xil_printf("iic_writex2: HDMI config done \r\n");
}


static int fmc_ipmi_get_common_info( XIicPs *IicPs, u8 iic_address, struct fru_header *common_info )
{
	u8 data[8];
	u8 checksum = 0;
	int i;

	// read area from EEPROM
	iic_read2( IicPs, iic_address, 0x00, data, 8 );

	// calculate checksum
	for ( i = 0; i < 8; i++ ) {
		checksum += data[i];
	}

	// verify checksum
	if ( checksum != 0x00 )	{
		iic_debug_xil_printf( "FRU CHECKSUM ERROR\n\r" );
		return XST_FAILURE;
	}

	// decode data
	common_info->version         = data[0];
	common_info->offset.internal = data[1];
	common_info->offset.chassis  = data[2];
	common_info->offset.board    = data[3];
	common_info->offset.product  = data[4];
	common_info->offset.multi    = data[5];
	common_info->pad             = data[6];
	common_info->checksum        = data[7];

	// verify version
	if ( common_info->version != 0x01 ) {
		iic_debug_xil_printf( "FRU VERSION ERROR\n\r" );
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}


static int fmc_ipmi_get_board_info( XIicPs *IicPs, u8 iic_address, struct fru_area_board *board_info )
{
	struct fru_header common_info;
	int retval;
	u8 checksum = 0;
	int offset = 0x08; // fixed offset of board_info in IPMI EEPROM
	int i;

	retval = fmc_ipmi_get_common_info( IicPs, iic_address, &common_info );
	if ( retval != XST_SUCCESS ) {
		return retval;
	}

	// read area from EEPROM
	offset = common_info.offset.board << 3;
	board_info->area_len = sizeof(board_info->area_data);
	iic_read2( IicPs, iic_address, offset, (u8*) &board_info->area_data, board_info->area_len );

	// calculate checksum
	for ( i = 0; i < board_info->area_len; i++ ) {
		checksum += board_info->area_data[i];
	}

	// verify checksum
	if ( checksum != 0x00 )	{
		iic_debug_xil_printf( "FRU CHECKSUM ERROR\n\r" );
		return XST_FAILURE;
	}

	// decode data
	i = 0;
	// Version
	board_info->area_ver = board_info->area_data[i++];
	// Area length (multiples of 8 bytes)
	board_info->area_len = board_info->area_data[i++] << 3;
	// Language (English)
	board_info->lang = board_info->area_data[i++];
	// Mrg. Data / Time - (Little Endian)
	board_info->mfg_date_time = (board_info->area_data[i+0])
							  | (board_info->area_data[i+1] <<  8)
							  | (board_info->area_data[i+2] << 16);
	i += 3;
	// Board Manufacturer (8 bit ASCII)
	board_info->mfg = &board_info->area_data[i+1];
	i += 1 + (board_info->area_data[i] & 0x3F);
	// Board Product Name (8 bit ASCII)
	board_info->prod = &board_info->area_data[i+1];
	i += 1 + (board_info->area_data[i] & 0x3F);
	// Board Serial Number (8 bit ASCII)
	board_info->serial = &board_info->area_data[i+1];
	i += 1 + (board_info->area_data[i] & 0x3F);
	// Board Part Number (8 bit ASCII)
	board_info->part = &board_info->area_data[i+1];
	i += 1 + (board_info->area_data[i] & 0x3F);
	// FRU File (8 bit ASCII)
	board_info->fru = &board_info->area_data[i+1];
	i += 1 + (board_info->area_data[i] & 0x3F);

	// verify version
	if ( board_info->area_ver != 0x01 ) {
		iic_debug_xil_printf( "FRU VERSION ERROR\n\r" );
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}


int zc702_fmc_ipmi_detect( XIicPs *IicPs, u8 FmcSlot, struct fru_area_board *board_info )
{
	int Status;
	u8 read_data;

	// set IIC Mux according to selected FMC slot
	if ( FmcSlot == 1 ) {
		zc702_iic_mux( IicPs, ZC702_I2C_SELECT_FMC1 );
	} else if ( FmcSlot == 2 ) {
		zc702_iic_mux( IicPs, ZC702_I2C_SELECT_FMC2 );
	} else {
		iic_debug_xil_printf("ERROR : Selected slot FMC%d invalid!\n\r", FmcSlot);
		return XST_FAILURE;
	}

	// read current address of IPMI EEPROM
	Status = iic_read1( IicPs, ZC702_FMC_IPMI_ADDR, &read_data );
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// read FRU board info from IPMI EEPROM
	Status = fmc_ipmi_get_board_info( IicPs, ZC702_FMC_IPMI_ADDR, board_info );
	if ( Status != XST_SUCCESS ) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}


#ifdef USRCLK_SUPPORT
void zc702_usrclk_init( XIicPs *IicPs, u8 Resolution )
{
	zc702_iic_mux( IicPs, ZC702_I2C_SELECT_USRCLK );

	switch (Resolution) {
		case VIDEO_RESOLUTION_VGA:
			iic_writex( IicPs, ZC702_USRCLK_ADDR, zc702_usrclk_config_VGA, ZC702_USRCLK_CONFIG_LEN );
//			iic_readx(  IicPs, ZC702_USRCLK_ADDR, zc702_usrclk_config_VGA, ZC702_USRCLK_CONFIG_LEN );
			break;
		case VIDEO_RESOLUTION_SVGA:
			iic_writex( IicPs, ZC702_USRCLK_ADDR, zc702_usrclk_config_SVGA, ZC702_USRCLK_CONFIG_LEN );
//			iic_readx(  IicPs, ZC702_USRCLK_ADDR, zc702_usrclk_config_SVGA, ZC702_USRCLK_CONFIG_LEN );
			break;
		case VIDEO_RESOLUTION_XGA:
			iic_writex( IicPs, ZC702_USRCLK_ADDR, zc702_usrclk_config_XGA, ZC702_USRCLK_CONFIG_LEN );
//			iic_readx(  IicPs, ZC702_USRCLK_ADDR, zc702_usrclk_config_XGA, ZC702_USRCLK_CONFIG_LEN );
			break;
		case VIDEO_RESOLUTION_HD720P:
			iic_writex( IicPs, ZC702_USRCLK_ADDR, zc702_usrclk_config_HD720p, ZC702_USRCLK_CONFIG_LEN );
//			iic_readx(  IicPs, ZC702_USRCLK_ADDR, zc702_usrclk_config_HD720p, ZC702_USRCLK_CONFIG_LEN );
			break;
		case VIDEO_RESOLUTION_SXGA:
			iic_writex( IicPs, ZC702_USRCLK_ADDR, zc702_usrclk_config_SXGA, ZC702_USRCLK_CONFIG_LEN );
//			iic_readx(  IicPs, ZC702_USRCLK_ADDR, zc702_usrclk_config_SXGA, ZC702_USRCLK_CONFIG_LEN );
			break;
		case VIDEO_RESOLUTION_WSXGA:
			iic_writex( IicPs, ZC702_USRCLK_ADDR, zc702_usrclk_config_WSXGA, ZC702_USRCLK_CONFIG_LEN );
//			iic_readx(  IicPs, ZC702_USRCLK_ADDR, zc702_usrclk_config_WSXGA, ZC702_USRCLK_CONFIG_LEN );
			break;
		case VIDEO_RESOLUTION_HD1080P:
			iic_writex( IicPs, ZC702_USRCLK_ADDR, zc702_usrclk_config_HD1080p, ZC702_USRCLK_CONFIG_LEN );
//			iic_readx(  IicPs, ZC702_USRCLK_ADDR, zc702_usrclk_config_HD1080p, ZC702_USRCLK_CONFIG_LEN );
			break;
	}
}
#endif
