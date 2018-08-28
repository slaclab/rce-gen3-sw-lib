// -*-Mode: C++;-*-
/**
@file
@brief Implement debug console printout for C code under RTEMS.

These functions must be callable from bsp_startup_hook_0(), before
most RTEMS and newlib initialization has been done.

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
\$Date: 2014-04-17 16:20:58 -0700 (Thu, 17 Apr 2014) $ by \$Author: tether $.

@par Revision number:
\$Revision: 3184 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/debug/src/rtems/print.c $

@par Credits:
SLAC
*/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <rtems.h>



#include "debug/print.h"

void dbg_printFatal(rtems_fatal_source, int, rtems_fatal_code, void (*)(const char*,...));

void dbg_prints(const char* msg) {printk(msg);}

void dbg_printv(const char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  vprintk(fmt, va);
  va_end(va);
}

void dbg_vprintv(const char* fmt, va_list va) {
  vprintk(fmt, va);
}

void dbg_bugcheck(const char* fmt, ...) {
  va_list va;
  dbg_prints("\nBUGCHECK!\n");
  va_start(va, fmt);
  vprintk(fmt, va);
  va_end(va);
  rtems_fatal(RTEMS_FATAL_SOURCE_APPLICATION, 0);
}

void dbg_vbugcheck(const char* fmt, va_list va) {
  dbg_prints("\nBUGCHECK!\n");
  vprintk(fmt, va);
  rtems_fatal(RTEMS_FATAL_SOURCE_APPLICATION, 0);
}
