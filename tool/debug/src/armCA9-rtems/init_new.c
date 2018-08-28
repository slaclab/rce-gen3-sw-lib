// -*-Mode: C;-*-
/**
@file
@brief Implement debug init. for RTEMS.
@verbatim
                               Copyright 2014
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
2014/02/16

@par Last commit:
\$Date: 2014-07-31 17:05:57 -0700 (Thu, 31 Jul 2014) $ by \$Author: tether $.

@par Revision number:
\$Revision: 3597 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/debug/src/armCA9-rtems/init_new.c $

@par Credits:
SLAC
*/

#include "debug/platform/init.h"

#include "debug/platform/fatal.h"

#include "debug/os/syslog.h"

#include "debug/os/extension.h"

#include "io/platform/consoleSupport.h"


void dbg_earlyInit(void) {
  /* Set up a polled console with no syslog capture, since the syslog
     has yet to be initialized. */
  io_setPolledConsole(NULL);
  dbg_initSyslog();
  /* Now we can use syslog capture, though we can't assume that the
     time-related library routines needed for making timestamps are
     available yet. */
  io_setPolledConsole(dbg_syslogPutcNoTimestamp);
}


void dbg_lateInit(void) {
  dbg_installExtension();
  dbg_setTaskingIsEnabled(true);
}
