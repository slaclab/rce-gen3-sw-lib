// -*-Mode: C++;-*-
/**
@file
@brief Implement debug console printout for C code under Linux.
@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
DAT

@author
Steve Tether <tether@slac.stanford.edu>

@par Date created:
2013/10/31

@par Last commit:
\$Date: 2014-04-07 21:30:01 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3125 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/debug/src/linux/print.c $

@par Credits:
SLAC
*/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>



#include "debug/print.h"

void dbg_prints(const char* msg) {printf(msg);}

void dbg_printv(const char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  vprintf(fmt, va);
  va_end(va);
}

void dbg_vprintv(const char* fmt, va_list va) {
  vprintf(fmt, va);
}

void dbg_bugcheck(const char* fmt, ...) {
  va_list va;
  dbg_prints("\nBUGCHECK!\n");
  va_start(va, fmt);
  vprintf(fmt, va);
  va_end(va);
  abort();
}

void dbg_vbugcheck(const char* fmt, va_list va) {
  dbg_prints("\nBUGCHECK!\n");
  vprintf(fmt, va);
  abort();
}
