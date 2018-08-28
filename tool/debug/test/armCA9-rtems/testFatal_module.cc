// -*-Mode: C++;-*-
/**
@file
@brief Test code for tool::debug under RTEMS.
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
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2013/08/02

@par Last commit:
\$Date: 2014-04-07 21:30:01 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3125 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/debug/test/armCA9-rtems/testFatal_module.cc $

@par Credits:
SLAC
*/
#include <stdio.h>
#include <string.h>



#include "debug/registers.hh"

#include "debug/os/extension.hh"

#include "debug/platform/fatal.hh"

#include "io/rwall.hh"

void printme(const char* msg) {
  tool::io::writeAll(1, msg, strlen(msg));
}

extern "C" void rce_appmain(void*) {
  tool::debug::installExtension();
  printme("After installExtension(). About to use SWI.\n");
  asm volatile("swi #0":::"memory");

  rtems_fatal(RTEMS_FATAL_SOURCE_APPLICATION,
              rtems_fatal_code(123));

  // CPU_Exception_frame ctx = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 0xdeadbeef,
  //                            (void*)0xfeedbeef, (void*)0xcabadaba, 0xbaddead, ARM_EXCEPTION_UNDEF};
  // tool::debug::printFatal(RTEMS_FATAL_SOURCE_EXCEPTION,
  //                         false,
  //                         (rtems_fatal_code)(&ctx),
  //                         printme);

}
