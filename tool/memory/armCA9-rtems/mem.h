// -*-Mode: C;-*-
/**
@file
@brief RTEMS-only operations for the "mem" subpackage.

RTEMS callbacks used to control task stack allocation. Stack allocation
information getter used by a shell command.

@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#if !defined(TOOL_MEMORY_ARMCA9_RTEMS_MEM_H)
#define      TOOL_MEMORY_ARMCA9_RTEMS_MEM_H

#include <rtems/score/heap.h> /* For Heap_Information_block typedef.*/

#if defined(__cplusplus)
extern "C" {
#endif

  void mem_initStackAlloc(size_t psize);

  void* mem_allocStack(size_t stackSize);

  void mem_deallocStack(void* stackAddr);

  void mem_getStackHeapInfo(Heap_Information_block *info);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif
