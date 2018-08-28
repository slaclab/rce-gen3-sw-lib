// -*-Mode: C++;-*-
/*!
*
* @brief   A simple "hello world" test module
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    April 12, 2012 -- Created
*
* $Revision: 1929 $
*
* @verbatim
*                               Copyright 2012
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#include "datCode.hh"
#include DAT_PUBLIC(service, logger, Logger.hh)
using service::logger::Logger;


extern "C" void rce_appmain(uintptr_t)
{
  Logger().info("Hello World!");
}

//#include <stdio.h>
//
//extern "C" void rce_appmain(void*)
//{
//  //*(unsigned*)0x1500 = 0xdeadbeef;
//  printf("Hello World!\n");
//}
