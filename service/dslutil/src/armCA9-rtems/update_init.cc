// -*-Mode: C++;-*-
//
//                            Copyright 2015
//                                 by
//                    The Board of Trustees of the
//                 Leland Stanford Junior University.
//                        All rights reserved.
//

#include "dslutil/update_init.hh"

#if defined(__cplusplus)
extern "C" {
#endif

/** @cond development */

/** @brief Initialize the updater system on module load (RTEMS)
 */

int lnk_prelude(void* prefs, void* elf)
{
  service::dslutil::updater::initialize();
  return 0;
}

/** @endcond */

#if defined(__cplusplus)
} // extern "C"
#endif
