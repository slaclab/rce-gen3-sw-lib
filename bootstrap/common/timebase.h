/*!@file      timebase.h
*
*  @brief     Timebase inline function.
*
*  @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
*  @date      November 8, 2012 -- Created
*
*  $Revision: 1384 $
*
*  @verbatim                    Copyright 2012
*                                      by
*                         The Board of Trustees of the
*                       Leland Stanford Junior University.
*                              All rights reserved.
*  @endverbatim
*/
#ifndef _TIMEBASE_H_
#define _TMIEBASE_H_

inline void readTB(uint32_t* tb)
{
  register uint32_t tmp;
  __asm__ volatile ("1: mftbu %[U] \t\n"
                    "   mftbl %[L] \t\n"
                    "   mftbu %[O] \t\n"
                    "   cmpw  %[U],%[O] \t\n"
                    "   bne+  1b"
                    : [U]"=r"(tb[0]), [L]"=r"(tb[1]), [O]"=r"(tmp)
                    :
                    : "memory");
}

#endif
