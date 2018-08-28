/**
* @file      xil_printf.c
*
* @brief     Wrapper function to avoid having Xilinx' xil_printf in our code.
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      September 6, 2013 -- Created
*
* $Revision: 2090 $
*
* @verbatim                    Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void xil_printf(const char *fmt, ...)
{
  va_list va;
  va_start(va, fmt);
  vprintf(fmt, va);
  va_end(va);
}
