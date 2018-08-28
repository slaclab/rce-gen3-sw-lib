// -*-Mode: C++;-*-
//
//                            Copyright 2014
//                                 by
//                    The Board of Trustees of the
//                 Leland Stanford Junior University.
//                        All rights reserved.
//

#include "dslutil/id_init.hh"

#if defined(__cplusplus)
extern "C" {
#endif

/** @cond development */


/** @brief Initialize the identifier system on module load (RTEMS)
 */
extern "C"
int lnk_prelude(void* prefs, void* elf)
{
  service::dslutil::identifier::initialize();
  return 0;
}

/** @endcond */

#if defined(__cplusplus)
} // extern "C"
#endif
