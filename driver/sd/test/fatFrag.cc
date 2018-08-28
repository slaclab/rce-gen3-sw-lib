// -*-Mode: C++;-*-
/*!
*
* @brief   A module for determining the number of fragments a file in the first
*          partition of an SD-card FAT filesystem is composed of.  A fragment
*          is defined to be a set of contiguous clusters.
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    November 7, 2012 -- Created
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
#include <stdio.h>
#include <inttypes.h>


#include "fatLib.hh"
#include "conversion/Swap.hh"
using tool::conversion::swap16l;
using tool::conversion::swap32l;


extern "C" void rce_appmain(uintptr_t)
{
  MBR    _mbr;
  MBR*   mbr   = &_mbr;
  MBR32  _mbr32;
  MBR32* mbr32 = &_mbr32;

  uint8_t* disk  = new uint8_t[512];
  uint32_t mbrOs = 0;
  printf("\nFirst sector is at offset, adx: %08lx, %p\n", mbrOs, disk);

  FILE*   file = fopen("/dev/sdA2", "rb");
  int32_t len  = fread(disk, 512, 1, file);
  if (len != 1)  printf("Reading error: len = %ld\n", len);

  fatMbr(disk, mbr);
  fat32Mbr(disk, mbr32);

  if (mbr->signature != 0x55aa)  printf ("Incorrect signature %02x\n", mbr->signature);

  uint32_t fatBeginLba  = mbr->rsvdSecCnt;
  uint32_t dataBeginLba = fatBeginLba + (mbr->numFATs * mbr32->fatSec);

  uint8_t* dir   = disk;
  uint32_t dirOs = (dataBeginLba + mbr32->rootClus - 2) * mbr->secSz;
  printf("\nRoot dir at offset, adx %08lx, %p:\n", dirOs, dir);
  fseek(file, dirOs, SEEK_SET);
  len  = fread(dir, mbr->secSz, 1, file);
  if (len != 1)  printf("Reading error: len = %ld\n", len);

  uint32_t dirEntries = 0;
  uint32_t cl = 0;
  int32_t  sz = 0;
  unsigned n  = 0x2;                      // File number to look for
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
  printf ("Number of directory entries found (including deleted items): %lu\n",
          dirEntries);

  if (cl)
  {
    printf("File %X.SYS of size 0x%08lx starts at cluster %08lx\n", n, sz, cl);

    int32_t  clSz  = mbr->secSz * mbr->secPerClus;
    uint32_t* fat  = (uint32_t*)disk;

    int      rc    = 0;
    uint32_t frags = 0;
    uint32_t fatOs = -1;
    int32_t  clCnt = 0;
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
        //printf("clSz = %08lx, clCnt = %08lx, totSz = %08lx\n", clSz, clCnt, clSz * clCnt);
        sz   -= clSz * len;
        ++frags;
        clCnt = 0;
        //printf("len = %08lx, sz = %08lx, clNext = %08lx\n", len, sz, clNext);
      }
    }
    while ((cl > 1) && (cl < 0x0ffffff6u));

    printf("\n");

    if ((cl <= 0x0ffffff8u) && (sz > clSz))
    {
      --rc;
      printf("Error: Out of bounds cluster number %08lx or "
             "EOF seen with %08lx = %ld B left to go\n",
             cl, sz, sz);
    }

    //printf("Cluster count = %08lx, sz = %08lx, clSz = %08lx\n", clCnt, sz, clSz);

    printf("File %X.SYS is made up of %lu fragment%c\n",
           n, frags, "s"[frags==1]);
  }
  else
    printf("File %X.SYS not found\n", n);

  delete [] disk;

  fclose(file);

  printf("\n");
}
