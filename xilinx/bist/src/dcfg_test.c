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


int hello_dcfg() 
{


   {
      int Status;
      
    xil_printf("\n\r********************************************************");
    xil_printf("\n\r********************************************************");
    xil_printf("\n\r**                 ZC702 - DCFG Test                  **");
    xil_printf("\n\r********************************************************");
    xil_printf("\n\r********************************************************\r\n");
      
      Status = DcfgSelfTestExample(XPAR_PS7_DEV_CFG_0_DEVICE_ID);
      
      if (Status == 0) {
         print("DcfgSelfTestExample PASSED\r\n");
      }
      else {
         print("DcfgSelfTestExample FAILED\r\n");
      }
   }
   

   return 0;
}

