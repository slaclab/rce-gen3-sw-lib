/*!@file   memTestX.S
*
* @brief   A bootable program to test memory
*
*    This file was derived from TestApp_Memory.c, annotated as follows:
*
*      Xilinx EDK 12.4 EDK_MS4.81d
*
*      This file is a sample test application
*
*      This application is intended to test and/or illustrate some
*      functionality of your system.  The contents of this file may
*      vary depending on the IP in your system and may use existing
*      IP driver functions.  These drivers will be generated in your
*      XPS project when you run the "Generate Libraries" menu item
*      in XPS.
*
*    Changes made to the original are:
*    - Removal of dependency on a UART
*    - Replacement of prints with front panel display signals
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    December 8, 2011 -- Created
*
* $Revision: 1376 $
*
* @verbatim                    Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

/*
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "datCode.hh"
#include DAT_PUBLIC(bootstrap, common, params.h)
#include DAT_PUBLIC(bootstrap, common, print.h)

#define XPAR_DDR_SDRAM_MPMC_BASEADDR EXTADDR
#define XPAR_DDR_SDRAM_MPMC_SIZE     ((unsigned)(EXTEND - EXTADDR) + 1) /* Bytes */

#include DAT_PRIVATE(bootstrap, memTestX, xil_testmem.h)
#define XST_SUCCESS 0
typedef unsigned XStatus;

//====================================================

int main (void)
{
   print("-- Entering main() --\r\n");

   /*
    * MemoryTest routine will not be run for the memory at
    * 0xfffff000 (xps_bram_if_cntlr_1)
    * because it is being used to hold a part of this application program
    */


   /* Testing MPMC Memory (DDR_SDRAM)*/
   {
      XStatus status;
      u32     cnt = XPAR_DDR_SDRAM_MPMC_SIZE;

      print("Starting MemoryTest for DDR_SDRAM:\r\n");
      print("  Running 32-bit test...");
      status = Xil_TestMem32((u32*)XPAR_DDR_SDRAM_MPMC_BASEADDR, cnt/4, 0xAAAA5555, XIL_TESTMEM_ALLMEMTESTS);
      if (status == XST_SUCCESS) {
         print("PASSED!\r\n");
      }
      else {
         print("FAILED!\r\n");
      }
      print("  Running 16-bit test...");
      status = Xil_TestMem16((u16*)XPAR_DDR_SDRAM_MPMC_BASEADDR, cnt/2, 0xAA55, XIL_TESTMEM_ALLMEMTESTS);
      if (status == XST_SUCCESS) {
         print("PASSED!\r\n");
      }
      else {
         print("FAILED!\r\n");
      }
      print("  Running 8-bit test...");
      status = Xil_TestMem8((u8*)XPAR_DDR_SDRAM_MPMC_BASEADDR, cnt, 0xA5, XIL_TESTMEM_ALLMEMTESTS);
      if (status == XST_SUCCESS) {
         print("PASSED!\r\n");
      }
      else {
         print("FAILED!\r\n");
      }
   }

   /**
    * MpmcSelfTestExample() will not be run for the memory
    * (DDR_SDRAM) because ECC is not supported.
    */


   print("-- Exiting main() --\r\n");
   return 0;
}

