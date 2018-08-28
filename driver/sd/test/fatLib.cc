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
#include <stdio.h>
#include <string.h>
#include <inttypes.h>


#include "fatLib.hh"
#include "conversion/Swap.hh"
using tool::conversion::swap16l;
using tool::conversion::swap32l;


void fatMbr(uint8_t* disk, MBR* mbr)
{
  printf("MRB:\n");
#if ENDIANNESS_IS_LITTLE
  mbr->jumpCode = (disk[0] << 16) | (disk[1] << 8) | disk[2];
#else
  mbr->jumpCode = (disk[2] << 16) | (disk[1] << 8) | disk[0];
#endif
  printf("Jump code                = 0x%06x\n", mbr->jumpCode);

  strncpy(mbr->oemName, (const char*)&disk[3], 8);
  mbr->oemName[8] = '\0';
  printf("OEM name                 = %s\n", mbr->oemName);

  mbr->secSz = swap16l(*(uint16_t*)&disk[0xb]);
  printf("Bytes per Sector         = %u\n", mbr->secSz);

  mbr->secPerClus = disk[0xd];
  printf("Sectors per Cluster      = %u\n", mbr->secPerClus);

  mbr->rsvdSecCnt = swap16l(*(uint16_t*)&disk[0xe]);
  printf("No. of Reserved Sectors  = %u\n", mbr->rsvdSecCnt);

  mbr->numFATs = disk[0x10];
  printf("No. of FATs              = %u\n", mbr->numFATs);

  mbr->dirSz = swap16l(*(uint16_t*)&disk[0x11]);
  printf("No. of root dir entries  = %u\n", mbr->dirSz);

  mbr->totSec = swap16l(*(uint16_t*)&disk[0x13]);
  printf("Total no. of sectors     = %u\n", mbr->totSec);

  mbr->mediaDsc = disk[0x15];
  printf("Media descriptor         = %02x\n", mbr->mediaDsc);

  mbr->fatSec = swap16l(*(uint16_t*)&disk[0x16]);
  printf("No. of sectors per FAT   = %u\n", mbr->fatSec);

  mbr->numSecPerTrk = swap16l(*(uint16_t*)&disk[0x18]);
  printf("No. of sectors per track = %u\n", mbr->numSecPerTrk);

  mbr->numHeads = swap16l(*(uint16_t*)&disk[0x1a]);
  printf("No. of Heads             = %u\n", mbr->numHeads);

  mbr->numHidden = swap32l(*(uint32_t*)&disk[0x1c]);
  printf("No. of hidden sectors    = 0x%08lx\n", mbr->numHidden);

  mbr->numLogSec = swap32l(*(uint32_t*)&disk[0x20]);
  printf("Total logical sectors    = 0x%08lx\n", mbr->numLogSec);

#if ENDIANNESS_IS_LITTLE
  mbr->signature = (disk[0x1fe] << 8) | disk[0x1ff];
#else
  mbr->signature = (disk[0x1ff] << 8) | disk[0x1fe];
#endif
  printf("Signature                = 0x%04x\n", mbr->signature);
}


void fatEbpb(uint8_t* disk, EBPB* ebpb)
{
  printf("\nEBPB:\n");

  ebpb->physDrNo = disk[0x24];
  printf("Physical drive number    = 0x%02x\n", ebpb->physDrNo);

  ebpb->rsvd = disk[0x25];
  printf("Reserved                 = 0x%02x\n", ebpb->rsvd);

  ebpb->extBootSig = disk[0x26];
  printf("Extended Boot signature  = 0x%02x\n", ebpb->extBootSig);

  ebpb->volId = swap32l(*(uint32_t*)&disk[0x27]);
  printf("Volume ID                = 0x%08lx\n", ebpb->volId);

  strncpy(ebpb->partVolLabel, (const char*)&disk[0x2b], 11);
  ebpb->partVolLabel[11] = '\0';
  printf("Partition Volume Label   = %s\n", ebpb->partVolLabel);

  strncpy(ebpb->fileSysType, (const char*)&disk[0x36], 8);
  ebpb->fileSysType[8] = '\0';
  printf("File system type         = %s\n", ebpb->fileSysType);
}


