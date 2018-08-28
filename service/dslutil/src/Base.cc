// -*-Mode: C++;-*-
//                               Copyright 2014
//                                    by
//                       The Board of Trustees of the
//                    Leland Stanford Junior University.
//                           All rights reserved.


#include <inttypes.h>
#include <string.h>

#include "dslutil/Base.h"


#if defined(__cplusplus)
extern "C" {
#endif


unsigned os_strtoint(const char* os_s)
{
  unsigned os = OS_NOTSET;
#if 0  
  if (os_s) {
    if (strcasestr(os_s, "linux"))
      os = OS_LINUX;
    else if (strcasestr(os_s, "rtems"))
      os = OS_RTEMS;
    else if (strcasestr(os_s, "rescue"))
      os = OS_RESCUE;
    else if (strcasestr(os_s, "ramdisk"))
      os = OS_RESCUE;
    else if (strcasestr(os_s, "default"))
      os = OS_NOTSET;
    else
      os = OS_UNKNOWN;
  } else {
    // Default is NOTSET
    os = OS_NOTSET;
  }
#endif  
  return os;
}



      

  /** @cond development





  @endcond
  */

#if defined(__cplusplus)
} // extern "C"
#endif
  
