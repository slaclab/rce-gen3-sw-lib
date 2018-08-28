// -*-Mode: C;-*-
/**
@file
@brief Implement and document atomic resource-set routines.
@verbatim
                               Copyright 2015
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

#include <stdio.h>

#include "debug/print.h"
#include "memory/mem.h"


/** @cond development */
void mem_rsetSetup(void* rset, int numResources, void* base, int stride);

void* mem_rsetAlloc(void* rset);

void mem_rsetFree(void* rset, void*resource);



enum {
  MAX_RESOURCES      = 64,
  MAX_STRIDE         = 26,

  NUM_SETS           = 64,
  SET_STRIDE         =  5,
  SET_SIZE           = (1 << SET_STRIDE),
  SET_SPACE          = NUM_SETS * SET_SIZE,
  SET_REGION         = MEM_REGION_RTS
};


static void* setOfSets = 0;



void mem_rsInitialize(void) {
  // Region allocators return addresses that are page-aligned which
  // is better than the cache-line alignment we require.
  setOfSets = mem_Region_alloc(SET_REGION, SET_SPACE);
  if (!setOfSets) dbg_bugcheck("%s: Unable to get memory for resource sets.\n", __func__);
  // Set up the first set, the master, to manage all the sets including itself.
  mem_rsetSetup(setOfSets, NUM_SETS, setOfSets, SET_STRIDE);
  // Keep users from allocating the master set. We take advantage of mem_rsetAlloc's
  // picking of the lowest-numbered free resource.
  if (!mem_rsetAlloc(setOfSets)) dbg_bugcheck("%s: Unable to reserve the master resource set.\n", __func__);
}

/** @endcond */ // development


void* mem_rsOpen(int numResources, void* base, int stride) {
  if (stride < 0 || stride > MAX_STRIDE) {
    return NULL;
  }
  if (numResources < 1 || numResources > MAX_RESOURCES) {
    return NULL;
  }
  void* const rset = mem_rsetAlloc(setOfSets);
  if (rset) {
    mem_rsetSetup(rset, numResources, base, stride);
  }
  return rset;
}


void mem_rsClose(void** rset) {
  mem_rsetFree(setOfSets, rset);
  *rset = NULL;
}


void* mem_rsAlloc(void* rset) {
  return mem_rsetAlloc(rset);
}


void mem_rsFree(void* rset, void* resource) {
  mem_rsetFree(rset, resource);
}

uint64_t mem_rsGet(void* rset) {
  return *(uint64_t*)rset;
}


/**
   @fn void* mem_rsOpen(int numResources, void* base, int stride)
   @brief Atomically allocate a resource-set to be used for atomic resource management.
   @param[in] numResources the number of resources to be managed by the resource-set,
   between 1 and 64 inclusive.
   @param[in] base the address of the array of resources that will be managed
   @param[in] stride log2(resource size in bytes)
   @return a resource-set handle, or a null pointer if no resources in the set are free

   A resource-set is used to atomically allocate and free a group of resources (objects
   laid out contiguously in memory). The array of resources may have up to 64 elements.
   The storage for this array is allocated by the user.

   A resource-set allocated by this function will initially have all the resources
   in the free state.

   A resource index, from 0 to numResources - 1, when shifted left by stride bits
   must not overflow a 32-bit word. For example with 64 resources the stride may
   be no more than 26 corresponding to 64MB resources.


   @fn void* mem_rsAlloc(void* rset)
   @brief Atomically allocate one of the resources managed by a resource-set.
   @param[in] rset A resource-set handle created by mem_rsOpen()
   @return A pointer to one of the managed resources or a null pointer if none are available


   @fn void mem_rsFree(void* rset, void* resource)
   @brief Atomically free a resource that was allocated using mem_rsAlloc()
   @param[in] rset the same resource-set handle that was used with mem_rsAlloc() when allocating the resource
   now being freed
   @param[in] resource a pointer to the resource being freed


   @fn void mem_rsClose(void** rset)
   @brief Atomically free a resource-set.
   @param[in] rset a pointer to a resource-set handle created by mem_rsAlloc(). After
   this call the handle will be a null pointer.

   @fn uint64_t mem_rsGet(void* rset)
   @brief Return the 64-bit resource-set allocation state.
   @param[in] rset A resource-set handle created by mem_rsOpen()
   @return A 64-bit value representing the resource-set allocation state.

   @if development
   @fn void mem_rsInitialize(void)

   @brief To be called during system startup after RTEMS has been
   loaded, not by users. Initialize the resource-set library.
   @endif
 */

