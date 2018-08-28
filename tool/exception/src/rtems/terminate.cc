// -*-Mode: C++;-*-
/**
@file
@brief Define the C++ terminate handler for RTEMS.
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
2014/02/24

@par Last commit:
\$Date: 2014-04-07 21:30:01 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3125 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/exception/src/rtems/terminate.cc $

@par Credits:
SLAC
*/


namespace project {

  namespace package {
  }

}
#include <cstdlib>
using std::free;

#include <cxxabi.h>
using abi::__cxa_current_exception_type;
using abi::__cxa_demangle;

#include <typeinfo>
using std::type_info;

#include <rtems.h>


#include "debug/print.h"



namespace tool {

  namespace exception {

    void rtemsTerminateHandler() {
      // Make sure there was an exception; terminate is also called for an
      // attempt to rethrow when there is no suitable exception.
      type_info *t = __cxa_current_exception_type();
      if (t) {
        // Note that "name" is the mangled name.
        char const *name = t->name();
        int status = -1;
        const char *const dem(__cxa_demangle(name, 0, 0, &status));
        dbg_printv("Suspending task 0x%08x: Didn't catch exception %s.\n",
                   unsigned(rtems_task_self()),
                   status ? name : dem
                   );
        if (!status) free(const_cast<char*>(dem));
      }
      else {
        dbg_printv("Suspending task 0x%08x: Tried to rethrow when there was no exception.\n",
                   unsigned(rtems_task_self())
                   );
      }
      rtems_task_suspend(rtems_task_self());
    }



  }
}


