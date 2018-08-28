#include <inttypes.h>
#include <stdio.h>

#include "task/Task.h"

#include "memory/resources.h"

typedef struct { // For debugging purposes only.
  uint64_t bitset;
  uint32_t filler[4]; 
  void* base;
  unsigned stride;
} ResSet;

void mem_rsInitialize(); // Normally called only at startup.


// This shared object made forthis test should include its own
// copies of memResource.o and memResourceHelper.o so that the call of
// mem_rsInitialize() doesn't use or affect the resource library compiled into
// rtems.so. It will however allocate some space in the RTS (or perhaps WORKSPACE)
// Region.

enum {NUM_RESOURCES = 64, RESOURCE_STRIDE = 2/* log2(sizeof resource[0]) */};

void Task_Start(int argc, const char** argv) {
  int i;
  int resource[NUM_RESOURCES];
  void* rset;

  mem_rsInitialize();
  for (i = 0; i < NUM_RESOURCES; ++i) resource[i] = i;

  printf("\nShould get all resources 0 through 63 inclusive.\n");

  rset = mem_rsOpen(NUM_RESOURCES, &resource[0], RESOURCE_STRIDE);
  if (!rset) {
    printf("ERROR: mem_rsOpen() failed.\n");
  }

  for (i = 0; i < NUM_RESOURCES; ++i) {
    printf("Set before allocation = 0x%016llx\n", ((ResSet*)rset)->bitset);
    int* const pr = (int*)mem_rsAlloc(rset);
    printf("Set after allocation = 0x%016llx\n", ((ResSet*)rset)->bitset);
    if (pr) printf("Allocated resource %d\n", *pr);
    else    printf("Allocation failed\n");
  }

  printf("\nThe next allocation should fail.\n");
  if (mem_rsAlloc(rset)) printf("ERROR: it succeeded.\n");
  else printf("Yes, it did.\n");

  printf("\nNow we should get only the odd-numbered resources.\n");
  for (i = 1; i < NUM_RESOURCES; i += 2) {
    mem_rsFree(rset, &resource[i]);
  }
  for (i = 1; i < NUM_RESOURCES; i += 2) {
    //printf("Set before allocation = 0x%016llx\n", ((ResSet*)rset)->bitset);
    int* const pr = (int*)mem_rsAlloc(rset);
    //printf("Set after allocation = 0x%016llx\n", ((ResSet*)rset)->bitset);
    if (pr) printf("Allocated resource %d\n", *pr);
    else    printf("Allocation failed\n");
  }

  printf("\nNow test the case where the only free resource\n");
  printf("has index number < 32.\n");

  mem_rsFree(rset, &resource[0]);
  if (&resource[0] == mem_rsAlloc(rset)) {
    printf("Got resource 0. Now make sure the set is empty.\n");
    if (mem_rsAlloc(rset)) printf("ERROR: the set wasn't empty.\n");
    else printf("Yes, it is.\n");
  }
  else {
    printf("ERROR: allocation failed or didn't get resource 0.\n");
  }
  mem_rsClose(&rset);
  if (rset) printf("\nERROR: mem_rsClose() didn't set the set pointer to NULL.\n");
}

void Task_Rundown(void) {
  printf("testMemRes completed.\n");
}
