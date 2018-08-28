// -*-Mode: C++;-*-
//
//                           Copyright 2014
//                                 by
//                    The Board of Trustees of the
//                 Leland Stanford Junior University.
//                        All rights reserved.
//

#include <cstdio>
#include "dslutil/RebootSpace.hh"

namespace service {
  namespace dslutil {
    namespace rebooter {
    
      void Space::_reboot(uint32_t os, uint32_t load_bit)
      {
        printf("_reboot member should not be used on the desktop.\n");
      }
    }
  }
}
