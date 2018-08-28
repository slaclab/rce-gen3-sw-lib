// -*-Mode: C++;-*-
/**
@file
@brief Declare a function that installs a C++ terminate handler for RTEMS.
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
\$Date: 2014-06-04 17:27:22 -0700 (Wed, 04 Jun 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3344 $

@par Credits:
SLAC
*/
#if !defined(TOOL_EXCEPTION_TERMINATE_HH)
#define      TOOL_EXCEPTION_TERMINATE_HH

#include <exception>

namespace tool {

  namespace exception {

    void rtemsTerminateHandler();

    inline void installTerminateHandler() {
      std::set_terminate(rtemsTerminateHandler);
    }
  }

}
#endif
