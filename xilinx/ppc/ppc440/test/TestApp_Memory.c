/*
 *
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

/*
 * Xilinx EDK 12.4 EDK_MS4.81d
 *
 * This file is a sample test application
 *
 * This application is intended to test and/or illustrate some
 * functionality of your system.  The contents of this file may
 * vary depending on the IP in your system and may use existing
 * IP driver functions.  These drivers will be generated in your
 * XPS project when you run the "Generate Libraries" menu item
 * in XPS.
 *
 * Your XPS project directory is at:
 *    C:\Users\claus\Documents\ctk\
 */


// Located in: ppc440_0/include/xparameters.h
#include "xparameters.h"

#include "stdio.h"

#include "xil_testmem.h"
#include "xstatus.h"

//====================================================

int main (void) {


   print("-- Entering main() --\r\n");

   /*
    * MemoryTest routine will not be run for the memory at
    * 0xffff0000 (xps_bram_if_cntlr_1)
    * because it is being used to hold a part of this application program
    */


   /*
    * MemoryTest routine will not be run for the memory at
    * 0xfc000000 (FLASH)
    * because it is a read-only memory
    */


   /* Testing Memory (DDR2_SDRAM_DIMM0)*/
   {
      int status;

      print("Starting MemoryTest for DDR2_SDRAM_DIMM0:\r\n");
      print("  Running 32-bit test...");
      status = Xil_TestMem32((u32*)XPAR_DDR2_SDRAM_DIMM0_MEM_BASEADDR, 1024, 0xAAAA5555, XIL_TESTMEM_ALLMEMTESTS);
      if (status == 0) {
         print("PASSED!\r\n");
      }
      else {
         print("FAILED!\r\n");
      }
      print("  Running 16-bit test...");
      status = Xil_TestMem16((u16*)XPAR_DDR2_SDRAM_DIMM0_MEM_BASEADDR, 2048, 0xAA55, XIL_TESTMEM_ALLMEMTESTS);
      if (status == 0) {
         print("PASSED!\r\n");
      }
      else {
         print("FAILED!\r\n");
      }
      print("  Running 8-bit test...");
      status = Xil_TestMem8((u8*)XPAR_DDR2_SDRAM_DIMM0_MEM_BASEADDR, 4096, 0xA5, XIL_TESTMEM_ALLMEMTESTS);
      if (status == 0) {
         print("PASSED!\r\n");
      }
      else {
         print("FAILED!\r\n");
      }
   }

   print("-- Exiting main() --\r\n");
   return 0;
}

