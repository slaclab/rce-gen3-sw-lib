/*
 * Copyright (c) 2009 Xilinx, Inc.  All rights reserved.
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

#include <stdio.h>
#include "xparameters.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xil_testmem.h"

#include "platform.h"
#include "memory_config.h"

void putnum(unsigned int num);
void print(char *ptr);

void test_memory_range_ddr(struct memory_range_s *range) {
    XStatus status;

    /* This application uses print statements instead of xil_printf/printf
     * to reduce the text size.
     *
     * The default linker script generated for this application does not have
     * heap memory allocated. This implies that this program cannot use any
     * routines that allocate memory on heap (printf is one such function).
     * If you'd like to add such functions, then please generate a linker script
     * that does allocate sufficient heap memory.
     */

    print("Testing memory region: "); print(range->name);  print("\n\r");
    print("    Memory Controller: "); print(range->ip);  print("\n\r");
    print("         Base Address: 0x"); putnum(range->base); print("\n\r");
    print("                 Size: 0x"); putnum(range->size); print (" bytes \n\r");

    status = Xil_TestMem32((u32*)range->base, 128*1024*1024, 0xAAAA5555, XIL_TESTMEM_ALLMEMTESTS);
    print("          32-bit test: "); print(status == XST_SUCCESS? "PASSED!":"FAILED!"); print("\n\r");

    status = Xil_TestMem16((u16*)range->base, 128*1024*2048, 0xAA55, XIL_TESTMEM_ALLMEMTESTS);
    print("          16-bit test: "); print(status == XST_SUCCESS? "PASSED!":"FAILED!"); print("\n\r");

    status = Xil_TestMem8((u8*)range->base, 128*1024*4096, 0xA5, XIL_TESTMEM_ALLMEMTESTS);
    print("           8-bit test: "); print(status == XST_SUCCESS? "PASSED!":"FAILED!"); print("\n\r");

}

void test_memory_range_ram(struct memory_range_s *range) {
    XStatus status;

    /* This application uses print statements instead of xil_printf/printf
     * to reduce the text size.
     *
     * The default linker script generated for this application does not have
     * heap memory allocated. This implies that this program cannot use any
     * routines that allocate memory on heap (printf is one such function).
     * If you'd like to add such functions, then please generate a linker script
     * that does allocate sufficient heap memory.
     */

    print("Testing memory region: "); print(range->name);  print("\n\r");
    print("    Memory Controller: "); print(range->ip);  print("\n\r");
    print("         Base Address: 0x"); putnum(range->base); print("\n\r");
    print("                 Size: 0x"); putnum(range->size); print (" bytes \n\r");

    status = Xil_TestMem32((u32*)range->base, 1024, 0xAAAA5555, XIL_TESTMEM_ALLMEMTESTS);
    print("          32-bit test: "); print(status == XST_SUCCESS? "PASSED!":"FAILED!"); print("\n\r");

    status = Xil_TestMem16((u16*)range->base, 2048, 0xAA55, XIL_TESTMEM_ALLMEMTESTS);
    print("          16-bit test: "); print(status == XST_SUCCESS? "PASSED!":"FAILED!"); print("\n\r");

    status = Xil_TestMem8((u8*)range->base, 4096, 0xA5, XIL_TESTMEM_ALLMEMTESTS);
    print("           8-bit test: "); print(status == XST_SUCCESS? "PASSED!":"FAILED!"); print("\n\r");

}

int hello_mem()
{

    //init_platform();


    print("\n\r********************************************************");
    print("\n\r********************************************************");
    print("\n\r**              ZC702 - DDR3 Memory TEST              **");
    print("\n\r********************************************************");
    print("\n\r********************************************************\r\n");
    print("NOTE: This application runs with D-Cache disabled.\r\n");
    print("As a result, cacheline requests will not be generated\r\n");

    test_memory_range_ddr(&memory_ranges[0]);
    test_memory_range_ram(&memory_ranges[1]);


    //cleanup_platform();

    return 0;
}