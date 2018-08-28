// -*-Mode: C++;-*-
//
//                           Copyright 2015
//                                 by
//                    The Board of Trustees of the
//                 Leland Stanford Junior University.
//                        All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dslutil/IdSpace.hh"
#include "dslutil/Base.h"

namespace service {
  namespace dslutil {
    namespace identifier {

      const char _last_update[]  = "/last_update";

      uint32_t
      Space::_os() { return OS_LINUX; }

      const char *
      Space::_id()
      {
        FILE *fd;        
        fd = fopen(_last_update,"rb");
        
        bzero(_idstr,DSLUTIL_MAX_ID_STR);        
        if(fd)                                     
          {          
          fread(_idstr,DSLUTIL_MAX_ID_STR,1,fd);
          fclose(fd);
          }
        return _idstr;
      }
    }
  }
}
