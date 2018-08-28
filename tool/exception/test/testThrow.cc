// -*-Mode: C++;-*-
/**
@file
@brief Throw an exception from the core that should be catchable in modules.
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
2012/05/9

@par Last commit:
\$Date: 2014-04-07 21:30:01 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3125 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/exception/test/testThrow.cc $

@par Credits:
SLAC
*/


#include "exception/Exception.hh"

#include <stdio.h>

namespace tool {

  namespace exception {

    void testThrow() {

      throw tool::exception::Error("foo");
    }

  }

}

