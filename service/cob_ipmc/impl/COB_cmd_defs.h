// -*-Mode: C;-*-
/* 
@file COB_cmd_defs.c
@verbatim
                               Copyright 2011
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:


@par Abstract:
This file contains enumerations of COB specific IPMB commands that 
are intercepted by the COB code. 

@author
Gregg Thayer <jgt@slac.stanford.edu>

@par Date created:
2012/01/03

@par Credits:
SLAC
*/

#ifndef COB_CMD_DEFS_H
#define COB_CMD_DEFS_H

#define SLAC_OEM_COB_NETFN 0x34

typedef enum _COB_CMDS 
{
   COB_CMD_SET_BOOTSTRAP       = 0x01, /* Unsupported Starting with COB5 */
   COB_CMD_GET_BOOTSTRAP       = 0x02, /* Unsupported Starting with COB5 */
   COB_CMD_WR_ELEM_BSI         = 0x03,
   COB_CMD_RD_ELEM_BSI         = 0x04,
   COB_CMD_READ_CDB_ID         = 0x05,
   COB_CMD_BOOT_IMMEDIATE      = 0x06, /* Unsupported Starting with COB5 */
   COB_CMD_SET_MONITOR_PER     = 0x07,
   COB_CMD_GET_MONITOR_PER     = 0x08,
   COB_CMD_DUMP_MONITOR        = 0x09,
   COB_CMD_RESET_RCE           = 0x0a,
   COB_CMD_RTM_EEPROM_READ     = 0x0b,
   COB_CMD_RTM_EEPROM_WRITE    = 0x0c,
   COB_CMD_REFRESH_SHELF_INFO  = 0x0d,
   COB_CMD_GET_BAY_POWER       = 0x0e,
   COB_CMD_GET_BAY_TEMPS       = 0x0f,
   COB_CMD_FP_SFP_GROUP        = 0x10, /* Unsupported */
   COB_CMD_SET_STANDALONE_SLOT = 0x11,
   COB_CMD_GET_BAY_GPIO        = 0x12,
   COB_CMD_GET_BAY_RAW_GPIO    = 0x13,
   COB_CMD_FIXUP_FRU_INFO_ID   = 0x14,
   COB_CMD_SET_ASSET_TAG       = 0x15,

   COB_CMD_I2C_TEST         = 0xf6,
   COB_CMD_I2C_READ         = 0xf7,
   COB_CMD_I2C_WRITE        = 0xf8,
   COB_CMD_RESET_COLD_DATA  = 0xf9,
   COB_CMD_SET_ZOMBIE_RCE   = 0xfa,
   COB_CMD_SET_BAY_POWER    = 0xfb,
   COB_CMD_READ_CDB_EEPROM  = 0xfc, /* Unsupported Starting with COB5 */
   COB_CMD_WRITE_CDB_EEPROM = 0xfd, /* Unsupported Starting with COB5 */
} COB_CMDS;


#endif /* COB_CMD_DEFS_H */
