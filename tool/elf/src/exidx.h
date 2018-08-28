// -*-Mode: C;-*-
/**
@file
@brief Declare functions exported by exidx.c.

@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

#if !defined(TOOL_ELF_EXIDX_H) 
#define      TOOL_ELF_EXIDX_H

#include "elf/Object.h"

#if defined(__cplusplus)
extern "C" {
#endif

  /** @brief Record the address and number of entries for any EXIDX segment
      that a loaded shared object might have.

      @param[in] elfObj Pointer to an elf_Object describing the shared object.
  */
  void rememberExidx(const elf_Object* elfObj);

#if defined(__cplusplus)
}
#endif

#endif
