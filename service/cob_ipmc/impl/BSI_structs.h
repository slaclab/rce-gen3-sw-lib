// -*-Mode: C;-*-
/* 
@file BSI_structs.h
@verbatim
                               Copyright 2011
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:


@par Abstract:

@author
Gregg Thayer <jgt@slac.stanford.edu>

@par Date created:
2012/01/03

@par Credits:
SLAC
*/
#ifndef BSI_STRUCTS_H
#define BSI_STRUCTS_H

#include "cmbcfg/CMB_switch_cfg.h"
#include "cob_ipmc/impl/COB_shelf_info.h"

/*
  This is the MAC configuration for the CMB.
  Note that the byte[7] and byte[6] are MBZ.
*/
typedef uint8_t CMB_mac_addr[8];

/*
  The CMB id is the unique 64 bit number stored on CMB's IDPROM
*/
typedef uint8_t CMB_id[8];

/* 
   The Xilinx Device DNA is a 57 bit number that 
   we store as a 64 bit number (8 bytes)
*/
typedef uint8_t RCE_device_dna[8];

/* 
   The Xilinx eFUSE user register is a 32-bit number, 
   that will be extended to 64-bits in the RCE and put 
   in the BSI.
*/
typedef uint8_t RCE_efuse_user[8];

#define GROUP_NAME_SIZE 32
#define UBOOT_VERSION_NAME_SIZE 32
#define DAT_VERSION_NAME_SIZE 32

typedef uint32_t BSI_cluster_addr;
typedef uint32_t BSI_ext_interconnect;

typedef struct _BSI_rce_data_v0
{
   uint32_t bsi_version;
} PACKED BSI_rce_data_v0;

typedef struct _BSI_rce_data_v1
{
   uint32_t         bsi_version;
   RCE_net_phy_type phy_type;
   CMB_mac_addr     rce_mac;
   uint32_t         int_interconnect;
} PACKED BSI_rce_data_v1;

typedef struct _BSI_rce_data_v2
{
   uint32_t         bsi_version;
   RCE_net_phy_type phy_type;
   CMB_mac_addr     rce_mac;
   uint32_t         int_interconnect;
   char             uboot_version[UBOOT_VERSION_NAME_SIZE];
   char             dat_version[DAT_VERSION_NAME_SIZE];
   RCE_device_dna   device_dna;
   RCE_efuse_user   efuse_user;
} PACKED BSI_rce_data_v2;

typedef union _BSI_rce_data
{
   BSI_rce_data_v0 v0;
   BSI_rce_data_v1 v1;
   BSI_rce_data_v2 v2;
} BSI_rce_data;

typedef struct _BSI_cluster_data
{
   CMB_id               id;
   BSI_cluster_addr     addr;
   char                 group_name[GROUP_NAME_SIZE];
   BSI_ext_interconnect ext_interconnect;
} PACKED BSI_cluster_data;

typedef uint32_t BSI_cluster_switch_cfg;

enum {BSI_OFFSET_NULL = -1};

typedef struct _BSI_version_info
{
   int      version;
   uint16_t rce_data_offset;
   uint16_t rce_data_size;
   uint16_t cluster_data_offset;
   uint16_t cluster_data_size;
   uint16_t cluster_switch_data_offset;
   uint16_t cluster_switch_data_size;
   uint16_t shelf_ip_info_offset;
   uint16_t shelf_ip_info_size;
   uint16_t fabric_map_offset;
   uint16_t fabric_map_size;
   uint16_t boot_response_offset;
   uint16_t boot_response_size;
} BSI_version_info;

static const BSI_version_info bsi_version_info[] = {
   {1,
    0x000*4, sizeof(BSI_rce_data_v1), 
    0x010*4, sizeof(BSI_cluster_data),
    0x030*4, sizeof(BSI_cluster_switch_cfg)*BSI_N_CLUSTER_PORTS,
    BSI_OFFSET_NULL, 0,
    0x080*4, sizeof(COB_fabric_map),
    0x1FF*4, sizeof(uint32_t)}, 
   {2, 
    0x000*4, sizeof(BSI_rce_data_v2), 
    0x050*4, sizeof(BSI_cluster_data),
    0x090*4, sizeof(BSI_cluster_switch_cfg)*BSI_N_CLUSTER_PORTS,
    0x0c0*4, sizeof(COB_shelf_ip_info),
    0x100*4, sizeof(COB_fabric_map),
    0x1FF*4, sizeof(uint32_t)}
};


/* This is the size of the BSI block RAM in bytes*/
enum { BSI_CFG_SIZE = 512*4 };

