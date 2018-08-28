// -*-Mode: C;-*-
/**
@file
@brief Simple test for /dev/dmabuf.

The devdmabuf kernel module must be loaded.

@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
DAT

@author
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2014/08/28

@par Last commit:
\$Date: 2014-09-12 12:33:50 -0700 (Fri, 12 Sep 2014) $ by \$Author: tether $.

@par Credits:
SLAC
*/
#include <sys/wait.h>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "memory/platform/dmabufapi.h"


char* map(int fd, const dma_BufferDesc* pbuf) {
  char* const dma = dma_mapBuffer(fd, pbuf);
  if (!dma) {
    perror("dma_mapBuffer()");
    exit(1);
  }
  return dma;
}

int child1(int, const dma_BufferDesc*);
int child2(char*);

int main(void) {
  dma_BufferDesc buffer[2];
  int const fd = dma_open();
  if (fd < 0) {
    perror("dma_open()");
    exit(1);
  }
  buffer[0].size = 4096;
  int status = dma_allocateBuffer(fd, &buffer[0]);
  if (status < 0) {
    perror("Allocation of first buffer");
    exit(1);
  }
  buffer[1].size = 5000;
  status = dma_allocateBuffer(fd, &buffer[1]);
  if (status < 0) {
    perror("Allocation of second buffer");
    exit(1);
  }

  char* const buf0 = map(fd, &buffer[0]);
  strcpy(buf0, "Shared data via explicit mmap().");
  dma_unmapBuffer(&buffer[0], buf0); // Prevent inheritance of mapping.
  child1(fd, &buffer[0]); // But allow inheritance of open fd and descriptor.

  char* const buf1 = map(fd, &buffer[1]);
  strcpy(buf1, "Shared data via inherited mapping.");
  child2(buf1);

  /* Wait for all children to terminate. */
  while (0 < wait(NULL) );

  status = dma_freeBuffer(fd, &buffer[0]);
  if (status < 0) {
    perror("Freeing of first buffer");
    exit(1);
  }
  dma_unmapBuffer(&buffer[1], buf1);
  status = dma_freeBuffer(fd, &buffer[1]);
  if (status < 0) {
    perror("Freeing of second buffer");
    exit(1);
  }
  return 0;
}

int child1(int fd, const dma_BufferDesc* pbuf) {
  /* Use the first DMA buffer via an explicit mapping. */
  int const pid = fork();
  if (pid > 0) return pid;
  char* const dma = map(fd, pbuf);
  printf("%s\n", dma);
  exit(0);
}


int child2(char* dma) {
  /* Use the buffer *dma whose mapping we've inherited. */
  int const pid = fork();
  if (pid > 0) return pid;
  printf("%s\n", dma);
  exit(0);
}
