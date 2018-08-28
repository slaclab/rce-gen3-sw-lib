// -*-Mode: C;-*-
/**
@file
@brief Declaration of the "mem" package atomic resource management functions.
@verbatim
                               Copyright 2017
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#if !defined(TOOL_MEMORY_ARMCA9_RESOURCES_H)
#define      TOOL_MEMORY_ARMCA9_RESOURCES_H

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

void* mem_rsAlloc(void* rset);

void  mem_rsFree (void* rset, void* resource);

void*  mem_rsOpen(int numResources, void* base, int stride);

void  mem_rsClose(void** rset);

uint64_t mem_rsGet(void* rset);

#if defined(__cplusplus)
}
#endif

#endif
