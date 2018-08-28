/*!@file   test.cc
*
* @brief   A test program for verifying the build system
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    February 5, 2013 -- Created
*
* $Revision: 1929 $
*
* @verbatim                    Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

#include "datCode.hh"

extern char* MsgAreaBase[];
extern char* MsgAreaSize[];

static char* bspMsgBuffer = (char*)MsgAreaBase;


void print(const char* str)
{
  const char* c = str;
  while (*c != '\0')
  {
    static char* msgBuffer = (char*)MsgAreaBase;
    *msgBuffer++ = *c++;
    if (msgBuffer >= &bspMsgBuffer[(int)MsgAreaSize])  msgBuffer = bspMsgBuffer;
    *msgBuffer   = 0x00;                /* Overwrite next location to show EOM */
  }
}


extern "C"
int main(void)
{
  print("Hello world!\n");

  return 0;
}
