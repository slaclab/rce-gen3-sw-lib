#ifndef COB_UTIL_H
#define COB_UTIL_H

#define PACKED __attribute__((packed))

#include "cob_ipmc/impl/COB_cmds.h"

enum
{
   ATCA_MIN_SLOT = 1,
   ATCA_MAX_SLOT = 16,
};

enum 
{ 
   COB_MAX_BAYS          = 7,
   COB_ALL_BAYS_MASK     = (1<<COB_MAX_BAYS)-1,
   COB_BAY_MAX_RCES      = 4,
   COB_BAY_ALL_RCES_MASK = (1<<COB_BAY_MAX_RCES)-1,
   COB_MAX_SFPS          = 8,
   COB_ALL_SFPS_MASK     = (1<<COB_MAX_SFPS)-1,
   COB_MAX_FRUS          = 2,
};

enum 
{
   DPM0 = 0,
   DPM1 = 1,
   DPM2 = 2,
   DPM3 = 3,
   DTM  = 4,
   RTM  = 5,
   CEN  = 6,
};

enum
{
   COB_INVALID_TEMP = 0x80,
};

enum
{
   COB_VOK_MASK = 0xff,
};

enum
{
   RCE_BSI_SIZE = 0x800,
   RCE_BSI_STATUS_ADDR = 0x07fc,
};

enum
{
   COB_FRU_ID = 0,
   RTM_FRU_ID = 1,
};

enum
{
   RMB_MAX_WATTS = 51,
};


#define RTM_INFO_SIZE 5
typedef struct _RTM_info
{
   uint8_t  power; /* Power allocation in fifth Watts */
   uint32_t type;
} PACKED RTM_info;

const char* COB_util_get_bay_name(int bay);

int COB_invalid_rmb_type(unsigned int type);

RTM_info* COB_get_rtm_info(const COB_ctx* ctx, 
			             int  ipmb_addr, 
			             int  rmb, 
			            void* buf);

void COB_util_get_masks(char* input, 
			char* sm_name, 
			 int  sm_name_len,
		    uint16_t* slot_mask, 
		     uint8_t* bay_mask, 
		     uint8_t* rce_mask,
			 int  verbose);

void COB_util_mask_usage();

#endif
