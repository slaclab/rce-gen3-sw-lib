#ifndef COB_CMDS_H
#define COB_CMDS_H

#include <stdint.h>
/*
===============================================================================
The IPMI command API
===============================================================================

The API exposes IPMI commands (some of which are specific to the COB) as 
a C interface. These commands are bridged by the Shelf Manager to the 
IPMC on the COB.

The method for sending one or more commands to the COB IPMC is to

1) Open a session with COB_open(<shelf_manager_ip>). If successful, this 
   will return a pointer to a control structure which is passed to all 
   subsequent commands in this session.

2) Send one or more commands. In addition to command specific arguments, these
   will all require 
   a) A pointer to the session context
   b) The IPMB Address of the COB (there is a utility function for retrieving
      the IPMB address from physical slot number (1-16))
   c) A pointer to a buffer into which to place the response

3) Close the session with COB_close.

For each command, the size of the required response buffer is defined as
is a struct to aid in the interpretation of the response. The minimal 
response contains the IPMI command value and the completion code.

An example of how one might use the API 

void rce_reset(const char* shelf_manager, int slot, int bay_mask, int rce_mask)
{

   COB_ctx* ctx;

   uint8_t rs_buf[COB_RCE_RESET_RS_SIZE];
   COB_rce_reset_rs* rs;

   uint8_t ipmb_addr;

   int slot, s;

   ctx = COB_open(shelf_manager);
   if(NULL == ctx) 
   {
      printf("ERROR - Couldn't connect to shelf manager %s\n", shelf_manager);
      return;
   }

   ipmb_addr = COB_ipmb_addr(ctx, slot);   

   if(0xff == ipmb_addr) return;

   rs = COB_rce_reset(ctx, ipmb_addr, bay_mask, rce_mask, rs_buf);

   if(NULL == rs)
   {
       printf("ERROR - %s/%i: Couldn't connect to IPMC\n", shelf_manager, slot);
   }
   else if(rs->ccode)
   { 
       printf("ERROR - %s/%i: "
              "IPMI command returned completion code %02x\n",
	      shelf_manager, slot, rs->ccode);
   }
   else if(rs->err_mask)
   {
       printf("ERROR - %s/%i: "
       	      "COB IPMC failed to execute reset commands in "
	      "bays in mask %02x\n",
	      shelf_manager, slot, rs->err_mask);
   }
   
   COB_close(ctx);

   return;
}
*/

typedef struct _COB_ctx_ COB_ctx;

COB_ctx* COB_open(const char* hostname);

void COB_close(const COB_ctx* ctx);

uint8_t COB_ipmb_addr(const COB_ctx* ctx, unsigned int slot);

int COB_raw_cmd(const COB_ctx* ctx,
		uint8_t ipmb_addr,
		uint8_t netfn,
		uint8_t cmd,
		uint8_t* data, unsigned int data_len,
		uint8_t* rs_buf, unsigned int rs_len);

int COB_slot_present(const COB_ctx* ctx, int ipmb_addr);
int COB_bay_present (const COB_ctx* ctx, int ipmb_addr, int bay);
int COB_bay_enabled (const COB_ctx* ctx, int ipmb_addr, int bay);

/*
  These are the function declarations for the IPMC commands
  that will be exposed to the public. They are largely a direct
  mappting of the raw IPMI commands supported by the IPMC with 
  little, if any, added value. (Except that you can send the
  commands at all, which is actually pretty valuable...)
*/

enum { COB_CCODE_RS_SIZE = 2 };
typedef struct
{
   uint8_t cmd;
   uint8_t ccode;
} COB_ccode_rs;

enum { IPMI_GET_DEVICE_ID_RS_SIZE = 17 };
typedef struct
{
  uint8_t cmd;
  uint8_t ccode;
  uint8_t dev_id;
  uint8_t dev_rev;
  uint8_t firmware_rev1;
  uint8_t firmware_rev2;
  uint8_t ipmi_ver;
  uint8_t dev_support;
  uint8_t manufacturer_id[3];
  uint8_t product_id[2];
  uint8_t aux_firmware_rev[4];
} IPMI_get_device_id_rs;

