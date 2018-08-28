// -*-Mode: C;-*-
/**
@file
@brief Map device registers using mmap() under ARM Linux.
@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/


#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include "memory/mem.h"

typedef struct {
  void*        virtualBase;
  size_t       mappedSize;
  size_t const actualSize;
} MappedRegs;

static uint32_t map   (MappedRegs*, const char*);
static void     unmap (MappedRegs*);



static MappedRegs l2cc = {NULL, 0, 0x0f44};
uint32_t  mem_mapL2Controller  (void) {return map  (&l2cc, __func__);}
void      mem_unmapL2Controller(void) {       unmap(&l2cc);          }



static MappedRegs slcr = {NULL, 0, 0x0b78};
uint32_t  mem_mapSlcr  (void) {return map  (&slcr, __func__);}
void      mem_unmapSlcr(void) {       unmap(&slcr);          }



static uint32_t map(MappedRegs* regs, const char* caller) {
  if (! regs->virtualBase) {
    const size_t pageSize = getpagesize();
    regs->mappedSize = pageSize * ((regs->actualSize + pageSize - 1U) / pageSize);
    const int fd   = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
      printf("%s: Opening /dev/mem failed; %s\n", caller, strerror(errno));
      return 0;
    }
    
    regs->virtualBase = mmap(0,
                           regs->mappedSize,
                           PROT_READ  | PROT_WRITE,
                           MAP_SHARED,
                           fd,
                           MEM_L2CC_PHYSICAL_BASE);
    if (regs->virtualBase == MAP_FAILED) {
      printf("%s: mmap() failed; %s\n", caller, strerror(errno));
      regs->virtualBase = NULL;
      close(fd);
      return 0;
    }
    close(fd);
  }
  return (uint32_t)regs->virtualBase;
}

void unmap(MappedRegs* regs) {
  if (regs->virtualBase) {
    munmap(regs->virtualBase, regs->mappedSize);
    regs->virtualBase = NULL;
  }
}
