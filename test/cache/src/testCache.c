/******************************************************************************
*
* (c) Copyright 2009 Xilinx, Inc. All rights reserved.
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
/*****************************************************************************/
/**
*
* @file testCache.c
*
* Contains utility functions to test cache.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a hbm  07/28/09 Initial release
*
* 2.00a RiC  09/06/13 Changed to use RTEMS cache functions
*
* </pre>
*
* @note
*
* This file contain functions that all operate on HAL.
*
******************************************************************************/
#include <stdio.h>

#include <rtems.h>


#define DATA_LENGTH 128

static unsigned Data[DATA_LENGTH];

/**
* Perform DCache range related API test such as DCacheFlushRange and
* DCacheInvalidateRange. This test function writes a constant value
* to the Data array, flushes the range, writes a new value, then invalidates
* the corresponding range.
*
* @return
*
*     - 0 is returned for a pass
*     - -1 is returned for a failure
*/
int TestDCacheRange(void)
{
  int      Index;
  int      Status;
  unsigned Value;

  printf("-- Cache Range Test --\n");


  for (Index = 0; Index < DATA_LENGTH; Index++)
    Data[Index] = 0xA0A00505;

  printf("    initialize Data done:\n");

  rtems_cache_flush_multiple_data_lines(Data, DATA_LENGTH * sizeof(unsigned));

  printf("    flush range done\n");
  for (Index = 0; Index < DATA_LENGTH; Index++)
    Data[Index] = Index + 3;

  rtems_cache_invalidate_multiple_data_lines(Data, DATA_LENGTH * sizeof(unsigned));

  printf("    invalidate dcache range done\n");

  Status = 0;

  for (Index = 0; Index < DATA_LENGTH; Index++) {
    Value = Data[Index];
    if (Value != 0xA0A00505) {
      Status = -1;
      printf("Data[%d] = %x\n", Index, Value);
      break;
    }
  }

  if (!Status) {
    printf("    Invalidate worked\n");
  }
  else {
    printf("Error: Invalidate dcache range not working\n");
  }

  printf("-- Cache Range Test Complete --\n");

  return Status;

}

/**
* Perform DCache all related API test such as DCacheFlush and
* DCacheInvalidate. This test function writes a constant value
* to the Data array, flushes the DCache, writes a new value, then invalidates
* the DCache.
*
* @return
*     - 0 is returned for a pass
*     - -1 is returned for a failure
*/
int TestDCacheAll(void)
{
  int      Index;
  int      Status;
  unsigned Value;

  printf("-- Cache All Test --\n");


  for (Index = 0; Index < DATA_LENGTH; Index++)
    Data[Index] = 0x50500A0A;

  printf("    initialize Data done:\n");

  rtems_cache_flush_entire_data();

  printf("    flush all done\n");

  for (Index = 0; Index < DATA_LENGTH; Index++)
    Data[Index] = Index + 3;

  rtems_cache_invalidate_entire_data();

  printf("    invalidate all done\n");

  Status = 0;

  for (Index = 0; Index < DATA_LENGTH; Index++) {
    Value = Data[Index];
    if (Value != 0x50500A0A) {
      Status = -1;
      printf("Data[%d] = %x\n", Index, Value);
      break;
    }
  }

  if (!Status) {
    printf("    Invalidate all worked\n");
  }
  else {
    printf("Error: Invalidate dcache all not working\n");
  }

  printf("-- DCache all Test Complete --\n");

  return Status;

}


/**
* Perform rtems_cache_invalidate_multiple_instruction_lines() on a few function
* pointers.
*
* @return
*
*     - 0 is returned for a pass
*     The function will hang if it fails.
*/
int TestICacheRange(void)
{

  rtems_cache_invalidate_multiple_instruction_lines(TestICacheRange, 1024);
  rtems_cache_invalidate_multiple_instruction_lines(TestDCacheRange, 1024);
  rtems_cache_invalidate_multiple_instruction_lines(TestDCacheAll, 1024);

  printf("-- Invalidate icache range done --\n");

  return 0;
}

/**
* Perform rtems_cache_invalidate_entire_instruction().
*
* @return
*
*     - 0 is returned for a pass
*     The function will hang if it fails.
*/
int TestICacheAll(void)
{
  rtems_cache_invalidate_entire_instruction();

  printf("-- Invalidate icache all done --\n");
  return 0;
}


void rce_appmain()
{
  int status = 0;

  printf("%s: Calling TestDCacheRange\n", __func__);
  status |= TestDCacheRange();
  printf("%s: Calling TestDCacheAll\n", __func__);
  status |= TestDCacheAll();
  printf("%s: Calling TestICacheRange\n", __func__);
  status |= TestICacheRange();
  printf("%s: Calling TestICacheAll\n", __func__);
  status |= TestICacheAll();
  printf("%s: Exiting with %sfailures seen\n", __func__, status ? "" : "no ");
}
