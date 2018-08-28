/*!@file   testApp.c
*
* @brief   A test application
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    December 12, 2011 -- Created
*
* $Revision: 1356 $
*
* @verbatim                    Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

void readFile(void);


int main(void)
{
  static unsigned x = 0xdeadbeef;
  const  unsigned y = 0xabadcafe;
  volatile unsigned* const z = (volatile unsigned* const)0x06000000;
  static const char* hello = "Hello World!"; /* 13 characters */

  z[0] = x;
  z[1] = y;
  z[2] = (unsigned)hello;

  readFile();

  return 0;
}
