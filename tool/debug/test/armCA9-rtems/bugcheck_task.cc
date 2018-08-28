// -*-Mode: C;-*-
/**
@file
@brief 

@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
RTS

@author
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2014/004/17

@par Last commit:
\$Date: 2014-04-17 16:20:58 -0700 (Thu, 17 Apr 2014) $ by \$Author: tether $.

@par Revision number:
\$Revision: 3184 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/debug/test/armCA9-rtems/bugcheck_task.cc $

@par Credits:
SLAC

*/



#include "task/Task.h"         
#include "debug/print.h"

extern "C" {

  void Task_Start(int argc, const char** argv) {
    dbg_printv("About to call dbg_bugcheck().\n");
    dbg_bugcheck("Test of bugcheck.\n");
  }

  void Task_Rundown() {
    dbg_printv("Ending bugcheck_task.\n");
    return;
  }

} // extern "C"

