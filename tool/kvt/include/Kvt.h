// -*-Mode: C;-*-
/**
@file
@brief This is the PUBLIC interface to the Key-Value-Table (Kvt) facility. 
@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@author
Michael Huffer, SLAC (mehsys@slac.stanford.edu)

@par Date created:
2013/10/28

@par Credits:
SLAC

*/

#ifndef KVT
#define KVT

#include <inttypes.h>
#include <stddef.h>

typedef uint64_t KvtKey;
typedef void*    KvtValue;
typedef void*    KvTable;

#if defined(__cplusplus)
extern "C" {
#endif

KvtKey KvtHash(const char* string);

uint32_t KvtSizeof(uint32_t max_entries);

KvTable KvtConstruct(uint32_t max_entries, void* buffer);

KvtValue KvtInsert(KvtKey, KvtValue, KvTable);

KvtValue KvtLookup(KvtKey, KvTable);

KvtValue KvtRename(KvtKey, KvtValue, KvTable);

KvtValue KvtRemove(KvtKey, KvTable);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif
