// -*-Mode: C++;-*-
//
//                           Copyright 2014
//                                 by
//                    The Board of Trustees of the
//                 Leland Stanford Junior University.
//                        All rights reserved.
//

#include "dslutil/IdSpace.hh"
#include "dslutil/Base.h"

namespace service {
  namespace dslutil {
    namespace identifier {

      uint32_t
      Space::_os() { return OS_UNKNOWN; }

      const char *
      Space::_id() { return NULL; }
      
    }
  }
}
