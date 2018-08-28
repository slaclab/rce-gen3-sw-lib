// -*-Mode: C;-*-
/**
@file
@brief Implement the installer for our RTEMS extension.
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
Steve Tether <tether@slac.stanford.edu>

@par Date created:
2014/02/16

@par Last commit:
\$Date: 2014-04-07 21:30:01 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3125 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/debug/src/rtems/extension_new.c $

@par Credits:
SLAC
*/
#include <stdio.h>

#include <rtems.h>



#include "debug/os/extension.h"
#include "debug/os/taskExit.h"

#include "debug/platform/fatal.h"

static rtems_extensions_table dbg_exttable = {
    0, // Task creation.
    0, // Task start.
    0, // Task restart.
    0, // Task delete.
    0, // Task switch.
    0, // Task begin.
    dbg_handleTaskExit,
    0 // Fatal error handler.
};

/* Note that installing a fatal error handler using this technique is
   useless since a set of static extensions is configured into RTEMS and
   the static extensions are searched first.  Combine that with the lack
   of return from fatal error handlers and you can see why a
   dynamically installed fatal error handler is never called. Instead
   we supply our own version of the static fatal error handler in
   fatal.c.
*/ 

void dbg_installExtension() {
  const rtems_name extname = rtems_build_name('D', 'A', 'T', '1');
  rtems_id   extid;
  const rtems_status_code status = rtems_extension_create(extname, &dbg_exttable, &extid);
  if (status != RTEMS_SUCCESSFUL) {
    printk("Failed to create the RTEMS extension, status code %u\n", (unsigned)status);
  }
}
