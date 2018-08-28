// -*-Mode: C;-*-
/**
@file Svt.h
@brief This is the PUBLIC interface to the SVT facility.
@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@author
Michael Huffer, SLAC (mehsys@slac.stanford.edu)

@par Credits:
SLAC
*/

#ifndef TOOL_SVT_SVT_H
#define TOOL_SVT_SVT_H

#include "system/statusCode.h"
#include "elf/elf32.h"

#include <inttypes.h>
#include <stddef.h>

#define SVT 2 /* Facility no., see configuration/system/facility.doc. */

#define SVT_MAX_TABLES  32

#define SVT_SYS_TABLE_NUM 31
#define SVT_APP_TABLE_NUM 30

#define SVT_SYS_TABLE (1 << SVT_SYS_TABLE_NUM)
#define SVT_APP_TABLE (1 << SVT_APP_TABLE_NUM)
#define SVT_ANY_TABLE 0xFFFFFFFF

typedef enum {
 SVT_SUCCESS             = STS_K_SUCCESS,  // Zero is always success, non-zero an error...
 SVT_INVALID_TABLE       = STS_ENCODE(SVT, 1),
 SVT_DUPLICATE           = STS_ENCODE(SVT, 2),
 SVT_IMAGE_NOT_TABLE     = STS_ENCODE(SVT, 3),
 SVT_NOSUCH_SYMBOL       = STS_ENCODE(SVT, 4) 
 } Svt_status;
 
typedef const char* Svt_symbol;  // Used to identify symbols for name table lookups...

#if defined(__cplusplus)
extern "C" {
#endif

void* Svt_Translate(Svt_symbol, uint32_t tables);

uint32_t Svt_Install(uint32_t table, const char* image);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif
