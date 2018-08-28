// -*-Mode: C;-*-
/* 
@file COB_shelf_info.h
@verbatim
                               Copyright 2011
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:


@par Abstract:
This file deals with the information that the IPMC retreived from the 
Shelf FRU information, stored in the Shelf EEPROM and delivered by
the ShelfManager.

@author
Gregg Thayer <jgt@slac.stanford.edu>

@par Date created:
2012/01/03

@par Credits:
SLAC
*/

#ifndef COB_SHELF_INFO_H
#define COB_SHELF_INFO_H

//#include <hal/cob/cob_defs.h>

#include "cob_ipmc/impl/COB_util.h"

typedef int COB_data_status;

enum {SHELF_NAME_LEN         = 32,
      COB_NUM_ZONE2_CHANNELS = 13};

/* 
   Public data structure that is used to store the mapping of the
   fabric point-to-point connections.
*/
typedef struct _COB_fabric_dest_bf
{
   uint16_t slot:4;
   uint16_t chan:4;
   uint16_t port:5;
   uint16_t MBZ:1;
   uint16_t error:1;
   uint16_t done:1;
} PACKED COB_fabric_dest_bf;

typedef union _COB_fabric_dest
{
   COB_fabric_dest_bf bf;
   uint16_t           ui;
} COB_fabric_dest;

typedef struct _COB_fabric_slot
{
   COB_fabric_dest chan[16];
} PACKED COB_fabric_slot;

typedef struct _COB_logical_slot_bf
{
   uint32_t slot:8;
   uint32_t MBZ:22;
   uint32_t hub:1;
   uint32_t done:1;
} PACKED COB_logical_slot_bf;

typedef union _COB_logical_slot
{
   uint32_t ui;
   COB_logical_slot_bf bf;
} COB_logical_slot;

typedef struct _COB_fabric_map
{
   COB_fabric_slot  slot[16];
   COB_logical_slot logical_slot;
} PACKED COB_fabric_map;

typedef struct _COB_shelf_ip_status_bf
{
   uint32_t MBZ:31;
   uint32_t done:1;
} PACKED COB_shelf_ip_status_bf;

typedef union _COB_shelf_ip_status
{
   uint32_t               ui;
   COB_shelf_ip_status_bf bf;
} COB_shelf_ip_status;

typedef struct _COB_shelf_ip_info
{
   uint32_t port_membership;
   uint32_t port_ingress_policy;
   uint32_t group_base;
   uint32_t group_end;
   uint32_t subnet_mask;
   uint32_t gateway;
   COB_shelf_ip_status status;
} PACKED COB_shelf_ip_info;

/*
  Public data structure that is used to store information 
  collected from the Shelf Manager (from Shelf FRU Information) 
*/

typedef struct _COB_shelf_info {
   COB_data_status   timestamp_status;
   uint32_t          timestamp;
   COB_data_status   slot_status;
   uint8_t           slot;
   COB_data_status   shelf_name_status;
   char              shelf_name[SHELF_NAME_LEN];
   COB_data_status   fabric_map_status;
   COB_fabric_map    fabric_map;
   COB_data_status   shelf_ip_info_status;
   COB_shelf_ip_info shelf_ip_info;
} COB_shelf_info;

#endif /* COB_INFO_H */
