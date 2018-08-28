// -*-Mode: C;-*-
/**
@file
@brief RTEMS-only code for the "mem" library. Custom stack allocation.


@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

/*

There are two use cases for stack allocation:

(1) Non-task initialization code requests stack space for tasks,
e.g., init tasks.

(2) A task requests stack space for a child task, usually by
calling rtems_task_create().

For case (1) we can't use most RTEMS directives nor most library
functions since these are written assuming that there is a current
task. Often what happens is that the function in question takes a
mutex of some kind and the mutex code tries to modify the Task
Control Block of the current task. Since the "current TCB" pointer
is zero you wind up with an attempt to modify low memory. The RTEMS
internal Heap implementation does work in these circumstances,
however. The solution adapted here is to use mem_Region_alloc()
to get a block of memory and to make a Heap out of that.

This case doesn't need mutual exclusion since RTEMS makes sure that
there is only one initialization sequence even when SMP is on. It
had better!

Case (2) does require some kind of exclusion in the general
case. Although RCE application code uses non-preemptive threading,
it seems that some preemptive threads are created by RTEMS
itself. At least, failing to provide exclusion causes the system to
lock up shortly after the shell prompt appears, while forcing stack
allocation requests to proceed one at a time using GDB breakpoints
made this problem go away.

We could use a semaphore for exclusion in this case, but when can
we create it? Even if we delay the creation until the first task
makes a request, the creation itself would still be subject to a
race condition.  It's at times like this that I miss the statically
initialized mutexes offered by POSIX threads.

The only solution I can think of is to use a hardware spinlock
implemented using the GCC atomic test-and-set builtin. We can yield
the CPU inside the spin loop in the hopes that we can proceed the
next time our thread gets to execute. We could use the spinlock to
guard the creation of a proper semaphore but having both a spinlock
and a semaphore seems like a redundancy and complicates the code
besides.
*/
   
#include <stdlib.h>
#include <string.h>

#include <rtems.h>
#include <rtems/score/heap.h>
#include <rtems/score/heapimpl.h>
#include <rtems/system.h>

#include <concurrency/BasicMutex.h>

#include <debug/print.h>

#include <memory/mem.h>


/* The implementation of the critical section markers. We don't turn
   off interrupts in a critical section since we assume that no ISR
   will ever try to allocate task stack space.
*/
static BasicMutex mutex = BASIC_MUTEX_INITIALIZER;

static inline void beginCritical(void) {
  if (_Thread_Executing) {
    while (!BasicMutex_trylock(&mutex)) {
      rtems_task_wake_after(RTEMS_YIELD_PROCESSOR);
    }
  }
}

static inline void endCritical(const char *errorMsg, const char* func) {
  if(_Thread_Executing) {
    BasicMutex_unlock(&mutex);
  }
  if (errorMsg) dbg_bugcheck("%s %s: %s\n", __FILE__, func, errorMsg);
}


#define POOL_ALIGNMENT  ((size_t)0x100000U)
#define POOL_ROUND      (POOL_ALIGNMENT - 1U)
#define PAGE_SIZE       ((size_t)0x1000U)
#define PAGE_ROUND      (PAGE_SIZE - 1U)



/* The size in bytes of the common pool for task stacks. */
static size_t poolSize = 0;

void mem_initStackAlloc(size_t psize) {
  poolSize = (psize + POOL_ROUND) & ~POOL_ROUND;
}



/* The heap used to allocate the actual stacks. */
static Heap_Control poolHeap;

/* The location of the pool. */
static void* poolAddr = NULL;

static void init(void);

void* mem_allocStack(size_t stackSize) {
  beginCritical();

  /* Delayed initialization. We want to perform the initialization as
     late as possible in the RTEMS startup sequence in order to ensure
     that the facilities we want to use will be available. Enabling
     multitasking is just about the last thing done and we assume that
     allocating the first task stack comes at about that time.
  */
  if (!poolAddr) init();

  /* Add the sizes of the inaccessible regions to the request. */
  const size_t segmentSize = ((stackSize+PAGE_ROUND) & ~PAGE_ROUND) + 2 * PAGE_SIZE;

  /* Allocate the stack from the pool Heap. We may get a little more
     space than we requested but we ignore any such extra space.
  */
  void *segmentAddr = _Heap_Allocate_aligned_with_boundary(&poolHeap, segmentSize, PAGE_SIZE, 0);
  if(!segmentAddr)
  {
    endCritical("Could not allocate a task stack.", __FUNCTION__);
  }

  endCritical(NULL, NULL);

  /* dbg_printv("%s %s: Allocated segment of %lu bytes at %p. %s.\n", */
  /*            __FILE__, */
  /*            __FUNCTION__, */
  /*            (unsigned long)segmentSize, */
  /*            segmentAddr, */
  /*            (_Thread_Executing) ? "In-task" : "Out of task"); */

  const uint32_t usegAddr = (uint32_t)segmentAddr;

  /* Store the segment size for use by the deallocator. Make sure it
     gets to memory because cache line invalidations are coming next.
  */
  const uint32_t l2base = mem_mapL2Controller();
  *((size_t*)segmentAddr) = segmentSize;
  mem_storeDataCacheRange(usegAddr, usegAddr + sizeof segmentSize, l2base);

  /* Invalidate cache lines for the inaccessible regions. Can't have
     cache writes to un-writeable memory! Except for our stored
     segment size we don't care whether the RAM content is stale.
  */
  mem_invalidateDataCacheRange(usegAddr, usegAddr + PAGE_SIZE, l2base);
  mem_invalidateDataCacheRange(usegAddr + segmentSize - PAGE_SIZE, usegAddr + segmentSize, l2base);

  /* Make the first and last pages of the segment inaccessible. */
  mem_setFlags(usegAddr, usegAddr + PAGE_SIZE, MEM_RESERVED);
  mem_setFlags(usegAddr + segmentSize - PAGE_SIZE, usegAddr + segmentSize, MEM_RESERVED);

  return segmentAddr + PAGE_SIZE;
}


