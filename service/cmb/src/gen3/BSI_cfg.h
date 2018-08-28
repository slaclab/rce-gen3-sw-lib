// -*-Mode: C;-*-
/*
@file BSI_cfg.h
@verbatim
                               Copyright 2011
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:


@par Abstract:
This file contains the masks, sizes and offsets for extractign fields
from words in the BSI (BootStrap Interface.)

@author
Gregg Thayer <jgt@slac.stanford.edu>

@par Date created:
2012/01/03

@par Credits:
SLAC
*/

#ifndef BSI_CFG_H
#define BSI_CFG_H

/* This is the size of the BSI block RAM in bytes*/
enum { BSI_CFG_SIZE = 512*4 };

/*
   This is the value that the CE writes to the above addresss
   when boot is successful
*/
enum { BSI_BOOT_RESPONSE_SUCCESS = 0 };

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- *//*

  The layout of the BSI configuration in memory is composed of fixed
  length components. There is a portion which is written by the
  Cluster Element and read by the IPMC, and there is a portion
  written by the IPMC and read by the Cluster Element. There is an
  additional portion that is written by the IPMC only to the
  Cluster Manager.

  This file will describe the layout of the BSI configuration data
  and provide enumerations and macros to assist in the extraction of
  these data.

  This is the portion of the BSI that is written by the Cluster Element.
  It is common to both Cluster Elements and the Cluster Manager.

  ----------------------------------------
  |  Cluster Element BSI Version Number  |
  |  Cluster Element Network PHY type    |
  |  Cluster Element MAC address         |
  |  Cluster Element Interconnect Def    |
  ----------------------------------------
  |                                      |
  =             Reserved                 =
  |                                      |
  ----------------------------------------

  Here are descriptions of the above:

  The Cluster Element BSI Version Number is a single 32-bit word which
  describes the layout BSI.

  The Cluster Element Network PHY type is a single 32-bit word which
  contains an enumeration of the network configuration needed by
  the switch. It is split into 4 1-byte fields, each of which describes
  the configuration of a single lane of communication with the switch.
  In cases of CMBs with multiple RCEs, one channel (of 4 lanes) is
  shared by Elements 0 and 1, and one lane is shared by Elements 2 and 3.
  Elements which share a channel must have lane definitions which do not
  conflict. This information is collated by the IPMC and given to the
  DTM as a list of configurations for the 24 ports. (See below.)

  The Cluster Element MAC address is a 2 word MAC address. The
  MAC address is laid out as a little endian byte array with the
  two MSB, MBZ.

  The Cluster Element Interconnect Definition is a single 32-bit word
  which describes the use of communication lines between the Cluster
  Element and the Cluster Manager. This information is collected from
  the BSI of the Cluster Elements in a way similar to the Ethernet PHY
  definitions. Precisely how the 32-bits is broken up and shared is
  currently unclear to me.

  There will be reserved space between the Cluster Element written
  portion of the BSI and the IPMC written portion.


  ----------------------------------------
  ----------------------------------------
  ----------------------------------------


  This is the portion of the BSI that is written by the IPMC and is
  common to both Cluster Elements and the Cluster Manager.

  ----------------------------------------
  |  CMB Serial Number                   |
  |  Cluster Address                     |
  |  Cluster Group Name  (8 words)       |
  |  External Interconnect Definition    |
  ----------------------------------------
  |                                      |
  =             Reserved                 =
  |                                      |
  ----------------------------------------

  The CMB Serial Number is two 32-bit words.

  The Cluster Address is a single 32-bit word and contains the fields:
  1) Version        - Version number covering the layout of the
                      cluster address word
  2) Cluster Number
  3) Bay Number
  4) Element Number

  The Cluster Group Name is a null terminated, little endian, character
  array. Meaning that the first character is the LSB of the first word.

  The External Interconnect Definition is a single 32-bit word which
  describes the portion of the RTM that is connected to the Cluster
  Element. In the case of DPMs this will be found in the EEPROM on the
  I2C corner of the RTM. In the case of the DTM, it will be found in
  the EEPROM on the RMB.

  There will be reserved space between the Cluster Element written
  portion of the BSI and the IPMC written portion.

  ----------------------------------------
  ----------------------------------------
  ----------------------------------------


  The BSI structure for the Cluster Manager additionally includes
  the following information.

  ----------------------------------------
  |                                      |
  |  Cluster Switch configuration        |
  |                                      |
  ----------------------------------------
  |                                      |
  |  Cluster Element                     |
  |  Interconnect Definitions            |
  =  (variable length?)                  =
  |                                      |
  ----------------------------------------

  The Cluster Switch configuration is currently a 24 word array
  which describe the type of connection which is made to the 24
  ports on the Cluster's switch. Each word is broken into two parts
  the lower part describes the configuration of the port (1G, XAUI, etc.)
  the upper part describes the group of ports each port belongs to. Each
  member of a port group is electrically connected to the same Ethernet
  destination.

  The Cluster Element Interconnect Definitions is an array of 32-bit
  the Cluster Element Interconnect Definitions which will be collected
  from each Cluster Element. (I don't yet know how they are organized...)
  I presume it will be one 32-bit word per DPM bay, but it may not be.

  ----------------------------------------
  ----------------------------------------

  ----------------------------------------
  |  RCE Status                          |
  ----------------------------------------

  At the end of the BSI, there will be a status which will be written
  by the RCE and polled by the IPMC. The precise meaning of this status
  value is currently undefined, and will be treated as a 32-bit number
  for now.
									  */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- */
/*
  Here are the offsets and the version number which describes the
  Cluster Element written portion of the BSI.
*/
enum {BSI_CLUSTER_CFG_VERSION = 1};

typedef enum _BSI_COMMON_CFG_OFFSETS
{
   BSI_RCE_DATA_OFFSET                      = 0x00,
   BSI_CLUSTER_CFG_VERSION_OFFSET           = 0x00,
   BSI_NETWORK_PHY_TYPE_OFFSET              = 0x01,
   BSI_MAC_ADDR_OFFSET                      = 0x02,
   BSI_INTERCONNECT_OFFSET                  = 0x04,
   BSI_RESERVED_OFFSET                      = 0x05,

   BSI_CLUSTER_DATA_OFFSET                  = 0x10,
   BSI_CMB_SERIAL_NUMBER_OFFSET             = 0x10,
   BSI_CLUSTER_ADDR_OFFSET                  = 0x12,
   BSI_CLUSTER_GROUP_NAME_OFFSET            = 0x13,
   BSI_EXTERNAL_INTERCONNECT_OFFSET         = 0x1B,
   BSI_RESERVED_OFFSET1                     = 0x1C,

   BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET  = 0x30,
   BSI_CLUSTER_ELEMENT_INTERCONNECT_OFFSET  = 0x48,
   
   BSI_UBOOT_VERSION_OFFSET                 = 0x58,
   BSI_DAT_VERSION_OFFSET                   = 0x5C,

   BSI_BOOT_RESPONSE_OFFSET                 = 0x1FF,
} BSI_COMMON_CFG_OFFSETS;

/* Sizes of the various arrays */
enum {BSI_GROUP_NAME_SIZE                   = BSI_EXTERNAL_INTERCONNECT_OFFSET        - BSI_CLUSTER_GROUP_NAME_OFFSET};
enum {BSI_DOMAIN_NAME_SIZE                  = 16}; /* Hardcode for now */
enum {BSI_CLUSTER_SWITCH_CONFIGURATION_SIZE = BSI_CLUSTER_ELEMENT_INTERCONNECT_OFFSET - BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET};
enum {BSI_CLUSTER_ELEMENT_INTERCONNECT_SIZE = 16}; /* Hardcode for now */
enum {BSI_UBOOT_VERSION_SIZE                = BSI_DAT_VERSION_OFFSET                  - BSI_UBOOT_VERSION_OFFSET};
enum {BSI_DAT_VERSION_SIZE                  = 4}; /* Hardcode for now */

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

#endif /* BSI_CFG_H */