// Below is specialized for FAT32
void fat32Mbr(uint8_t* disk, MBR32* mbr)
{
  printf("\nFAT 32:\n");

  mbr->fatSec = swap32l(*(uint32_t*)&disk[0x24]);
  printf("No. of sectors per FAT   = %lu\n", mbr->fatSec);

  mbr->mirrorFlags = swap16l(*(uint16_t*)&disk[0x28]);
  printf("Mirror flags             = %04x\n", mbr->mirrorFlags);

  mbr->fsVersion = swap16l(*(uint16_t*)&disk[0x2a]);
  printf("FS version               = %u\n", mbr->fsVersion);

  mbr->rootClus = swap32l(*(uint32_t*)&disk[0x2c]);
  printf("Root dir 1st cluster     = %lu\n", mbr->rootClus);

  mbr->fsInfo = swap16l(*(uint16_t*)&disk[0x30]);
  printf("FS info sec no.          = %u\n", mbr->fsInfo);

  mbr->backupSec = swap16l(*(uint16_t*)&disk[0x32]);
  printf("Backup boot sec no.      = %u\n", mbr->backupSec);

  mbr->ldn = disk[0x40];
  printf("Logical drive no.        = %u\n", mbr->ldn);

  mbr->curHead = disk[0x41];
  printf("Current head             = %u\n", mbr->curHead);

  mbr->extSig = disk[0x42];
  printf("Extended signature       = 0x%02x\n", mbr->extSig);

  mbr->partSerNo = swap32l(*(uint16_t*)&disk[0x43]);
  printf("Volume ID                = 0x%08lx = %lu\n", mbr->partSerNo, mbr->partSerNo);

  strncpy(mbr->volLabel, (const char*)&disk[0x47], 11);
  mbr->volLabel[11] = '\0';
  printf("Volume Label             = %s\n", mbr->volLabel);

  strncpy(mbr->fsType, (const char*)&disk[0x52], 8);
  mbr->fsType[8] = '\0';
  printf("File system type         = %s\n", mbr->fsType);
}


void fatPart(uint8_t* partition, uint32_t* lbaBegin)
{
  for (int i = 0; i < 4; ++i)
  {
    printf("Partion %d:\n", i);
    printf("  Boot Flag   = %u\n", partition[0]);

    uint32_t chsBegin = swap32l((*(uint32_t*)&partition[0])) >> 8;
    printf("  CHS Begin   = %06"PRIx32":",  chsBegin);
    printf("    head = %02x, sector = %02x, cylinder = %03x\n",
#if ENDIANNESS_IS_LITTLE
           partition[1], partition[2] & 0x3f,
           ((partition[2] & 0xc0) << 2) | partition[3]
#else
           partition[3], partition[2] & 0x3f,
           ((partition[2] & 0xc0) << 2) | partition[1]
#endif
           );

    printf("  Type Code   = 0x%02x\n", partition[4]);

    uint32_t chsEnd = swap32l((*(uint32_t*)&partition[4])) >> 8;
    printf("  CHS End     = %06"PRIx32":",  chsEnd);
    printf("    head = %02x, sector = %02x, cylinder = %03x\n",
#if ENDIANNESS_IS_LITTLE
           partition[5], partition[6] & 0x3f,
           ((partition[6] & 0xc0) << 2) | partition[7]
#else
           partition[3], partition[2] & 0x3f,
           ((partition[2] & 0xc0) << 2) | partition[1]
#endif
           );

    lbaBegin[i] = swap32l(*(uint32_t*)&partition[8]);
    uint32_t numSects = swap32l(*(uint32_t*)&partition[12]);
    printf("  LBA Begin   = %08"PRIx32" = %"PRIu32"\n", lbaBegin[i], lbaBegin[i]);
    printf("  Num Sectors = %08"PRIx32" = %"PRIu32"\n", numSects, numSects);
    printf("  LBA End     = %08"PRIx32" = %"PRIu32"\n", lbaBegin[i] + numSects, lbaBegin[i] + numSects);

    partition += 16;
  }
}