static void init() {

  poolAddr = mem_Region_alloc(MEM_REGION_WORKSPACE, poolSize);
  if (!poolAddr) {
    dbg_bugcheck("%s %s: Can't allocate a block of memory for task stacks.\n", __FILE__, __FUNCTION__);
  }

  /* Change the page size of the allocated memory to 4 KB. */
  if (!mem_explode((uint32_t)poolAddr, (uint32_t)poolAddr + poolSize)) {
    dbg_bugcheck("%s %s: Can't change the page size of task stack memory.\n", __FILE__, __FUNCTION__);
  }

  uintptr_t firstBlockSize = _Heap_Initialize(&poolHeap, poolAddr, poolSize, PAGE_SIZE);

  if (!firstBlockSize) {
    dbg_bugcheck("%s %s: Can't set up the Heap used for stacks.\n", __FILE__, __FUNCTION__);
  }
}



void  mem_deallocStack(void*  stackAddr) {

  /* Find the true beginning of the segment and retrieve the segment
     size saved by mem_allocStack().
  */
  void* const segmentAddr = stackAddr - PAGE_SIZE;
  uint32_t usegAddr = (uint32_t)segmentAddr;
  mem_setFlags(usegAddr, usegAddr + PAGE_SIZE, MEM_RWDATA);
  size_t segmentSize = *((size_t*)segmentAddr);

  /* dbg_printv("%s %s: Deallocate segment of %lu bytes at %p. %s.\n", */
  /*            __FILE__, */
  /*            __FUNCTION__, */
  /*            (unsigned long)segmentSize, */
  /*            segmentAddr, */
  /*            (_Thread_Executing) ? "In-task" : "Out of task"); */

  /* Fix access rights at the top. */
  mem_setFlags(usegAddr + segmentSize - PAGE_SIZE, usegAddr + segmentSize, MEM_RWDATA);

  /* Return the segment to the pool. */
  beginCritical();

  if (!_Heap_Free(&poolHeap, segmentAddr))
  {
    endCritical("Could not free a task stack.", __FUNCTION__);
  }

  endCritical(NULL, NULL);
}


void mem_getStackHeapInfo(Heap_Information_block *info) {
  beginCritical();
  if (poolAddr) {
    _Heap_Get_information(&poolHeap, info);
  }
  else {
    memset(info, 0, sizeof *info);
  }
  endCritical(NULL, NULL);
}



// ========== API documentation ==========

/** @cond development */

/** @fn void mem_initStackAlloc(size_t psize)
    @brief Set the size of the stack memory pool.
    @param[in] pSize  The total size of the stack space in
    bytes. Will be rounded up to a multiple of 1 MB.

    The total amount of space for stacks is determined by RTEMS from
    its configuration.  You can control this directly by setting
    CONFIGURE_STACK_SPACE_SIZE. Otherwise RTEMS will use approximately

    CONFIGURE_MAXIMUM_TASKS * RTEMS_MINIMUM_STACK_SIZE + CONFIGURE_EXTRA_TASK_STACKS

    This function will be called rather early in the RTEMS
    initialization sequence, just after the initialization of RTEMS
    data structures.
*/



/** @fn void* mem_allocStack(size_t stackSize)
    @brief Allocate a task stack.
    @param[in] stackSize The size of the stack in bytes. Will be
    rounded up to a multiple of 4 KB.
    @return The lowest address of the accessible stack memory.

    Each stack will have some no-access storage immediately above and
    below it, in order to cause access exceptions on many cases of
    stack overflow and underflow.  The stack memory itself will be
    readable, writeable, cached and non-executable.

*/



/** @fn mem_deallocStack(void* stackAddr)
    @brief Return a stack's space to the pool.
    @param[in] stackAddr The stack's low address as returned by mem_allocStack().
*/



/* @fn void mem_getStackHeapInfo(Heap_Information_block *info)
   @brief Fill a Heap_Info structure with info about the heap used for stacks.
   @param[out] info A pointer to an RTEMS Heap Information Block.
   Will be set to all zeros if no stack heap has yet been created.
*/

/** @endcond */
