// -*-Mode: C;-*-
/**
@file
@brief DMA buffer API for use in user code.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#if !defined(TOOL_MEMORY_DMABUFAPI_H)
#define      TOOL_MEMORY_DMABUFAPI_H

#include "memory/platform/dmabuf.h" // ioctl() commands, buffer descriptor.

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>


#if defined(__cplusplus)
extern "C" {
#endif

static inline int dma_open() {
  int const fd = open("/dev/dmabuf", O_RDWR | O_NONBLOCK);
  return fd;
}

static inline void dma_close(int fd) {
  close(fd);
}

static inline int dma_allocateBuffer(int fd, dma_BufferDesc* desc) {
  return ioctl(fd, DMA_K_ALLOCATE, &desc);
}

static inline int dma_freeBuffer(int fd, const dma_BufferDesc* desc) {
  return ioctl(fd, DMA_K_FREE, &desc);
}

static inline void* dma_mapBuffer(int fd, const dma_BufferDesc* desc) {
  void* dma = mmap(NULL, desc->size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, desc->dmaAddress);
  if (dma == MAP_FAILED) dma = NULL;
  return dma;
}

static inline int dma_unmapBuffer(const dma_BufferDesc* desc, void* vaddr) {
  return munmap(vaddr, desc->size);
}

#if defined(__cplusplus)
} // extern "C"
#endif
#endif
