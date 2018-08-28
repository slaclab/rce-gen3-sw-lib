/*!@file   print.h
*
* @brief   Routines for printing at a primative level.
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    November 8, 2012 -- Created
*
* $Revision: 1384 $
*
* @verbatim                    Copyright 2012
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef _PRINT_H_
#define _PRINT_H_

#define NL "\r\n"

typedef union
{
  uint64_t u64;
  uint32_t u32[2];
} uint64_u;

void print(const char* str);
void print_x(int n, uint32_t val);
void print64(const char* str, uint64_u value, const char* nl);
void println(const char* str, uint32_t width, uint32_t value, const char* nl);
void printc(char chr, uint32_t cnt);

#endif
