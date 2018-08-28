// -*-Mode: C;-*-
/**
@file
@brief Implement a task-exit handler for RTEMS.
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
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/debug/src/rtems/taskExit_new.c $

@par Credits:
SLAC
*/


#include "debug/os/taskExit.h"

void dbg_handleTaskExit(rtems_tcb* tcb) {rtems_task_delete(tcb->Object.id);}
