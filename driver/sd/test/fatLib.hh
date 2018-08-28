// -*-Mode: C++;-*-
/*!
*
* @brief   A module for dumping an SD-card FAT filesystem
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    July 1, 2012 -- Created
*
* $Revision: 2246 $
*
* @verbatim
*                               Copyright 2012
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef _FAT_LIB_HH_
#define _FAT_LIB_HH_
#include <inttypes.h>

#if 0 // From OS X
struct bsbpb {
  u_int16_t   bps;            /* [-S] bytes per sector */
  u_int8_t    spc;            /* [-c] sectors per cluster */
  u_int16_t   res;            /* [-r] reserved sectors */
  u_int8_t    nft;            /* [-n] number of FATs */
  u_int16_t   rde;            /* [-e] root directory entries */
  u_int16_t   sec;            /* [-s] total sectors */
  u_int8_t    mid;            /* [-m] media descriptor */
  u_int16_t   spf;            /* [-a] sectors per FAT */
  u_int16_t   spt;            /* [-u] sectors per track */
  u_int16_t   hds;            /* [-h] drive heads */
  u_int32_t   hid;            /* [-o] hidden sectors */
  u_int32_t   bsec;           /* [-s] big total sectors */
};
/* FAT32 extensions */
struct bsxbpb {
  u_int32_t   bspf;           /* [-a] big sectors per FAT */
  u_int16_t   xflg;           /* control flags */
  u_int16_t   vers;           /* file system version */
  u_int32_t   rdcl;           /* root directory start cluster */
  u_int16_t   infs;           /* [-i] file system info sector */
  u_int16_t   bkbs;           /* [-k] backup boot sector */
};
#endif

typedef struct
{
  unsigned jumpCode;
  char     oemName[8 + 1];              // + 1 for '\0'
  uint16_t secSz;
  uint8_t  secPerClus;
  uint16_t rsvdSecCnt;
  uint8_t  numFATs;
  uint16_t dirSz;
  uint16_t totSec;
  uint8_t  mediaDsc;
  uint16_t fatSec;
  uint16_t numSecPerTrk;
  uint16_t numHeads;
  uint32_t numHidden;
  uint32_t numLogSec;
  uint16_t signature;
} MBR;


typedef struct
{
  uint8_t  physDrNo;
  uint8_t  rsvd;
  uint8_t  extBootSig;
  uint32_t volId;
  char     partVolLabel[11 + 1];        // +1 for '\0'
  char     fileSysType[8 + 1];          // +1 for '\0'
} EBPB;

typedef struct
{
  uint32_t fatSec;
  uint16_t mirrorFlags;
  uint16_t fsVersion;
  uint32_t rootClus;
  uint16_t fsInfo;
  uint16_t backupSec;
  uint8_t  ldn;
  uint8_t  curHead;
  uint8_t  extSig;
  uint32_t partSerNo;
  char     volLabel[11 + 1];            // + 1 for '\0'
  char     fsType[8 + 1];               // + 1 for '\0'
} MBR32;


void fatMbr(uint8_t* disk, MBR* mbr);
void fatEbpb(uint8_t* disk, EBPB* ebpb);
void fat32Mbr(uint8_t* disk, MBR32* mbr);
void fatPart(uint8_t* partition, uint32_t* lbaBegin);
void fatFsInfo(uint8_t* block);
bool fatDir(uint8_t* dir, uint8_t* end, uint32_t* dirEntries);

uint32_t n2Fn(unsigned n);
uint32_t find(const char* f, uint8_t* dir, int32_t* sz);
uint32_t find(unsigned n, uint8_t* dir, int32_t* sz);

#endif