IPMI_get_device_id_rs* IPMI_get_device_id(const COB_ctx* ctx,
					        uint8_t  ipmb_addr,
					           void* rs_buf);

enum { COB_BSI_WRITE_RS_SIZE = COB_CCODE_RS_SIZE };
typedef COB_ccode_rs COB_bsi_write_rs;

COB_bsi_write_rs* COB_bsi_write(const COB_ctx* ctx, 
				      uint8_t  ipmb_addr,
				      uint8_t  bay_mask, 
				      uint8_t  rce_mask,
				     uint16_t  address,
				     uint16_t  length,
				      uint8_t* data,
				         void* rs_buf);

enum { COB_BSI_READ_RS_SIZE = 19 };
typedef struct
{
   uint8_t cmd;
   uint8_t ccode;
   uint8_t len;
   uint8_t data[16];
} COB_bsi_read_rs;

COB_bsi_read_rs* COB_bsi_read(const COB_ctx* ctx,
			            uint8_t  ipmb_addr,
			            uint8_t  bay,
			            uint8_t  rce,
			           uint16_t  address,
			           uint16_t  length,
			               void* rs_buf);

enum { COB_ID_PROM_READ_RS_SIZE = 10 };
typedef struct
{
   uint8_t cmd;
   uint8_t ccode;
   uint8_t id[8];
} COB_id_prom_read_rs;

COB_id_prom_read_rs* COB_id_prom_read(const COB_ctx* ctx,
				            uint8_t  ipmb_addr,
				            uint8_t  bay,
				               void* rs_buf);

enum { COB_RCE_RESET_RS_SIZE = 3 };
typedef struct
{
   uint8_t cmd;
   uint8_t ccode;
   uint8_t err_mask;
} COB_rce_reset_rs;

COB_rce_reset_rs* COB_rce_reset(const COB_ctx* ctx,
				      uint8_t  ipmb_addr,
				      uint8_t  bay_mask,
				      uint8_t  rce_mask,
				         void* rs_buf);

enum { COB_RTM_EEPROM_READ_RS_SIZE = 19 };
typedef struct
{
   uint8_t cmd;
   uint8_t ccode;
   uint8_t len;
   uint8_t data[16];
} COB_rtm_eeprom_read_rs;

COB_rtm_eeprom_read_rs* COB_rtm_eeprom_read(const COB_ctx* ctx,
					          uint8_t  ipmb_addr,
					          uint8_t  eeprom_sel,
					          uint8_t  address,
					          uint8_t  length,
					             void* rs_buf);

enum { COB_RTM_EEPROM_WRITE_RS_SIZE = COB_CCODE_RS_SIZE };
typedef COB_ccode_rs COB_rtm_eeprom_write_rs;

COB_rtm_eeprom_write_rs* COB_rtm_eeprom_write(const COB_ctx* ctx,
					            uint8_t  ipmb_addr,
					            uint8_t  eeprom_sel,
					            uint8_t  address,
					            uint8_t  length,
					            uint8_t* data,
					               void* rs_buf);
enum { COB_FRU_INFO_READ_RS_SIZE = 19 };
typedef struct
{
   uint8_t cmd;
   uint8_t ccode;
   uint8_t len;
   uint8_t data[16];
} COB_fru_info_read_rs;

COB_fru_info_read_rs* COB_fru_info_read(const COB_ctx* ctx,
					      uint8_t  ipmb_addr,
					      uint8_t  fru,
					     uint16_t  address,
					      uint8_t  length,
					         void* rs_buf);

enum { COB_FRU_INFO_WRITE_RS_SIZE = 3 };
typedef struct
{
  uint8_t cmd;
  uint8_t ccode;
  uint8_t len;
} COB_fru_info_write_rs;

COB_fru_info_write_rs* COB_fru_info_write(const COB_ctx* ctx,
				                uint8_t  ipmb_addr,
					        uint8_t  fru,
					       uint16_t  address,
					        uint8_t  length,
					        uint8_t* data,
					           void* rs_buf);

