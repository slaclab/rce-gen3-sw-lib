// -*-Mode: C++;-*-
/*!
*
* @brief   A module for dumping an in-memory (e.g., ramdisk) FAT filesystem
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
#include <stdio.h>
#include <inttypes.h>


#include "fatLib.hh"
#include "conversion/Swap.hh"
using tool::conversion::swap16l;
using tool::conversion::swap32l;

int load(unsigned n, uint8_t* mem, int32_t clSz, uint8_t* fat, uint8_t* dir, uint8_t* data);
uint8_t* mcpy(uint8_t* mem, uint8_t* data, uint32_t sz);


void fatDump()
{
  MBR    _mbr;
  MBR*   mbr   = &_mbr;
  MBR32  _mbr32;
  MBR32* mbr32 = &_mbr32;

  uint8_t* disk = (uint8_t*)0x6f000000;

  fatMbr(disk, mbr);
  fat32Mbr(disk, mbr32);

  uint8_t* partitions = &disk[mbr->secSz - sizeof(mbr->signature) - 4 * 16];
  uint32_t lbaBegin[4];

  fatPart(partitions, lbaBegin);

  uint8_t* volId = &disk[lbaBegin[0] * mbr->secSz];
  printf("VolumeID at: %p\n", volId);

  uint8_t* fat1 = &disk[mbr->rsvdSecCnt * mbr->secSz];
  printf("FAT 1 at: %p\n", fat1);
  uint8_t* fat2 = &fat1[mbr32->fatSec * mbr->secSz];
  printf("FAT 2 at: %p\n", fat2);
  uint8_t* dir  = &fat2[mbr32->fatSec * mbr->secSz];

  printf("Root dir at %p:\n", dir);
  uint32_t dirEntries = 0;
  fatDir(dir, &dir[mbr->secSz], &dirEntries);

  uint8_t* data = &dir[mbr->dirSz * 32];
  printf("Data starts at: %p\n", data);
  printf("First data word = %s\n", data);

  int32_t  clSz = mbr->secSz * mbr->secPerClus;
  unsigned n    = 1;                    // File number to load
  uint8_t* mem  = (uint8_t*)0x03000000;

  int rc = load(n, mem, clSz, fat1, dir, data);

  printf("File %03X.SYS %ssuccessfully (%d) loaded to %p\n",
         n, rc ? "un" : "", rc, mem);

  printf("\n");
}


uint8_t* mcpy(uint8_t* mem, uint8_t* data, uint32_t sz)
{
  uint32_t  l32 = sz >> 2;
  uint32_t* d32 = (uint32_t*)data;
  uint32_t* m32 = (uint32_t*)mem;

  while (l32--)  *m32++ = *d32++;

  mem  = (uint8_t*)m32;
  data = (uint8_t*)d32;
  switch (sz & 0x3)
  {
    case 3: *mem++  = *data++;
    case 2: *mem++  = *data++;
    case 1: *mem++  = *data++;
    case 0: return mem;
  }

  if (sz!= 0)  printf("bugcheck: sz > 3: %lu", sz);

  return mem;
}


int load(unsigned n, uint8_t* mem, int32_t clSz, uint8_t* fat, uint8_t* dir, uint8_t* data)
{
  int32_t   sz;
  uint32_t  cl = find(n, dir, &sz);
  if (cl == 0)  return -1;
  uint16_t* ft = (uint16_t*)fat;

  do
  {
    data += (cl - 2) * clSz;
    mem   = mcpy(mem, data, clSz);
    sz   -= clSz;
    cl    = swap16l(ft[cl]);
  }
  while ((sz > clSz) && (cl > 1) && (cl < 0x0000fff8u));

  if (sz > clSz)  return -2;

  data += (cl - 2) * clSz;
  mcpy(mem, data, sz);

  if (swap16l(ft[cl]) < 0x0000fff8u)  return -3;

  return 0;
}


extern "C" void rce_appmain(uintptr_t)
{
#if tgt_gen2
  fatDump();
#endif
}
