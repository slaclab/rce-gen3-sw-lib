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
/**************************************************************************
* Filename:     menu.c
*
* Description:
* Menu that runs the board test applications.
*
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  cmc  8-27-09  Initial Release
*
*
******************************************************************************/
#include "menu.h"
#include "board_test_app.h"
#include <ctype.h>
#include "xparameters.h"
#include "xil_printf.h"
#include "xil_cache.h"

/*
 * Macros to enable/disable caches.
 */
#ifndef ENABLE_ICACHE
#define ENABLE_ICACHE()		Xil_ICacheEnable()
#endif
#ifndef	ENABLE_DCACHE
#define ENABLE_DCACHE()		Xil_DCacheEnable()
#endif
#ifndef	DISABLE_ICACHE
#define DISABLE_ICACHE()	Xil_ICacheDisable()
#endif
#ifndef DISABLE_DCACHE
#define DISABLE_DCACHE()	Xil_DCacheDisable()
#endif

extern int hello_bram (void);
extern int main_sysmonIntr(void);
extern int hello_mem (void);

//int main()
int main_automate()
{

	//DISABLE_ICACHE();
	//DISABLE_DCACHE();
    init_platform();

    print("\n\r********************************************************");
    print("\n\r********************************************************");
    print("\n\r**    Xilinx Zynq-7000 AP SoC ZC702 Evaluation Kit    **");
    print("\n\r********************************************************");
    print("\n\r********************************************************\r\n");

	while(1) {
		hello_uart(XUARTPS_DFT_BAUDRATE);
		hello_led();
		hello_iic_eeprom();
		//hello_flash();
		hello_timer();
    hello_scugic();
    hello_dcfg();
		hello_switch();
		hello_intr();
		//hello_can();	
		hello_watchdog();
	}
	print("Good-bye!\r\n");
    cleanup_platform();
    return 0;
}

//int main_user_interact()
int main()
{
    int choice, exit_flag;

	//ENABLE_ICACHE();
	//ENABLE_DCACHE();
    init_platform();

    /* Initialize RS232_Uart_1 - Set baudrate and number of stop bits */
    //XUartNs550_SetBaud(XPAR_RS232_UART_1_BASEADDR, XPAR_XUARTNS550_CLOCK_HZ,
    //                   UART_BAUDRATE);
    //XUartNs550_SetLineControlReg(XPAR_RS232_UART_1_BASEADDR,
    //                              XUN_LCR_8_DATA_BITS);

    print("\n\r********************************************************");
    print("\n\r********************************************************");
    print("\n\r**    Xilinx Zynq-7000 AP SoC ZC702 Evaluation Kit    **");
    print("\n\r********************************************************");
    print("\n\r********************************************************\r\n");

	exit_flag = 0;
	while(exit_flag != 1) {
    	//do {
    	    print("Choose Feature to Test:\r\n");
    	    print("1: UART Test\r\n");
    	    print("2: LED Test\r\n");
    	    print("3: IIC Test\r\n");
    	    //print("4: FLASH Test\r\n");
    	    print("4: TIMER Test\r\n");
    	    print("5: SWITCH Test\r\n");
    	    print("6: SCUGIC Test\r\n");
    	    print("7: DCFG Test\r\n");
    	    print("8: DDR3 Memory Test\r\n");
    	    print("9: Interrupt Test\r\n");
    	    //print("A: CAN Test\r\n");
    	    print("A: Watchdog Timer Test\r\n");
    	    print("0: Exit\r\n");
    	    choice = inbyte();
        	if (isalpha(choice)) {
        	    choice = toupper(choice);
        	}
    	//} while (!isdigit(choice));
    	xil_printf("%c\r\n", choice);

		switch(choice) {
			case '0':
				exit_flag = 1;
				break;
			case '1':
				hello_uart(XUARTPS_DFT_BAUDRATE);
				break;
			case '2':
				hello_led();
				break;
			case '3':
				hello_iic_eeprom();
				break;
			//case '4':
				//hello_flash();
				//break;
			case '4':
				hello_timer();
				break;
			case '5':
				hello_switch();
				break;
			case '6':
				hello_scugic();
				break;
			case '7':
				hello_dcfg();
				break;
			case '8':
				hello_mem();
				break;
			case '9':
				hello_intr();
				break;
			//case 'A':
				//hello_can();
				//break;
			case 'A':
				hello_watchdog();
				break;
			default:
				break;
		}
		if(exit_flag != 1) {
			print("Press any key to return to main menu\r\n");
			inbyte();
		}
	}
	print("Good-bye!\r\n");
    cleanup_platform();

    return 0;
}
