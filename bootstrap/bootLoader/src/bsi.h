/*!@file     bsi.h
*
* @brief     BootStrap Interface structure definition
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)

* @date      April 30, 2013 -- Created
*
* $Revision: 2123 $
*
* @verbatim                    Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef BOOTSTRAP_FSBL_BSI_H
#define BOOTSTRAP_FSBL_BSI_H

#define BSI_BRAM     0x84000000       /* Location of BSI BRAM from firmware  */

struct MAC {
  unsigned char u8[8];
};

struct BSI {
  unsigned      verSize;                   /* 0x00 */
  unsigned      netPhy;                    /* 0x01 */
  struct MAC    mac;                       /* 0x02 */
  unsigned      interconnect;              /* 0x04 */
  unsigned      rsvd_05[0x10-0x05];        /* 0x05 */
  unsigned      serNo[2];                  /* 0x10 */
  unsigned      clAddr;                    /* 0x12 */
  unsigned char group[8*sizeof(unsigned)]; /* 0x13 */
  unsigned      extInterconnect;           /* 0x1b */
  unsigned      rsvd_1d[0x30-0x1c];        /* 0x1c */
  unsigned      switchCfg[24];             /* 0x30 */
  unsigned      ceInterconnect;            /* 0x48 */
};

#endif
