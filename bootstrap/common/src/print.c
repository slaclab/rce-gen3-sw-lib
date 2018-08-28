/*!@file   print.c
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
#include "datCode.hh"
#include DAT_PUBLIC(bootstrap, common, print.h)
#include DAT_PUBLIC(bootstrap, common, apu.h)

extern char* MsgAreaBase[];
extern char* MsgAreaSize[];

static char* bspMsgBuffer = (char*)MsgAreaBase;


void print(const char* str)
{
  const char* c = str;
  while (*c != '\0')
  {
    static char* msgBuffer = (char*)MsgAreaBase;
    APU_udi5fcm(0, 0, *c); // UDI UART
    *msgBuffer++ = *c++;
    if (msgBuffer >= &bspMsgBuffer[(int)MsgAreaSize])  msgBuffer = bspMsgBuffer;
    *msgBuffer   = 0x00;                /* Overwrite next location to show EOM */
  }
}

void print_x(int n, uint32_t val)
{
  char buf[9];
  int  i;
  buf[n--] = '\0';
  for (i = n; i >= 0; --i)
  {
    buf[i] = "0123456789abcdef"[val & 0xf];
    val >>= 4;
  }
  print(buf);
}

void print64(const char* str, uint64_u value, const char* nl)
{
  print(str);  print_x(8, value.u32[0]);  print(" ");
               print_x(8, value.u32[1]);  print(nl);
}

void println(const char* str, uint32_t width, uint32_t value, const char* nl)
{
  print(str);  print_x(width, value);  print(nl);
}

void printc(char chr, uint32_t cnt)
{
  char     str[2] = {chr, '\0'};
  uint32_t i;
  for (i = 0; i < cnt; ++i)  print(str);}
