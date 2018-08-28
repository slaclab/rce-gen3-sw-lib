// -*-Mode: C++;-*-
/**
@file
@brief Throw a C++ exception from a task and see what happens.

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
Stephen Tether, SLAC (tether@slac.stanford.edu)

@par Date created:
2014/02/21

@par Last commit:
\$Date: 2014-06-16 12:17:33 -0700 (Mon, 16 Jun 2014) $ by \$Author: tether $.

@par Revision number:
\$Revision: 3392 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/exception/test/rtems/throw_task.cc $

@par Credits:
SLAC

*/

#include <stdexcept>



#include "task/Task.h"         
#include "debug/print.h"
#include "exception/terminate.hh"

class MyException {
};

extern "C" void Task_Start(int argc, const char** argv)
{
  tool::exception::installTerminateHandler();

  dbg_prints("About to throw local class MyException.\n");
  try {
    throw MyException();
  }
  catch (MyException &exc) {
    dbg_prints("Caught it!\n");
  }

  dbg_prints("About to throw std::runtime_error.\n");
  try {
    throw std::runtime_error("Error!");
  }
  catch (std::runtime_error &exc) {
    dbg_printv("Caught it! what='%s'\n", exc.what());
  }
}

extern "C" void Task_Rundown()
{
  dbg_prints("In Task_Rundown() for throw_task.cc\n");
}