enum { COB_RAW_I2C_READ_RS_SIZE = 19 };
typedef struct
{
   uint8_t cmd;
   uint8_t ccode;
   uint8_t len;
   uint8_t data[16];
} COB_raw_i2c_read_rs;

COB_raw_i2c_read_rs* COB_raw_i2c_read(const COB_ctx* ctx,
				            uint8_t  ipmb_addr,
				            uint8_t  i2c_bus,
				            uint8_t  dev_addr,
				            uint8_t  length,
				            uint8_t* data,
				               void* rs_buf);

enum { COB_RAW_I2C_WRITE_RS_SIZE = COB_CCODE_RS_SIZE };
typedef COB_ccode_rs COB_raw_i2c_write_rs;

COB_raw_i2c_write_rs* COB_raw_i2c_write(const COB_ctx* ctx,
					      uint8_t  ipmb_addr,
					      uint8_t  i2c_bus,
					      uint8_t  dev_addr,
					      uint8_t  length,
					      uint8_t* data,
					         void* rs_buf);

enum { COB_I2C_TEST_RS_SIZE = 3 };
typedef struct
{
   uint8_t cmd;
   uint8_t ccode;
   uint8_t error;
} COB_i2c_test_rs;

COB_i2c_test_rs* COB_i2c_test(const COB_ctx* ctx,
			            uint8_t  ipmb_addr,
			            uint8_t  bay,
			            uint8_t  rce_mask,
			            uint8_t  pwr_mask,
			               void* rs_buf);

enum { COB_BAY_POWER_RS_SIZE = 6 };
typedef struct
{
   uint8_t  cmd;
   uint8_t  ccode;
   uint16_t voltage;
   uint16_t current;
} COB_bay_power_rs;

COB_bay_power_rs* COB_bay_power(const COB_ctx* ctx,
				      uint8_t  ipmb_addr,
				      uint8_t  bay,
				         void* rs_buf);

enum { COB_BAY_TEMPS_RS_SIZE = 4 };
typedef struct
{
   uint8_t cmd;
   uint8_t ccode;
   uint8_t btemp;
   uint8_t jtemp;
} COB_bay_temp_rs;

COB_bay_temp_rs* COB_bay_temp(const COB_ctx* ctx,
			            uint8_t  ipmb_addr,
			            uint8_t  bay,
			            uint8_t  rce,
			               void* rs_buf);

enum { COB_GET_CMB_GPIO_RS_SIZE = 8 };
typedef struct
{
   uint8_t cmd;
   uint8_t ccode;
   uint8_t rce_det;
   uint8_t rce_rst;
   uint8_t rce_rdy;
   uint8_t rce_dne;
   uint8_t rce_vok;
   uint8_t power;
} COB_get_cmb_gpio_rs;

COB_get_cmb_gpio_rs* COB_get_cmb_gpio(const COB_ctx* ctx,
				            uint8_t  ipmb_addr,
				            uint8_t  bay,
				               void* rs_buf);

enum { COB_GET_CEN_GPIO_RS_SIZE = 6 };
typedef struct
{
   uint8_t cmd;
   uint8_t ccode;
   uint8_t cen_vok;
   uint8_t sfp_det;
   uint8_t sfp_los;
   uint8_t power;
} COB_get_cen_gpio_rs;

COB_get_cen_gpio_rs* COB_get_cen_gpio(const COB_ctx* ctx,
				            uint8_t  ipmb_addr,
				               void* rs_buf);

enum { COB_GET_RTM_GPIO_RS_SIZE = 5 };
typedef struct
{
   uint8_t cmd;
   uint8_t ccode;
   uint8_t handle_closed;
   uint8_t rmb_present;
   uint8_t rmb_enabled;
} COB_get_rtm_gpio_rs;

COB_get_rtm_gpio_rs* COB_get_rtm_gpio(const COB_ctx* ctx,
				            uint8_t  ipmb_addr,
				               void* rs_buf);


