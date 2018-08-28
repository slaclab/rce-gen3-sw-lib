// -*-Mode: C;-*-
/**
@file
@brief Public API for the ARM dynamic linker.
@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#if !defined(TOOL_ELF_LINKER_H)
#define      TOOL_ELF_LINKER_H



#if defined(__cplusplus)
extern "C" {
#endif


#include "elf/lnkStatus.h"

#include "ldr/Ldr.h"

enum lnk_linkerOptions {
  LNK_INSTALL         = 1 << 1
};

Ldr_elf* lnk_load(Ldr_name soname, void* rootPrefs, uint32_t* status, lnk_Status* extra);

/** @cond development */
void* lnk_go(Ldr_elf* elf);
/** @endcond */

void* lnk_lookup(Ldr_elf* elf, const char* name);

#if defined(__cplusplus)
}
#endif
#endif
