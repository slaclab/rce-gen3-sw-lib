// -*-Mode: C;-*-
/**
@file
@brief For test purposes implement an ELF loader.
@verbatim
                               Copyright 2013
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
2013/11/26

@par Last commit:
\$Date: 2014-04-07 21:30:01 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3125 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/elf/test/linux/loadElf.c $

@par Credits:
SLAC
*/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#include "elf/baseTypes.h"
#include "elf/Object.h"

typedef struct {
  FILE*      objFile;
  unsigned   numPhdr;
  unsigned   phdrNum;
  elf_Header header;
  unsigned   lowVaddr;
  unsigned   highVaddr;
  unsigned   memSize;
  char*      elfBase;
  char*      address;
} ElfInfo;


static void  read(void* dest, size_t unitSize, unsigned numUnits, FILE* f);
static char* allocate(size_t n);
static void  loadSegment(ElfInfo *info);

const char* loadElf(const char* filename, unsigned *size) {
  ElfInfo info;
  info.numPhdr  = 0;
  info.phdrNum  = 0;
  info.lowVaddr = 0xffffffffU;
  info.highVaddr= 0;
  info.memSize  = 0;
  info.elfBase  = NULL;
  info.address  = NULL;
  info.objFile  = fopen(filename, "r");
  if (!info.objFile) {
    printf("Could not open %s: %s.\n", filename, strerror(errno));
    exit(1);
  }

  printf("\nReading ELF header for %s.\n", filename);
  read(&info.header, sizeof info.header, 1, info.objFile);
  info.numPhdr = elf_Header_numPhdr(&info.header);

  fseek(info.objFile, elf_Header_offPhdr(&info.header), SEEK_SET);
  loadSegment(&info);
  *size = info.memSize;
  return info.address;
}

void read(void* dest, size_t unitSize, unsigned numUnits, FILE* f) {
  if (numUnits != fread(dest, unitSize, numUnits, f)) {
    if (feof(f)) printf("Premature EOF.\n");
    else         printf("Read error: %s\n", strerror(errno));
    exit(1);
  }
}

char* allocate(size_t n) {
  void* p;
  int err;
  if ((err = posix_memalign(&p, 4096, n))) {
    printf("Allocation failed: %s\n", strerror(err));
    exit(1);
  }
  return (char*)p;
}

void loadSegment(ElfInfo *info) {
  /* The file is positioned at the next program header or after the last program header. */
  /* Going down we examine the program headers to find the lowest vaddr and the total memsize.
     Coming back up we load the segments.
  */
  unsigned const phdrNum = info->phdrNum;
  if (phdrNum < info->numPhdr) {
    elf_Pheader phdr;
    printf("Reading program header %u at offset 0x%x. ", phdrNum, (unsigned)ftell(info->objFile));
    read(&phdr, sizeof phdr, 1, info->objFile);
    if (elf_Pheader_isLoadable(&phdr)) {
      printf("Loadable.\n");
      printf("    off 0x%x  fsize 0x%x  vaddr 0x%x  memsize 0x%x.\n",
             elf_Pheader_fileOffset(&phdr),
             elf_Pheader_fileSize(&phdr),
             elf_Pheader_virtAddr(&phdr),
             elf_Pheader_memSize(&phdr));
      const unsigned vaddr  = elf_Pheader_virtAddr(&phdr);
      const unsigned mvaddr = vaddr +  elf_Pheader_memSize(&phdr);
      info->lowVaddr  = vaddr  < info->lowVaddr  ? vaddr  : info->lowVaddr;
      info->highVaddr = mvaddr > info->highVaddr ? mvaddr : info->highVaddr;
      info->memSize += elf_Pheader_memSize(&phdr);
    }
    else {
      printf("\n");
    }
    ++info->phdrNum;
    /* Next program header. */
    loadSegment(info);

    /* Read a loadable segment and clear the last (memsize - filesize) bytes.*/
    if (elf_Pheader_isLoadable(&phdr)) {
      char*    const addr  = info->elfBase  + elf_Pheader_virtAddr(&phdr);
      unsigned const fsize = elf_Pheader_fileSize(&phdr);
      unsigned const msize = elf_Pheader_memSize(&phdr);
      printf("Loading segment %u.", phdrNum);
      printf("    off 0x%x  fsize 0x%x  vaddr 0x%x  memsize 0x%x.\n",
             elf_Pheader_fileOffset(&phdr),
             elf_Pheader_fileSize(&phdr),
             elf_Pheader_virtAddr(&phdr),
             elf_Pheader_memSize(&phdr));
      fseek(info->objFile, elf_Pheader_fileOffset(&phdr), SEEK_SET);
      printf("    Reading from file offset 0x%x.\n", (unsigned)ftell(info->objFile));
      printf("    Loading at %p.\n", addr);
      read(addr, fsize, 1, info->objFile);
      memset(addr + fsize, '\0',  msize - fsize);
    }
  }/* if (phdrNum < info->numPhdr) ... */
  else {
    /* We've examined all the program headers. Anything loadable? */
    if (info->memSize > 0) {
      const unsigned size = info->highVaddr - info->lowVaddr;
      printf("Total memory requirement is 0x%x bytes.\n", size);
      info->address = allocate(size);
      info->elfBase = info->address - info->lowVaddr;
      printf("ELF object loaded at %p, memory base address = %p.\n", info->address, info->elfBase);
    }
    else {
      printf("The ELF file has no loadable segments.\n");
      exit(1);
    }
  }
}