void fatFsInfo(uint8_t* block)
{
  printf("Signature 1              = %c%c%c%c\n",
#if ENDIANNESS_IS_LITTLE
         block[0], block[1], block[2], block[3]
#else
         block[3], block[2], block[1], block[0]
#endif
         );

  printf("Signature 2              = %c%c%c%c\n",
#if ENDIANNESS_IS_LITTLE
         block[0x1e4], block[0x1e5], block[0x1e6], block[0x1e7]
#else
         block[0x1e7], block[0x1e6], block[0x1e5], block[0x1e4]
#endif
         );

  printf("Last free      cluster   = %08lx\n", swap32l(*(uint32_t*)&block[0x1e8]));
  printf("Last allocated cluster   = %08lx\n", swap32l(*(uint32_t*)&block[0x1ec]));

  uint32_t signature;
#if ENDIANNESS_IS_LITTLE
  signature = (block[0x1fc] << 24) | (block[0x1fd] << 16) | (block[0x1fe] << 8) | block[0x1ff];
#else
  signature = (block[0x1ff] << 24) | (block[0x1fe] << 16) | (block[0x1fd] << 8) | block[0x1fc];
#endif
  printf("Signature 3              = %08lx\n", signature);
}


bool fatDir(uint8_t* dir, uint8_t* end, uint32_t* cnt)
{
  uint8_t* dp   = dir;
  while (*dp)
  {
    if (*dp != 0xe5)                    // Don't dump deleted files
    {
      char filename[12];
      strncpy(filename, (const char*)&dp[0], 8+3);
      filename[11] = '\0';
      uint32_t* fn = (uint32_t*)filename;
      printf("  Filename           = '%-11s' [%08lx %08lx %08lx]\n",
             filename, fn[0], fn[1], fn[2]);

      uint8_t attrib = dp[0xb];
      printf("  Attrib             = 0x%02x\n", attrib);

      uint16_t time = swap16l(*(uint16_t*)&dp[0x16]);
      printf("  Time               = 0x%04x\n", time);

      uint16_t date = swap16l(*(uint16_t*)&dp[0x18]);
      printf("  Date               = 0x%04x\n", date);

      uint16_t fstClusHi = swap16l(*(uint16_t*)&dp[0x14]);
      printf("  First Cluster high = 0x%04x\n", fstClusHi);

      uint16_t fstClusLo = swap16l(*(uint16_t*)&dp[0x1a]);
      printf("  First Cluster low  = 0x%04x\n", fstClusLo);

      uint32_t fileSize  = swap32l(*(uint32_t*)&dp[0x1c]);
      printf("  File size          = 0x%08"PRIx32"\n", fileSize);

      printf("\n");
    }
    dp += 32;
    ++(*cnt);
    if (dp >= end)  return false;
  }
  return true;
}


uint32_t n2Fn(unsigned n)
{
  char* hx = (char*)"0123456789ABCDEF";
  union
  {
    char     fn[4];
    uint32_t fn4;
  } name = { {hx[((n & 0xf00) >> 8)],
              hx[((n & 0x0f0) >> 4)],
              hx[((n & 0x00f)     )],
              ' '} };
  if (n < 0x100)
  {
    name.fn4 = (name.fn4 << 8) | ' ';
    if (n < 0x10)
      name.fn4 = (name.fn4 << 8) | ' ';
  }
  return name.fn4;
}


uint32_t find(const char* name, uint8_t* dir, int32_t* sz)
{
  size_t len = strlen(name);
  char*  dot = strchr(name, '.');
  if (dot == name + len)  return 0;

  char fn[8+3+1];
  strncpy(fn, name, dot - name);
  fn[dot - name] = '\0';
  sprintf(fn, "%-8s%3s", fn, dot + 1);

  uint8_t* dp   = dir;
  while (*dp)
  {
    if (*dp != 0xe5)                    // Don't dump deleted files
    {
      if (strncmp(fn, (const char*)&dp[0], 8+3) == 0)
      {
        *sz = swap32l(*(uint32_t*)&dp[0x1c]);
        return ((swap16l(*(uint16_t*)&dp[0x14]) << 16) |
                 swap16l(*(uint16_t*)&dp[0x1a])      );
      }
    }
    dp += 32;
  }
  return 0;
}


uint32_t find(unsigned n, uint8_t* dir, int32_t* sz)
{
  char fn[8+3 + 1];

  sprintf(fn,"%X.SYS", n);

  return find(fn, dir, sz);
}
