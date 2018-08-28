// -*-Mode: C++;-*-
/**
@file
@brief Test of exception handling in modules under RTEMS.
@verbatim
                               Copyright 2012
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
2012/05/07

@par Last commit:
\$Date: 2014-04-07 21:30:01 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3125 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/exception/test/handling.cc $

@par Credits:
SLAC
*/
#include <stdio.h>
#include <stdexcept>


#include "exception/Exception.hh"

// We use a statically allocated object with both a constructor and a
// destructor. The constructor should run when the task is linked and
// the destructor when it is unlinked (or the linker instance
// destroyed).
struct Stat {
  Stat()  {printf("Exception info for module 'handler' should now be registered.\n");}
  ~Stat() {printf("Exception info for module 'handler' should now be unregistered.\n");}
};

static Stat mystat;

namespace tool {
  namespace exception {
    void testThrow();
  }
}


extern "C" void rce_appmain(void*) {

  // Let's see if we can catch a tool::exception::Error that we throw
  // from inside this module. If we can it means that
  // exception-handling information for the module was properly
  // registered by rce_modinit().
  const char* msg = "FAILURE. Did not catch Error.\n";
  try {
    printf("About to throw Error from inside the module.\n");
    throw tool::exception::Error("foo");
  }
  catch (tool::exception::Error &e) {
    msg = "SUCCESS. Caught Error.\n";
  }
  printf(msg);

  // Now we'll try catching the Error exception when it's thrown
  // from the core (see JIRA CCI-9).
  msg = "FAILURE. Did not catch the Error exception.\n";
  try {
    printf("About to throw the Error exception from inside the core.\n");
    tool::exception::testThrow();
  }
  catch (tool::exception::Error &e) {
    msg = "SUCCESS. Caught Error.\n";
  }
  printf(msg);
}
