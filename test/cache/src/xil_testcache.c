/**
* @file      xil_testcache.c
*
* @brief     Wrapper module for calling Xilinx' cache testing code.
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      September 6, 2013 -- Created
*
* $Revision: 2101 $
*
* @verbatim                    Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#include <stdio.h>

#include "xil_testcache.h"


void rce_appmain()
{
  int status = 0;

  printf("%s: Calling Xil_TestDCacheRange\n", __func__);
  status |= Xil_TestDCacheRange();
  printf("%s: Calling Xil_TestDCacheAll\n", __func__);
  status |= Xil_TestDCacheAll();
  printf("%s: Calling Xil_TestICacheRange\n", __func__);
  status |= Xil_TestICacheRange();
  printf("%s: Calling Xil_TestICacheAll\n", __func__);
  status |= Xil_TestICacheAll();
  printf("%s: Exiting with %sfailures seen\n", __func__, status ? "" : "no ");
}
