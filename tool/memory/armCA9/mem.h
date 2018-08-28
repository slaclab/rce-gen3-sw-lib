// -*-Mode: C;-*-
/**
@file
@brief Some basic MMU and cache operations.

The operations declared in this file will be implemented without using
any operating system facilities, so they will be usable even when no
OS has yet been started.

@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

*/
#if !defined(TOOL_MEMORY_ARMCA9_MEM_H)
#define      TOOL_MEMORY_ARMCA9_MEM_H

#include <inttypes.h>

#if defined(__cplusplus)
extern "C" {
#endif

enum mem_AttributeFlag {
  MEM_READ       = 1 << 0,
  MEM_WRITE      = 1 << 1,
  MEM_EXEC       = 1 << 2,
  MEM_CACHED     = 1 << 3,
  MEM_DEVICE     = 1 << 4,
  MEM_HANDSHAKE  = 1 << 5,
  MEM_SHARED     = 1 << 6
};

typedef enum mem_AttributeFlag mem_AttributeFlag;

enum mem_UseType {
  MEM_RESERVED    = 0,
  MEM_NORMAL      = MEM_READ | MEM_WRITE | MEM_EXEC | MEM_CACHED,
  MEM_TEXT        = MEM_READ | MEM_EXEC | MEM_CACHED,
  MEM_RWDATA      = MEM_READ | MEM_WRITE | MEM_CACHED,
  MEM_RODATA      = MEM_READ | MEM_CACHED,
  MEM_UNCACHED    = MEM_READ | MEM_WRITE,
  MEM_MMU         = MEM_READ | MEM_HANDSHAKE,
  MEM_REGISTERS   = MEM_READ | MEM_WRITE | MEM_DEVICE | MEM_SHARED,
  MEM_ROREGISTERS = MEM_READ | MEM_DEVICE | MEM_SHARED
};

typedef enum mem_UseType mem_UseType;

static inline uint32_t mem_getPhysRead        (uint32_t virt)             __attribute__((always_inline));
static inline uint32_t mem_getPhysWrite       (uint32_t virt)             __attribute__((always_inline));
static inline int      mem_getFlagsRead       (uint32_t virt)             __attribute__((always_inline));
static inline int      mem_getFlagsWrite      (uint32_t virt)             __attribute__((always_inline));
static inline uint32_t mem_getPhysFlagsRead   (uint32_t virt, int* flags) __attribute__((always_inline));
static inline uint32_t mem_getPhysFlagsWrite  (uint32_t virt, int* flags) __attribute__((always_inline));

void mem_setFlags(uint32_t beginVirt, uint32_t endVirt, int flags);

int mem_explode(uint32_t virtBegin, uint32_t virtEnd);

/** @cond development */
int mem_setupMmuTable();

void mem_printMmuTable(const uint32_t* tableStart, void (*printFunction)(const char* fmt, ...));
/** @endcond */


void mem_storeDataCacheRange(uint32_t beginVirt, uint32_t endVirt, uint32_t l2Base);

void mem_storeDataCache(uint32_t l2Base);

void mem_invalidateDataCacheRange(uint32_t beginVirt, uint32_t endVirt, uint32_t l2Base);

void mem_invalidateDataCache(uint32_t l2Base);

void mem_invalidateInstructionCacheRange(uint32_t beginVirt, uint32_t endVirt);

void mem_invalidateInstructionCache(void);

/** @cond development */
#define MEM_L2CC_PHYSICAL_BASE 0xF8F02000U
/** @endcond */

uint32_t mem_mapL2Controller(void);

void    mem_unmapL2Controller(void);

/** @cond development */
#define MEM_SLCR_PHYSICAL_BASE 0xF8000000U
/** @endcond */

uint32_t mem_mapSlcr(void);

void mem_unmapSlcr(void);

enum  mem_RegionName {
  MEM_REGION_SYSLOG,
  MEM_REGION_MMU,
  MEM_REGION_RTS,
  MEM_REGION_WORKSPACE,
  MEM_REGION_UNCACHED,
  MEM_REGION_SOCKET,
  MEM_REGION_AXI0_TEST,
  MEM_REGION_FW_VER_CTL,
  MEM_REGION_BSI,
  MEM_REGION_AXI1_TEST,
  MEM_REGION_USERDEV,
  MEM_REGION_USERPPI,
  MEM_REGION_IOP,
  MEM_REGION_STATIC,
  MEM_REGION_HIGHREG,
  MEM_REGION_OCM
};

typedef enum  mem_RegionName  mem_RegionName;

char* mem_Region_alloc(int name, uint32_t numBytes);

void  mem_Region_undoAlloc(int name, char* allocPtr);

char* mem_Region_startAddress(int name);

uint32_t mem_Region_remaining(int name);

uint32_t mem_Region_size(int name);

uint32_t mem_Region_pageSize(int name);

uint32_t mem_Region_workspaceHoldback(void);

void mem_Region_printConfig(void (*printFunction)(const char* fmt, ...));

#if defined(__cplusplus)
} // extern "C"
#endif

#include "memory/cpu/memFlags-inl.h"

#endif
