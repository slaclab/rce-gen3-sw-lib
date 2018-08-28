// -*-Mode: C;-*-
/**
@file
@brief Public definitions used with the dmabuf kernel module under ArchLinux ARM.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#if !defined(TOOL_MEMORY_DMABUF_H)
#define      TOOL_MEMORY_DMABUF_H

#include <asm-generic/ioctl.h>

struct dma_BufferDesc {
  /** @privatesection */
  unsigned long dmaAddress;
  /** @publicsection */
  unsigned long size;
};

typedef struct dma_BufferDesc dma_BufferDesc;

enum {
  DMA_K_MAGIC    = 'c',
  DMA_K_ALLOCATE = _IOWR(DMA_K_MAGIC, 0, dma_BufferDesc),
  DMA_K_FREE     = _IOW( DMA_K_MAGIC, 1, dma_BufferDesc)
};
#endif
