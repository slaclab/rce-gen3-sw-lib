// -*-Mode: C;-*-
/*
@file CMB_common.h
@verbatim
                               Copyright 2011
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:


@par Abstract:
This defines structures that are used on both the Cluster Element
and the IPM Controller.

@author
Gregg Thayer <jgt@slac.stanford.edu>

@par Date created:
2012/01/03

@par Credits:
SLAC
*/

#ifndef CMB_COMMON_H
#define CMB_COMMON_H

#include <stdint.h>

/*
  This is the MAC configuration for the CMB.
  Note that the byte[7] and byte[6] are unused.
*/
typedef struct _CMB_mac_addr_bf {
   uint8_t bytes[6];
   uint8_t mbz[2];
} CMB_mac_addr_bf;

typedef union _CMB_mac_addr {
  uint32_t u32[2];
  CMB_mac_addr_bf bf;
} CMB_mac_addr;

typedef struct _CMB_mac_cfg {
  CMB_mac_addr mac0;
  CMB_mac_addr mac1;
} CMB_mac_cfg;

/* This is the structure for the CMB ID Prom */
typedef struct _CMB_id_bf {
   uint8_t bytes[6];
   uint8_t mbz[2];
} CMB_id_bf;

typedef union _CMB_id {
   uint32_t u32[2];
   CMB_id_bf bf;
} CMB_id;

/*************************************************************************
    These structures define the postamble which is the same for all boards
    and is a variable length list of endpoint descriptions
*************************************************************************/
#define CMB_EXT_LUT 26 /* Number of external connections on a CMB */
#define CMB_INT_LUT 12 /* Number of internal connections on a CMB */

typedef struct _CMB_endpt_attr_bf {
  uint8_t reserved;
} CMB_endpt_attr_bf;

typedef union _CMB_endpt_attr
{
  uint8_t u8;
  CMB_endpt_attr_bf bf;
} CMB_endpt_attr;

typedef uint8_t CMB_endpt_type;

typedef struct _CMB_endpt
{
  CMB_endpt_type type;
  CMB_endpt_attr attr;
} CMB_endpt;

#endif // CMB_BOOT_H