/*
   This is the value that the CE writes to the above addresss
   when boot is successful
*/
enum { BSI_BOOT_RESPONSE_SUCCESS = 0 };

/* ---------------------------------------------------------------------- */
/*
  These are the utility values for extracting fields from the BSI
*/
/* ---------------------------------------------------------------------- */
#define BSI_CLUSTER_INFO_VERSION 1
/* The size of the fields in the cluster address word */
typedef enum _BSI_CLUSTER_ADDR_S
{
   BSI_CLUSTER_ADDR_S_VERSION = 8,
   BSI_CLUSTER_ADDR_S_CLUSTER = 8,
   BSI_CLUSTER_ADDR_S_BAY     = 8,
   BSI_CLUSTER_ADDR_S_ELEMENT = 8,
}  BSI_CLUSTER_ADDR_S;

/* Right justified bit offset of each of the cluster addr word fields */
typedef enum _BSI_CLUSTER_ADDR_V
{
   BSI_CLUSTER_ADDR_V_VERSION = 24,
   BSI_CLUSTER_ADDR_V_CLUSTER = 16,
   BSI_CLUSTER_ADDR_V_BAY     =  8,
   BSI_CLUSTER_ADDR_V_ELEMENT =  0,
}  BSI_CLUSTER_ADDR_V;

/* Masks for the cluster address word */
typedef enum _BSI_CLUSTER_ADDR_M
{
   BSI_CLUSTER_ADDR_M_VERSION = (0xff << BSI_CLUSTER_ADDR_V_VERSION),
   BSI_CLUSTER_ADDR_M_CLUSTER = (0xff << BSI_CLUSTER_ADDR_V_CLUSTER),
   BSI_CLUSTER_ADDR_M_BAY     = (0xff << BSI_CLUSTER_ADDR_V_BAY),
   BSI_CLUSTER_ADDR_M_ELEMENT = (0xff << BSI_CLUSTER_ADDR_V_ELEMENT),
}  BSI_CLUSTER_ADDR_M;

#define BSI_VERSION_FROM_CLUSTER_ADDR(cl_addr) \
   ((cl_addr & BSI_CLUSTER_ADDR_M_VERSION) >> BSI_CLUSTER_ADDR_V_VERSION)

#define BSI_ELEMENT_FROM_CLUSTER_ADDR(cl_addr) \
   ((cl_addr & BSI_CLUSTER_ADDR_M_ELEMENT) >> BSI_CLUSTER_ADDR_V_ELEMENT)

#define BSI_BAY_FROM_CLUSTER_ADDR(cl_addr) \
   ((cl_addr & BSI_CLUSTER_ADDR_M_BAY) >> BSI_CLUSTER_ADDR_V_BAY)

#define BSI_CLUSTER_FROM_CLUSTER_ADDR(cl_addr) \
   ((cl_addr & BSI_CLUSTER_ADDR_M_CLUSTER) >> BSI_CLUSTER_ADDR_V_CLUSTER)

typedef enum _BSI_CLUSTER_SWITCH_PORT_S
{
   BSI_CLUSTER_SWITCH_PORT_S_GROUP = 16,
   BSI_CLUSTER_SWITCH_PORT_S_TYPE  = 16,
}  BSI_CLUSTER_SWITCH_PORT_S;

/* Right justified bit offset of each of the cluster addr word fields */
typedef enum _BSI_CLUSTER_SWITCH_PORT_V
{
   BSI_CLUSTER_SWITCH_PORT_V_GROUP = 16,
   BSI_CLUSTER_SWITCH_PORT_V_TYPE  =  0,
}  BSI_CLUSTER_SWITCH_PORT_V;

/* Masks for the cluster address word */
typedef enum _BSI_CLUSTER_SWITCH_PORT_M
{
   BSI_CLUSTER_SWITCH_PORT_M_GROUP = (0xffff << BSI_CLUSTER_SWITCH_PORT_V_GROUP),
   BSI_CLUSTER_SWITCH_PORT_M_TYPE  = (0xffff << BSI_CLUSTER_SWITCH_PORT_V_TYPE),
}  BSI_CLUSTER_SWITCH_PORT_M;

#define GROUP_FROM_CLUSTER_SWITCH_PORT(port_def) \
   ((port_def & BSI_CLUSTER_SWITCH_PORT_M_GROUP) >> BSI_CLUSTER_SWITCH_PORT_V_GROUP)

#define TYPE_FROM_CLUSTER_SWITCH_PORT(port_def) \
   ((port_def & BSI_CLUSTER_SWITCH_PORT_M_TYPE) >> BSI_CLUSTER_SWITCH_PORT_V_TYPE)

#endif /* BSI_STRUCTS_H */
