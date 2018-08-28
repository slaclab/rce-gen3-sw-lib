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
Sergio Maldonado, SLAC (smaldona@slac.stanford.edu)

@par Date created:
2014/01/13

@par Last commit:
\$Date: 2014-04-07 21:29:49 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3123 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/service/shell/test/hello_task.cc $

@par Credits:
SLAC

*/



#include "task/Task.h"         
#include "debug/print.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
** ++
**
**
** --
*/

void Task_Start(int argc, const char** argv)
{
  dbg_printv("hello_task was called with %d arguments @ %p: ",
             argc, argv);
  int i=0;
  while (i < argc) {
    dbg_printv("%s ", argv[i]);
    i++;
  }
  dbg_printv("\n");
  rtems_task_suspend(rtems_task_self());
}

/*
** ++
**
**
** --
*/

void Task_Rundown()
  {
    dbg_printv("ending hello_task\n");
    return;
  }

#if defined(__cplusplus)
} // extern "C"
#endif

