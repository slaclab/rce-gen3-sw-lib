// -*-Mode: C;-*-
/**
@file Hash.h
@brief This is a public interface to the hashing functions.
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

#ifndef HASH
#define HASH

#include <inttypes.h>
#include <stddef.h>

#define HASH_SEED ((uint32_t)1771875) // Useful seed value for the hash function below...

#if defined(__cplusplus)
extern "C" {
#endif

uint64_t HashWord(const uint32_t* vector, uint32_t length, uint32_t seed); 

uint64_t HashString(const char*  vector, uint32_t seed); 

uint64_t Hash64_32(uint64_t, uint32_t);  

#if defined(__cplusplus)
} // extern "C"
#endif

#endif
