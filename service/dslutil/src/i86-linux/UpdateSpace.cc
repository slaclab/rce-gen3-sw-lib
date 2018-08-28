// -*-Mode: C++;-*-
//
//                           Copyright 2015
//                                 by
//                    The Board of Trustees of the
//                 Leland Stanford Junior University.
//                        All rights reserved.
//

#include <cstdio>
#include "dslutil/UpdateSpace.hh"
#include "dslutil/Base.h"

namespace service {
  namespace dslutil {
    namespace updater {

      uint32_t
      Space::_os() { return OS_UNKNOWN; }
    
      void Space::_update(uint32_t id, uint32_t ip, const char* src, const char* dst)
      {
        printf("_update member should not be used on the desktop.\n");
      }
    }
  }
}
