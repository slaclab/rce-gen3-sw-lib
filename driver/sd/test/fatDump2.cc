// -*-Mode: C++;-*-
/*!
*
* @brief   A module for dumping an SD-card FAT filesystem
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    July 1, 2012 -- Created
*
* $Revision: 3121 $
*
* @verbatim
*                               Copyright 2012
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#include <rtems.h>

#include <stdio.h>
#include <inttypes.h>


#include "fatLib.hh"
#include "debug/print.hh"
#include "time/Time.hh"
#include "conversion/Swap.hh"
using tool::conversion::swap16l;
using tool::conversion::swap32l;

#define MMC_MAX_BLOCK_LEN	512


namespace driver {

  namespace sd {

    namespace test {

void fatDump()
{
  unsigned blkLen, fsBlk;
  MBR      _mbr;
  MBR*     mbr   = &_mbr;
  EBPB     _ebpb;
  EBPB*    ebpb  = &_ebpb;
  MBR32    _mbr32;
  MBR32*   mbr32 = &_mbr32;

  uint8_t* disk  = new uint8_t[MMC_MAX_BLOCK_LEN];
  uint32_t mbrOs = 0;
  printf("\nFirst sector is at offset, adx: %08lx, %p\n", mbrOs, disk);

  FILE*   file = fopen("/dev/sdA", "rb");
  int32_t len  = fread(disk, MMC_MAX_BLOCK_LEN, 1, file);
  if (len != 1)  printf("Reading error: len = %ld\n", len);

  fatMbr(disk, mbr);
  fatEbpb(disk, ebpb);
  fat32Mbr(disk, mbr32);

  if (mbr->secSz != 0) {
    blkLen = mbr->secSz;
    fsBlk  = mbr32->fsInfo;
  } else {
    printf("\nMBR is blank\n");
    blkLen = MMC_MAX_BLOCK_LEN;
    fsBlk  = 1;
  }

  uint8_t* partitions = &disk[blkLen - sizeof(mbr->signature) - 4 * 16];
  uint32_t lbaBegin[4];
  printf("\nPartitions at: %p\n", partitions);
  fatPart(partitions, lbaBegin);
  if (partitions[4] != 0xb)  printf("Error: Unsupported FAT type code %02x\n", partitions[4]);

  uint8_t* fsInfo = new uint8_t[blkLen];
  uint32_t fsInfoOs = blkLen * fsBlk;
  printf("\nFS Info at offset, adx: %08lx, %p\n", fsInfoOs, fsInfo);
  fseek(file, fsInfoOs, SEEK_SET);
  len  = fread(fsInfo, blkLen, 1, file);
  if (len != 1)  printf("Reading error: len = %ld\n", len);
  fatFsInfo(fsInfo);

  uint8_t* part0   = new uint8_t[blkLen];
  uint8_t* fsInfo0 = new uint8_t[mbr->secSz];
  uint8_t* fat1    = new uint8_t[mbr->secSz];
  uint8_t* fat2    = new uint8_t[mbr->secSz];
  uint8_t* dir     = new uint8_t[mbr->secSz];
  unsigned i;
  for (i = 0; i < 2; ++i) {
    uint32_t partOs = lbaBegin[i] * blkLen;
    printf("\nParition %u at offset, adx: %08lx, %p\n", i, partOs, part0);
    fseek(file, partOs, SEEK_SET);
    len  = fread(part0, blkLen, 1, file);
    if (len != 1)  printf("Reading error: len = %ld\n", len);

    printf("\n");

    fatMbr(part0, mbr);
    fatEbpb(part0, ebpb);
    fat32Mbr(part0, mbr32);

    fsInfoOs = partOs + mbr->secSz * mbr32->fsInfo;
    printf("\nPartition %u FS Info at offset, adx: %08lx, %p\n", i, fsInfoOs, fsInfo0);
    fseek(file, fsInfoOs, SEEK_SET);
    len  = fread(fsInfo0, mbr->secSz, 1, file);
    if (len != 1)  printf("Reading error: len = %ld\n", len);
    fatFsInfo(fsInfo0);

    uint32_t fatBeginLba  = lbaBegin[i] + mbr->rsvdSecCnt;
    uint32_t dataBeginLba = fatBeginLba + (mbr->numFATs * mbr32->fatSec);

    uint32_t fat1Os = fatBeginLba * mbr->secSz;
    printf("\nFAT 1 at offset, adx: %08lx, %p\n", fat1Os, fat1);
    fseek(file, fat1Os, SEEK_SET);
    len  = fread(fat1, mbr->secSz, 1, file);
    if (len != 1)  printf("Reading error: len = %ld\n", len);

    uint32_t fat2Os = (fatBeginLba + mbr32->fatSec) * mbr->secSz;
    printf("\nFAT 2 at offset, adx: %08lx, %p\n", fat2Os, fat2);
    fseek(file, fat2Os, SEEK_SET);
    len  = fread(fat2, mbr->secSz, 1, file);
    if (len != 1)  printf("Reading error: len = %ld\n", len);

    uint32_t dirOs = (dataBeginLba + mbr32->rootClus - 2) * mbr->secSz;
    printf("\nRoot dir at offset, adx %08lx, %p:\n", dirOs, dir);
    fseek(file, dirOs, SEEK_SET);
    len  = fread(dir, mbr->secSz, 1, file);
    if (len != 1)  printf("Reading error: len = %ld\n", len);
  }

#if 0                             // Commented out and leaving it as an example
  uint32_t dirEntries = 0;
  uint32_t cl = 0;
  int32_t  sz = 0;
  unsigned n  = 2;                      // File number to look for
  bool     found;
  do
  {
    found = fatDir(dir, &dir[mbr->secSz], &dirEntries);

    int32_t  size;
    uint32_t cluster = find(n, dir, &size);
    if (cluster)
    {
      cl = cluster;
      sz = size;
      break;
    }

    len = fread(dir, mbr->secSz, 1, file);
    if (len != 1)  printf("Reading error: len = %ld\n", len);
  }
  while (!found);
  printf ("Number of directory entries found (including deleted items): %08lx\n",
          dirEntries);

  uint8_t* data   = new uint8_t[mbr->secSz];
  uint32_t dataOs = (dataBeginLba + dirEntries * 32) * mbr->secSz;
  printf("\nData starts at offset, adx: %08lx, %p\n", dataOs, data);
  fseek(file, dataOs, SEEK_SET);
  len  = fread(data, mbr->secSz, 1, file);
  if (len != 1)  printf("Reading error: len = %ld\n", len);

  if (cl)
  {
    printf("File %X.SYS of size %08lx starts at cluster %08lx\n", n, sz, cl);
    printf("Loading...\n"); // The shell prompt appears before loading is complete

    int32_t  size  = sz;
    int32_t  clSz  = mbr->secSz * mbr->secPerClus;
    uint8_t* mem   = new uint8_t[size + clSz]; //0x03000000;
    if (mem == 0)  printf("No space found to load file %X.SYS of size %lu\n", n, sz);
    uint8_t* elf   = mem;

    uint32_t* fat  = (uint32_t*)fat1;

    int      rc    = 0;
    uint32_t frags = 0;
    uint32_t fatOs = -1;
    int32_t  clCnt = 0;
    uint32_t clOs  = cl;
    uint64_t t0    = tool::time::time();
    uint32_t clNext;
    do
    {
      //printf("fatOs = %08lx, cl>>7 = %08lx, ", fatOs, cl >> (9-2));
      if ((cl >> (9-2)) != fatOs)         // Assumes FAT32 & 512 bytes/sector
      {
        fatOs = cl >> (9-2);              // 9: 512, 2: 4 bytes/FAT32 entry
        //printf("fatOs = %08lx, seek = %08lx\n", fatOs, (fatBeginLba + fatOs) * mbr->secSz);
        fseek(file, (fatBeginLba + fatOs) * mbr->secSz, SEEK_SET);
        //printf("seek done\n");
        len  = fread(fat, mbr->secSz, 1, file);
        //printf("read: len = %08lx\n", len);
        if (len != 1)  { --rc; printf("Reading error: len = %ld\n", len);  break; }
      }
      clNext = swap32l(fat[cl & 0x7f]);    // 0x7f = (1<<(9-2))-1
      ++clCnt;
      ++cl;
      //printf("cl = %08lx, clNext = %08lx, clCnt = %08lx\n", cl, clNext, clCnt);
      if (cl != clNext)
      {
        cl = clNext;
        fseek(file, dataBeginLba * mbr->secSz + (clOs - 2) * clSz, SEEK_SET);
        //printf("clSz = %08lx, clCnt = %08lx, totSz = %08lx\n", clSz, clCnt, clSz * clCnt);
        len = fread(mem, clSz, clCnt, file);
        if (len != clCnt) { --rc; printf("Reading error: len = %ld\n", len);  break; }
        mem  += clSz * len;
        sz   -= clSz * len;
        ++frags;
        clOs  = clNext;
        clCnt = 0;
        //printf("len = %08lx, sz = %08lx, clNext = %08lx\n", len, sz, clNext);
      }
    }
    while ((cl > 1) && (cl < 0x0ffffff6u));

    uint64_t t1 = tool::time::time();
    uint64_t dt = t1 - t0;

    printf("\n");

    if ((cl <= 0x0ffffff8u) && (sz > clSz))
    {
      --rc;
      printf("Error: Out of bounds cluster number %08lx or "
             "EOF seen with %08lx = %ld B left to go\n",
             cl, sz, sz);
    }

    printf("File %X.SYS %s (%d) loaded to %p in %lu fragments\n",
           n, rc ? "not" : "successfully", rc, elf, frags);
    printf("  size: %08lx, time: %llu ns, rate: %.2f MB/s\n", size, dt, double(size)/(double(dt)/1000.));

    delete [] mem;
  }
  else
    printf("File %X.SYS not found\n", n);

  delete [] data;
#endif
  delete [] dir;
  delete [] fat2;
  delete [] fat1;
  delete [] fsInfo0;
  delete [] part0;
  delete [] fsInfo;
  delete [] disk;

  fclose(file);

  printf("\n");
}


extern "C" void rce_appmain()
{
  uint64_t t0 = tool::time::lticks();

  fatDump();

  struct timespec t = tool::time::t2s(tool::time::lticks() - t0);

  tool::debug::printv("fatDump2: Exiting; Run time: %u.%03u seconds\n",
                      t.tv_sec, t.tv_nsec / 1000000);
}

    } // test

  } // sd

} // driver