enum { COB_GET_CMB_RAW_GPIO_RS_SIZE = 7 };
typedef struct
{
   uint8_t cmd;
   uint8_t ccode;
   uint8_t reg[5];
} COB_get_cmb_raw_gpio_rs;

COB_get_cmb_raw_gpio_rs* COB_get_cmb_raw_gpio(const COB_ctx* ctx,
					            uint8_t  ipmb_addr,
					            uint8_t  bay,
					            uint8_t  reg,
					               void* rs_buf);

enum { COB_GET_RTM_RAW_GPIO_RS_SIZE = 3 };
typedef struct
{
   uint8_t cmd;
   uint8_t ccode;
   uint8_t reg;
} COB_get_rtm_raw_gpio_rs;

COB_get_rtm_raw_gpio_rs* COB_get_rtm_raw_gpio(const COB_ctx* ctx,
					            uint8_t  ipmb_addr,
					            uint8_t  reg,
					               void* rs_buf);

enum { COB_DUMP_PERIOD_SET_RS_SIZE = COB_CCODE_RS_SIZE };
typedef struct COB_ccode_rs COB_dump_period_set_rs;

COB_dump_period_set_rs* COB_dump_period_set(const COB_ctx* ctx,
					          uint8_t  ipmb_addr,
					          uint8_t  idx,
					         uint16_t  seconds,
					             void* rs_buf);

enum { COB_DUMP_PERIOD_GET_RS_SIZE = 4 };
typedef struct
{
   uint8_t  cmd;
   uint8_t  ccode;
   uint16_t secs;
} COB_dump_period_get_rs;

COB_dump_period_get_rs* COB_dump_period_get(const COB_ctx* ctx,
					          uint8_t  ipmb_addr,
					          uint8_t  idx,
					             void* rs_buf);

enum { COB_DUMP_RS_SIZE = COB_CCODE_RS_SIZE };
typedef COB_ccode_rs COB_dump_rs;

COB_dump_rs* COB_dump(const COB_ctx* ctx,
		            uint8_t  ipmb_addr,
		            uint8_t  idx,
		               void* rs_buf);

enum { COB_FRU_INFO_ID_FIXUP_RS_SIZE = COB_CCODE_RS_SIZE };
typedef COB_ccode_rs COB_fru_info_id_fixup_rs;

COB_fru_info_id_fixup_rs* COB_fru_info_id_fixup(const COB_ctx* ctx,
						      uint8_t  ipmb_addr,
						      uint8_t  fru,
						         void* rs_buf);

enum { COB_FRU_INFO_ASSET_TAG_SET_RS_SIZE = COB_CCODE_RS_SIZE };
typedef COB_ccode_rs COB_fru_info_asset_tag_set_rs;

COB_fru_info_asset_tag_set_rs* COB_fru_info_asset_tag_set(const COB_ctx* ctx,
							  uint8_t ipmb_addr,
							  uint8_t fru,
							  const char* tag,
							  void* rs_buf);

enum { COB_SHELF_INFO_REFRESH_RS_SIZE = COB_CCODE_RS_SIZE };
typedef COB_ccode_rs COB_shelf_info_refresh_rs;

COB_shelf_info_refresh_rs* COB_shelf_info_refresh(const COB_ctx* ctx,
						        uint8_t  ipmb_addr,
						           void* rs_buf);

enum { COB_COLD_DATA_RESET_RS_SIZE = COB_CCODE_RS_SIZE };
typedef COB_ccode_rs COB_cold_data_reset_rs;

COB_cold_data_reset_rs* COB_cold_data_reset(const COB_ctx* ctx,
					          uint8_t  ipmb_addr,
					             void* rs_buf);

enum { COB_ZOMBIE_SET_RS_SIZE = COB_CCODE_RS_SIZE };
typedef COB_ccode_rs COB_zombie_set_rs;

COB_zombie_set_rs* COB_zombie_set(const COB_ctx* ctx,
				        uint8_t  ipmb_addr,
				        uint8_t  bay_mask,
				        uint8_t  rce_mask,
				        uint8_t  zombie,
				           void* rs_buf);

#endif /* COB_CMDS_H */
