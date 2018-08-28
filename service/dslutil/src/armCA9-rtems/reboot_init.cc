// -*-Mode: C++;-*-
//
//                            Copyright 2014
//                                 by
//                    The Board of Trustees of the
//                 Leland Stanford Junior University.
//                        All rights reserved.
//

#include "dslutil/reboot_init.hh"

#if defined(__cplusplus)
extern "C" {
#endif

/** @cond development */

/** @brief Initialize the rebooter system on module load (RTEMS)
 */

int lnk_prelude(void* prefs, void* elf)
{
  service::dslutil::rebooter::initialize();
  return 0;
}

/** @endcond */

#if defined(__cplusplus)
} // extern "C"
#endif
