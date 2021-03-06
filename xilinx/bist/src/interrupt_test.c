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
 * 
 *
 * This file is a generated sample test application.
 *
 * This application is intended to test and/or illustrate some 
 * functionality of your system.  The contents of this file may
 * vary depending on the IP in your system and may use existing
 * IP driver functions.  These drivers will be generated in your
 * SDK application project when you run the "Generate Libraries" menu item.
 *
 */


#include <stdio.h>
#include "xparameters.h"
#include "xil_cache.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "scugic_header.h"
#include "xdevcfg.h"
#include "devcfg_header.h"
#include "xemacps.h"
#include "xemacps_example.h"
#include "emacps_header.h"
#include "xiicps.h"
#include "iicps_header.h"
#include "xqspips.h"
#include "qspips_header.h"
#include "xcanps.h"
#include "canps_header.h"
#include "xscutimer.h"
#include "scutimer_header.h"
#include "xscuwdt.h"
#include "scuwdt_header.h"
#include "xwdtps.h"
#include "wdtps_header.h"


int hello_intr() 
{

   static XScuGic intc;
   static XEmacPs ps7_ethernet_0;
   static XCanPs ps7_can_0;
   static XScuTimer ps7_scutimer_0;
   static XScuWdt ps7_scuwdt_0;

   {
       int Status;

    print("\n\r********************************************************");
    print("\n\r********************************************************");
    print("\n\r**               ZC702 Interrupt Tests                **");
    print("\n\r********************************************************");
    print("\n\r********************************************************\r\n");

       Status = ScuGicInterruptSetup(&intc, XPAR_PS7_SCUGIC_0_DEVICE_ID);
       if (Status == 0) {
          print("ScuGic Interrupt Setup PASSED\r\n");
       } 
       else {
         print("ScuGic Interrupt Setup FAILED\r\n");
      } 
   }
   
   {
      int Status;

      print("\r\n Running Interrupt Test  for ps7_ethernet_0...\r\n");
      
      Status = EmacPsDmaIntrExample(&intc, &ps7_ethernet_0, \
                                 XPAR_PS7_ETHERNET_0_DEVICE_ID, \
                                 XPAR_PS7_ETHERNET_0_INTR);
	
      if (Status == 0) {
         print("EmacPsDmaIntrExample PASSED\r\n");
      } 
      else {
         print("EmacPsDmaIntrExample FAILED\r\n");
      }

   }
   {
      int Status;

      print("\r\n Running Interrupt Test  for ps7_can_0...\r\n");
      
      Status = CanPsIntrExample(&intc, &ps7_can_0, \
                                 XPAR_PS7_CAN_0_DEVICE_ID, \
                                 XPAR_PS7_CAN_0_INTR);
	
      if (Status == 0) {
         print("CanPsIntrExample PASSED\r\n");
      } 
      else {
         print("CanPsIntrExample FAILED\r\n");
      }

   }
   

   {
      int Status;

      print("\r\n Running Interrupt Test  for ps7_scutimer_0...\r\n");
      
      Status = ScuTimerIntrExample(&intc, &ps7_scutimer_0, \
                                 XPAR_PS7_SCUTIMER_0_DEVICE_ID, \
                                 XPAR_PS7_SCUTIMER_0_INTR);
	
      if (Status == 0) {
         print("ScuTimerIntrExample PASSED\r\n");
      } 
      else {
         print("ScuTimerIntrExample FAILED\r\n");
      }

   }
   
   {
      int Status;

      print("\r\n Running Interrupt Test  for ps7_scuwdt_0...\r\n");
      
      Status = ScuWdtIntrExample(&intc, &ps7_scuwdt_0, \
                                 XPAR_PS7_SCUWDT_0_DEVICE_ID, \
                                 XPAR_PS7_SCUWDT_0_INTR);
	
      if (Status == 0) {
         print("ScuWdtIntrExample PASSED\r\n");
      } 
      else {
         print("ScuWdtIntrExample FAILED\r\n");
      }

   }
   
   return 0;
}

