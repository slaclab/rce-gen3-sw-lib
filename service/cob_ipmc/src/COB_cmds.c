#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <freeipmi/freeipmi.h>
#include <freeipmi/api/ipmi-api.h>

#include "cob_ipmc/impl/COB_cmds.h"
#include "cob_ipmc/impl/COB_cmd_defs.h"
#include "cob_ipmc/impl/COB_util.h"

enum 
{ 
   SLAC_COB_NETFN = 0x34,
};

uint8_t COB_ipmb_addr(const COB_ctx* ctx, unsigned int slot)
{
   int ret;
   uint8_t rq[6];
   uint8_t rs[9];
   
   rq[0] = 0x01; // Get Address Info Command
   rq[1] = 0x00; // PICMG Idetifier
   rq[2] = 0x00; // FRU Device ID
   rq[3] = 0x03; // Address Key Type (Physical)
   rq[4] = slot; // Address Key
   rq[5] = 0x00; // Site Type (Front Board)

   if(0 == slot)
   {
      return 0x20;
   }

   ret = ipmi_cmd_raw_ipmb((ipmi_ctx_t)ctx, 
			   0, // channel number
			   0x20,
			   0, // rs_lun
			   IPMI_NET_FN_GROUP_EXTENSION_RQ,
			   &rq, sizeof(rq),
			   &rs, sizeof(rs));
   
   if(rs[1] || ret != 9)
   {
      return -1;
   }

   return rs[4];
}

static int ipmb_cmd(const COB_ctx* ctx,
		    uint8_t ipmb_addr,
		    uint8_t netfn,
		    uint8_t* rq, unsigned int rq_size,
		    uint8_t* rs, unsigned int rs_size)
{
   static const uint8_t channel_number = 0;
   static const uint8_t rs_lun = 0;
   int ret;

   ret = ipmi_cmd_raw_ipmb((ipmi_ctx_t) ctx,
			   channel_number,
			   ipmb_addr,
			   rs_lun,
			   netfn,
			   rq, rq_size,
			   rs, rs_size);
   
//   if(-1 == ret) printf("ipmi_ctx_errnum: %i\n", 
//			ipmi_ctx_errnum((ipmi_ctx_t)ctx));

   return ret;
}

int COB_raw_cmd(const COB_ctx* ctx,
		uint8_t ipmb_addr,
		uint8_t netfn,
		uint8_t cmd,
		uint8_t* data, unsigned int data_len,
		uint8_t* rs_buf, unsigned int rs_len)
{

   uint8_t rq_buf[64];
   int rq_buf_len = 1+data_len;

   int len;
   int i;

   if(data_len > 63) return -1;

   rq_buf[0] = cmd;
   memcpy(&rq_buf[1], data, data_len);

   if(-1 == (len = ipmb_cmd(ctx, 
			    ipmb_addr,
			    netfn,
			    rq_buf, rq_buf_len,
			    rs_buf, rs_len)))
   {
      return -1;
   }
   
   for(i=0; i<len-1; ++i)
   {
      rs_buf[i] = rs_buf[i+1];
   }

   return len-1;
}

static const uint8_t present_pins[] ={0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51};
static const uint8_t enabled_pins[] ={0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27};

int COB_slot_present(const COB_ctx* ctx, int ipmb_addr)
{
   // until we get the IPMI GetDeviceID to identify COBs,
   // use the get ID of the CEN bay to identify COBs.
   
   uint8_t id_rs_buf[COB_ID_PROM_READ_RS_SIZE];
   COB_id_prom_read_rs* id_rs;
   
   id_rs = COB_id_prom_read(ctx, ipmb_addr, CEN, id_rs_buf);
   if(NULL == id_rs)
   {
      return 0;
   }
   
   return 1;
}

int COB_bay_present(const COB_ctx* ctx, int ipmb_addr, int bay)
{
   // This uses PPS IPMI commands
   // to read the GPIO lines in the Actel. A future IPMC
   // command should make this better.
   // I may even want to fake such a command up for the 
   // public API...

   static const int PPS_GET_GPIO_STATE_RS_SIZE = 6;
   static const int PPS_GET_GPIO_STATE_CMD_SIZE = 6;
   static const uint8_t PPS_NETFN = 0x2e;
   static const uint8_t PPS_GET_GPIO_STATE_CMD = 0x38;

   uint8_t rs_buf[PPS_GET_GPIO_STATE_RS_SIZE];
   uint8_t cmd_buf[PPS_GET_GPIO_STATE_CMD_SIZE];
   
   uint8_t pin;

   int present;

   if(bay>=COB_MAX_BAYS) return 0;

   if(bay==CEN) return 1;

   pin = present_pins[bay];

   cmd_buf[0] = 0x0a;
   cmd_buf[1] = 0x40;
   cmd_buf[2] = 0x00;
   cmd_buf[3] = 0x00;
   cmd_buf[4] = 0x00;
   cmd_buf[5] = pin;

   if((-1 == COB_raw_cmd(ctx, ipmb_addr, 
			 PPS_NETFN, PPS_GET_GPIO_STATE_CMD,
			 cmd_buf, PPS_GET_GPIO_STATE_CMD_SIZE,
			 rs_buf, PPS_GET_GPIO_STATE_RS_SIZE)))
   {
      return 0;
   }
   
   if(rs_buf[0]) return 0;

   present = rs_buf[5] ? 0 : 1;
 
   return present;
}

int COB_bay_enabled(const COB_ctx* ctx, int ipmb_addr, int bay)
{
   // This uses PPS IPMI commands
   // to read the GPIO lines in the Actel. A future IPMC
   // command should make this better.
   // I may even want to fake such a command up for the 
   // public API...

   static const int PPS_GET_GPIO_STATE_RS_SIZE = 6;
   static const int PPS_GET_GPIO_STATE_CMD_SIZE = 6;
   static const uint8_t PPS_NETFN = 0x2e;
   static const uint8_t PPS_GET_GPIO_STATE_CMD = 0x38;

   uint8_t rs_buf[PPS_GET_GPIO_STATE_RS_SIZE];
   uint8_t cmd_buf[PPS_GET_GPIO_STATE_CMD_SIZE];
   
   uint8_t pin;

   int enabled;

   if(bay>=COB_MAX_BAYS) return 0;

   pin = enabled_pins[bay];

   cmd_buf[0] = 0x0a;
   cmd_buf[1] = 0x40;
   cmd_buf[2] = 0x00;
   cmd_buf[3] = 0x00;
   cmd_buf[4] = 0x00;
   cmd_buf[5] = pin;

   if((-1 == COB_raw_cmd(ctx, ipmb_addr, 
			 PPS_NETFN, PPS_GET_GPIO_STATE_CMD,
			 cmd_buf, PPS_GET_GPIO_STATE_CMD_SIZE,
			 rs_buf, PPS_GET_GPIO_STATE_RS_SIZE)))
   {
      return 0;
   }
   
   if(rs_buf[0]) return 0;

   enabled = rs_buf[5];
 
   return enabled;
}

COB_ctx* COB_open(const char* hostname)
{
   ipmi_ctx_t ipmi_ctx = NULL;
   
   if(!(ipmi_ctx = ipmi_ctx_create()))
   {
      printf("ERROR - ipmi_ctx = NULL\n");
      COB_close((COB_ctx*)ipmi_ctx);
      return(NULL);
   }

   if(0 != ipmi_ctx_open_outofband(ipmi_ctx,
				   hostname,
				   "", // username
				   "", // password
				   IPMI_AUTHENTICATION_TYPE_NONE,
				   IPMI_PRIVILEGE_LEVEL_ADMIN,
				   IPMI_SESSION_TIMEOUT_DEFAULT,
				   IPMI_RETRANSMISSION_TIMEOUT_DEFAULT,
				   IPMI_WORKAROUND_FLAGS_DEFAULT,
				   IPMI_FLAGS_DEFAULT))
   {
      COB_close((COB_ctx*)ipmi_ctx);
      return(NULL);
   }

   return (COB_ctx*)ipmi_ctx;
}

void COB_close(const COB_ctx* ctx)
{
   ipmi_ctx_t ipmi_ctx = (ipmi_ctx_t)ctx;
   ipmi_ctx_close(ipmi_ctx);
   ipmi_ctx_destroy(ipmi_ctx);
}

IPMI_get_device_id_rs* IPMI_get_device_id(const COB_ctx* ctx,
 					        uint8_t  ipmb_addr,
					           void* rs_buf)
{
  uint8_t rq_buf = 0x01;
  IPMI_get_device_id_rs* rs = (IPMI_get_device_id_rs*)rs_buf;
  
  if(-1 == ipmb_cmd(ctx,
		    ipmb_addr,
		    IPMI_NET_FN_APP_RQ,
		    &rq_buf, 1,
		    rs_buf, IPMI_GET_DEVICE_ID_RS_SIZE))
  {
    return NULL;
  }
  return rs;
}

COB_bsi_write_rs* COB_bsi_write(const COB_ctx* ctx, 
				  uint8_t ipmb_addr,
				  uint8_t bay_mask, 
				  uint8_t rce_mask,
				  uint16_t address,
				  uint16_t length,
				  uint8_t* data,
				  void* rs_buf)
{
   uint8_t rq_buf[64];
   COB_bsi_write_rs* rs = (COB_bsi_write_rs*)rs_buf;
   int rq_buf_len = 8+length;

   rq_buf[0] = COB_CMD_WR_ELEM_BSI;
   rq_buf[1] = bay_mask;
   rq_buf[2] = rce_mask;
   rq_buf[3] = 0; // Do Interrupt, not used
   rq_buf[4] = (address&0xff00)>>8;
   rq_buf[5] = (address&0x00ff)>>0;
   rq_buf[6] = (length&0xff00)>>8;
   rq_buf[7] = (length&0x00ff)>>0;
   memcpy(&rq_buf[8], data, length);

   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     SLAC_COB_NETFN,
		     rq_buf, rq_buf_len,
		     rs_buf, COB_BSI_WRITE_RS_SIZE))
   {
      return NULL;
   }
   
   return rs;
}

COB_bsi_read_rs* COB_bsi_read(const COB_ctx* ctx,
				uint8_t ipmb_addr,
				uint8_t bay,
				uint8_t rce,
				uint16_t address,
				uint16_t length,
				void* rs_buf)
{
   uint8_t rq_buf[7];
   uint8_t temp_rs[COB_BSI_READ_RS_SIZE];

   COB_bsi_read_rs* rs = (COB_bsi_read_rs*)rs_buf;

   if(length>16)
   {
      return NULL;
   }

   rq_buf[0] = COB_CMD_RD_ELEM_BSI;
   rq_buf[1] = bay;
   rq_buf[2] = rce;
   rq_buf[3] = (address&0xff00)>>8;
   rq_buf[4] = (address&0x00ff)>>0;
   rq_buf[5] = (length&0xff00)>>8;
   rq_buf[6] = (length&0x00ff)>>0;

   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     SLAC_COB_NETFN,
		     rq_buf, sizeof(rq_buf),
		     temp_rs, COB_BSI_READ_RS_SIZE))
   {
      return NULL;
   }
   
   rs->cmd   = temp_rs[0];
   rs->ccode = temp_rs[1];
   rs->len   = length;
   memcpy(rs->data, &temp_rs[2], length);
   
   return rs;
}


COB_id_prom_read_rs* COB_id_prom_read(const COB_ctx* ctx,
					uint8_t ipmb_addr,
					uint8_t bay,
					void* rs_buf)
{
   uint8_t rq_buf[2];
   COB_id_prom_read_rs* rs = (COB_id_prom_read_rs*)rs_buf;

   rq_buf[0] = COB_CMD_READ_CDB_ID;
   rq_buf[1] = bay;
   
   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     SLAC_COB_NETFN,
		     rq_buf, sizeof(rq_buf),
		     rs_buf, COB_ID_PROM_READ_RS_SIZE))
   {
      return NULL;
   }
   
   return rs;
}


COB_rce_reset_rs* COB_rce_reset(const COB_ctx* ctx,
				  uint8_t ipmb_addr,
				  uint8_t bay_mask,
				  uint8_t rce_mask,
				  void* rs_buf)
{
   uint8_t rq_buf[3];
   COB_rce_reset_rs* rs = (COB_rce_reset_rs*)rs_buf;

   rq_buf[0] = COB_CMD_RESET_RCE;
   rq_buf[1] = bay_mask;
   rq_buf[2] = rce_mask;

   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     SLAC_COB_NETFN,
		     rq_buf, sizeof(rq_buf),
		     rs_buf, COB_RCE_RESET_RS_SIZE))
   {
      return NULL;
   }

   return rs;
}

COB_rtm_eeprom_read_rs* COB_rtm_eeprom_read(const COB_ctx* ctx,
					      uint8_t ipmb_addr,
					      uint8_t eeprom_sel,
					      uint8_t address,
					      uint8_t length,
					      void* rs_buf)
{
   uint8_t rq_buf[4];
   uint8_t temp_rs[COB_RTM_EEPROM_READ_RS_SIZE];
   
   COB_rtm_eeprom_read_rs* rs = (COB_rtm_eeprom_read_rs*)rs_buf;

   if(length>16) return NULL;

   rq_buf[0] = COB_CMD_RTM_EEPROM_READ;
   rq_buf[1] = eeprom_sel;
   rq_buf[2] = address;
   rq_buf[3] = length;

   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     SLAC_COB_NETFN,
		     rq_buf, sizeof(rq_buf),
		     temp_rs, COB_RTM_EEPROM_READ_RS_SIZE))
   {
      return NULL;
   }

   rs->cmd = temp_rs[0];
   rs->ccode = temp_rs[1];
   rs->len = length;
   memcpy(rs->data, &temp_rs[2], length);

   return rs;
}

COB_rtm_eeprom_write_rs* COB_rtm_eeprom_write(const COB_ctx* ctx,
						uint8_t ipmb_addr,
						uint8_t eeprom_sel,
						uint8_t address,
						uint8_t length,
						uint8_t* data,
						void* rs_buf)
{
   uint8_t rq_buf[64];
   COB_rtm_eeprom_write_rs* rs = (COB_rtm_eeprom_write_rs*)rs_buf;
   int rq_buf_len = 4 + length;
   
   rq_buf[0] = COB_CMD_RTM_EEPROM_WRITE;
   rq_buf[1] = eeprom_sel;
   rq_buf[2] = address;
   rq_buf[3] = length;
   memcpy(&rq_buf[4], data, length);
   
   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     SLAC_COB_NETFN,
		     rq_buf, rq_buf_len,
		     rs_buf, COB_RTM_EEPROM_WRITE_RS_SIZE))
   {
      return NULL;
   }

   return rs;
}

COB_fru_info_read_rs* COB_fru_info_read(const COB_ctx* ctx,
					      uint8_t  ipmb_addr,
					      uint8_t  fru,
					     uint16_t  address,
					      uint8_t  length,
					         void* rs_buf)
{
   uint8_t rq_buf[5];
   uint8_t temp_rs[COB_FRU_INFO_READ_RS_SIZE];
   
   COB_fru_info_read_rs* rs = (COB_fru_info_read_rs*)rs_buf;

   if(length>16) return NULL;

   rq_buf[0] = IPMI_CMD_READ_FRU_DATA;
   rq_buf[1] = fru;
   rq_buf[2] = (address>>0)&0xff;
   rq_buf[3] = (address>>8)&0xff;
   rq_buf[4] = length;

   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     IPMI_NET_FN_STORAGE_RQ,
		     rq_buf, sizeof(rq_buf),
		     temp_rs, COB_FRU_INFO_READ_RS_SIZE))
   {
      return NULL;
   }

   rs->cmd = temp_rs[0];
   rs->ccode = temp_rs[1];
   rs->len = temp_rs[2];
   memcpy(rs->data, &temp_rs[3], length);

   return rs;
}

COB_fru_info_write_rs* COB_fru_info_write(const COB_ctx*  ctx,
						 uint8_t  ipmb_addr,
					         uint8_t  fru,
						uint16_t  address,
						 uint8_t  length,
						 uint8_t* data,
						 void* rs_buf)
{
   uint8_t rq_buf[64];
   COB_fru_info_write_rs* rs = (COB_fru_info_write_rs*)rs_buf;
   int rq_buf_len = 4 + length;
   
   rq_buf[0] = IPMI_CMD_WRITE_FRU_DATA;
   rq_buf[1] = fru;
   rq_buf[2] = (address>>0)&0xff;
   rq_buf[3] = (address>>8)&0xff;
   memcpy(&rq_buf[4], data, length);

   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     IPMI_NET_FN_STORAGE_RQ,
		     rq_buf, rq_buf_len,
		     rs_buf, COB_FRU_INFO_WRITE_RS_SIZE))
   {
      return NULL;
   }

   return rs;
}

COB_raw_i2c_read_rs* COB_raw_i2c_read(const COB_ctx* ctx,
					uint8_t ipmb_addr,
					uint8_t i2c_bus,
					uint8_t dev_addr,
					uint8_t length,
					uint8_t* data,
					void* rs_buf)
{
   uint8_t rq_buf[4];
   COB_raw_i2c_read_rs* rs = (COB_raw_i2c_read_rs*)rs_buf;
   uint8_t temp_rs[COB_RAW_I2C_READ_RS_SIZE];

   if(length>16) return NULL;

   rq_buf[0] = COB_CMD_I2C_READ;
   rq_buf[1] = i2c_bus;
   rq_buf[2] = dev_addr;
   rq_buf[3] = length;

   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     SLAC_COB_NETFN,
		     rq_buf, sizeof(rq_buf),
		     temp_rs, COB_RAW_I2C_READ_RS_SIZE))
   {
      return NULL;
   }

   rs->cmd = temp_rs[0];
   rs->ccode = temp_rs[1];
   rs->len = length;
   memcpy(rs->data, &temp_rs[2], length);
   
   return rs;
}


COB_raw_i2c_write_rs* COB_raw_i2c_write(const COB_ctx* ctx,
					  uint8_t ipmb_addr,
					  uint8_t i2c_bus,
					  uint8_t dev_addr,
					  uint8_t length,
					  uint8_t* data,
					  void* rs_buf)
{
   uint8_t rq_buf[64];
   COB_raw_i2c_write_rs* rs = (COB_raw_i2c_write_rs*)rs_buf;
   int rq_buf_len = 5+length;
   
   rq_buf[0] = COB_CMD_I2C_WRITE;
   rq_buf[1] = i2c_bus;
   rq_buf[2] = dev_addr;
   rq_buf[3] = length;
   memcpy(&rq_buf[4], data, length);
   
   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     SLAC_COB_NETFN,
		     rq_buf, rq_buf_len,
		     rs_buf, COB_RAW_I2C_WRITE_RS_SIZE))
   {
      return NULL;
   }

   return rs;
}


COB_i2c_test_rs* COB_i2c_test(const COB_ctx* ctx,
				uint8_t ipmb_addr,
				uint8_t bay,
				uint8_t rce_mask,
				uint8_t pwr_mask,
				void* rs_buf)
{
   uint8_t rq_buf[4];
   COB_i2c_test_rs* rs = (COB_i2c_test_rs*)rs_buf;

   rq_buf[0] = COB_CMD_I2C_TEST;
   rq_buf[1] = bay;
   rq_buf[2] = rce_mask;
   rq_buf[3] = pwr_mask;

   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     SLAC_COB_NETFN,
		     rq_buf, sizeof(rq_buf),
		     rs_buf, COB_I2C_TEST_RS_SIZE))
   {
      return NULL;
   }
   
   return rs;
}


COB_bay_power_rs* COB_bay_power(const COB_ctx* ctx,
				  uint8_t ipmb_addr,
				  uint8_t bay,
				  void* rs_buf)
{
   uint8_t rq_buf[2];
   COB_bay_power_rs* rs = (COB_bay_power_rs*)rs_buf;
   
   rq_buf[0] = COB_CMD_GET_BAY_POWER;
   rq_buf[1] = bay;
   
   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     SLAC_COB_NETFN,
		     rq_buf, sizeof(rq_buf),
		     rs_buf, COB_BAY_POWER_RS_SIZE))
   {
      return NULL;
   }
   
   // IPMC returns voltage and current LSB first
   rs->voltage = ((rs->voltage&0xff)<<8) | ((rs->voltage&0xff00)>>8);
   rs->current = ((rs->current&0xff)<<8) | ((rs->current&0xff00)>>8);
   
   return rs;
}


COB_bay_temp_rs* COB_bay_temp(const COB_ctx* ctx,
				uint8_t ipmb_addr,
				uint8_t bay,
				uint8_t rce,
				void* rs_buf)
{
   uint8_t rq_buf[3];
   COB_bay_temp_rs* rs = (COB_bay_temp_rs*)rs_buf;
   
   rq_buf[0] = COB_CMD_GET_BAY_TEMPS;
   rq_buf[1] = bay;
   rq_buf[2] = rce;

   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     SLAC_COB_NETFN,
		     rq_buf, sizeof(rq_buf),
		     rs_buf, COB_BAY_TEMPS_RS_SIZE))
   {
      return NULL;
   }

   return rs;
}

COB_get_cmb_gpio_rs* COB_get_cmb_gpio(const COB_ctx* ctx,
				      uint8_t ipmb_addr,
				      uint8_t bay,
				      void* rs_buf)
{
   uint8_t rq_buf[2];
   COB_get_cmb_gpio_rs* rs = (COB_get_cmb_gpio_rs*)rs_buf;

   if(bay>DTM) // only good for Bays 0-4 
   {
      return NULL;
   }

   rq_buf[0] = COB_CMD_GET_BAY_GPIO;
   rq_buf[1] = bay;

   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     SLAC_COB_NETFN,
		     rq_buf, sizeof(rq_buf),
		     rs_buf, COB_GET_CMB_GPIO_RS_SIZE))
   {
      return NULL;
   }
   
   return rs;
}

COB_get_cen_gpio_rs* COB_get_cen_gpio(const COB_ctx* ctx,
				      uint8_t ipmb_addr,
				      void* rs_buf)
{
   uint8_t rq_buf[2];
   COB_get_cen_gpio_rs* rs = (COB_get_cen_gpio_rs*)rs_buf;

   rq_buf[0] = COB_CMD_GET_BAY_GPIO;
   rq_buf[1] = CEN; 

   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     SLAC_COB_NETFN,
		     rq_buf, sizeof(rq_buf),
		     rs_buf, COB_GET_CEN_GPIO_RS_SIZE))
   {
      return NULL;
   }
   
   return rs;
}

COB_get_rtm_gpio_rs* COB_get_rtm_gpio(const COB_ctx* ctx,
				      uint8_t ipmb_addr,
				      void* rs_buf)
{
   uint8_t rq_buf[2];
   COB_get_rtm_gpio_rs* rs = (COB_get_rtm_gpio_rs*)rs_buf;

   rq_buf[0] = COB_CMD_GET_BAY_GPIO;
   rq_buf[1] = RTM;

   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     SLAC_COB_NETFN,
		     rq_buf, sizeof(rq_buf),
		     rs_buf, COB_GET_RTM_GPIO_RS_SIZE))
   {
      return NULL;
   }
   
   return rs;
}

COB_get_cmb_raw_gpio_rs* COB_get_cmb_raw_gpio(const COB_ctx* ctx,
					      uint8_t ipmb_addr,
					      uint8_t bay,
					      uint8_t reg,
					      void* rs_buf)
{
   uint8_t rq_buf[3];
   COB_get_cmb_raw_gpio_rs* rs = (COB_get_cmb_raw_gpio_rs*)rs_buf;
   
   if(bay>DTM) // only good for Bays 0-4 
   {
      return NULL;
   }
   
   if(reg>4) // only good for regs 0-4 (FIXME magic nubmer)
   {
      return NULL;
   }

   rq_buf[0] = COB_CMD_GET_BAY_RAW_GPIO;
   rq_buf[1] = bay;
   rq_buf[2] = reg;

   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     SLAC_COB_NETFN,
		     rq_buf, sizeof(rq_buf),
		     rs_buf, COB_GET_CMB_RAW_GPIO_RS_SIZE))
   {
      return NULL;
   }
   
   return rs;
}

COB_get_rtm_raw_gpio_rs* COB_get_rtm_raw_gpio(const COB_ctx* ctx,
					      uint8_t ipmb_addr,
					      uint8_t reg,
					      void* rs_buf)
{
   uint8_t rq_buf[2];
   COB_get_rtm_raw_gpio_rs* rs = (COB_get_rtm_raw_gpio_rs*)rs_buf;
   
   if(reg>4) // only good for regs 0-4 (FIXME magic nubmer)
   {
      return NULL;
   }

   rq_buf[0] = COB_CMD_GET_BAY_RAW_GPIO;
   rq_buf[1] = 5;
   rq_buf[2] = reg;

   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     SLAC_COB_NETFN,
		     rq_buf, sizeof(rq_buf),
		     rs_buf, COB_GET_RTM_RAW_GPIO_RS_SIZE))
   {
      return NULL;
   }
   
   return rs;
}

COB_dump_period_set_rs* COB_dump_period_set(const COB_ctx* ctx,
					      uint8_t ipmb_addr,
					      uint8_t idx,
					      uint16_t seconds,
					      void* rs_buf)
{
   uint8_t rq_buf[4];
   COB_dump_period_set_rs* rs = (COB_dump_period_set_rs*)rs_buf;

   rq_buf[0] = COB_CMD_SET_MONITOR_PER;
   rq_buf[1] = idx;
   rq_buf[2] = (seconds&0xff00)>>8;
   rq_buf[3] = (seconds&0x00ff)>>0;

   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     SLAC_COB_NETFN,
		     rq_buf, sizeof(rq_buf),
		     rs_buf, COB_DUMP_PERIOD_SET_RS_SIZE))
   {
      return NULL;
   }
   
   return rs;
}


COB_dump_period_get_rs* COB_dump_period_get(const COB_ctx* ctx,
					      uint8_t ipmb_addr,
					      uint8_t idx,
					      void* rs_buf)
{
   uint8_t rq_buf[2];
   COB_dump_period_get_rs* rs = (COB_dump_period_get_rs*)rs_buf;

   rq_buf[0] = COB_CMD_GET_MONITOR_PER;
   rq_buf[1] = idx;

   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     SLAC_COB_NETFN,
		     rq_buf, sizeof(rq_buf),
		     rs_buf, COB_DUMP_PERIOD_GET_RS_SIZE))
   {
      return NULL;
   }
   
   return rs;
}


COB_dump_rs* COB_dump(const COB_ctx* ctx,
			uint8_t ipmb_addr,
			uint8_t idx,
			void* rs_buf)
{
   uint8_t rq_buf[2];
   COB_dump_rs* rs = (COB_dump_rs*)rs_buf;

   rq_buf[0] = COB_CMD_DUMP_MONITOR;
   rq_buf[1] = idx;

   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     SLAC_COB_NETFN,
		     rq_buf, sizeof(rq_buf),
		     rs_buf, COB_DUMP_RS_SIZE))
   {
      return NULL;
   }
   
   return rs;
}

COB_fru_info_id_fixup_rs* COB_fru_info_id_fixup(const COB_ctx* ctx,
						  uint8_t ipmb_addr,
						  uint8_t fru,
						  void* rs_buf)
{
   uint8_t rq_buf[2];
   COB_fru_info_id_fixup_rs* rs = (COB_fru_info_id_fixup_rs*)rs_buf;

   rq_buf[0] = COB_CMD_FIXUP_FRU_INFO_ID;
   rq_buf[1] = fru;

   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     SLAC_COB_NETFN,
		     rq_buf, sizeof(rq_buf),
		     rs_buf, COB_FRU_INFO_ID_FIXUP_RS_SIZE))
   {
      return NULL;
   }
   
   return rs;
}

COB_fru_info_asset_tag_set_rs* COB_fru_info_asset_tag_set(const COB_ctx* ctx,
							  uint8_t ipmb_addr,
							  uint8_t fru,
							  const char* tag,
							  void* rs_buf)
{
   uint8_t rq_buf[34];
   COB_fru_info_asset_tag_set_rs* rs = (COB_fru_info_asset_tag_set_rs*)rs_buf;
   int len;

   rq_buf[0] = COB_CMD_SET_ASSET_TAG;
   rq_buf[1] = fru;
   strncpy((char*)&(rq_buf[2]), tag, 32);
   len = strlen((char*)&rq_buf[2]);
   
   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     SLAC_COB_NETFN,
		     rq_buf, len+2, 
		     rs_buf, COB_FRU_INFO_ASSET_TAG_SET_RS_SIZE))
   {
      return NULL;
   }
   
   return rs;
}

COB_shelf_info_refresh_rs* COB_shelf_info_refresh(const COB_ctx* ctx,
						    uint8_t ipmb_addr,
						    void* rs_buf)
{
   uint8_t rq_buf[1];
   COB_shelf_info_refresh_rs* rs = (COB_shelf_info_refresh_rs*)rs_buf;
   
   rq_buf[0] = COB_CMD_REFRESH_SHELF_INFO;
   
   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     SLAC_COB_NETFN,
		     rq_buf, sizeof(rq_buf),
		     rs_buf, COB_SHELF_INFO_REFRESH_RS_SIZE))
   {
      return NULL;
   }

   return rs;
}

COB_cold_data_reset_rs* COB_cold_data_reset(const COB_ctx* ctx,
					      uint8_t ipmb_addr,
					      void* rs_buf)
{
   uint8_t rq_buf[1];
   COB_cold_data_reset_rs* rs = (COB_cold_data_reset_rs*)rs_buf;
   
   rq_buf[0] = COB_CMD_RESET_COLD_DATA;
   
   if(-1 == ipmb_cmd(ctx,
		     ipmb_addr,
		     SLAC_COB_NETFN,
		     rq_buf, sizeof(rq_buf),
		     rs_buf, COB_COLD_DATA_RESET_RS_SIZE))
   {
      return NULL;
   }

   return rs;
}

COB_zombie_set_rs* COB_zombie_set(const COB_ctx* ctx,
				  uint8_t ipmb_addr,
				  uint8_t bay_mask,
				  uint8_t rce_mask,
				  uint8_t zombie,
				  void* rs_buf)
{
   uint8_t zombie_rq_buf[4];
   COB_zombie_set_rs* zombie_rs = (COB_zombie_set_rs*)rs_buf;

   uint8_t gpio_rs_buf[COB_GET_CMB_GPIO_RS_SIZE];
   COB_get_cmb_gpio_rs* gpio_rs;

   uint8_t bay;
   uint8_t rce;
   uint8_t rce_det;
   
   int ret = 0;
   int done = 0;

   zombie_rq_buf[0] = COB_CMD_SET_ZOMBIE_RCE;

   for(bay=0; bay<5; ++bay)
   {
      if(bay_mask & (1<<bay))
      {
	 zombie_rq_buf[1] = bay;
	 
	 gpio_rs = COB_get_cmb_gpio(ctx, ipmb_addr, bay, gpio_rs_buf);

	 if(NULL == gpio_rs)
	 {
	    rce_det = 0xff;
	 }
	 else
	 {
	    rce_det = gpio_rs->rce_det;
	 }

	 for(rce=0; rce<4; ++rce)
	 {
	    if((rce_mask & (1<<rce)) && (rce_det & (1<<rce)))
	    {
	       zombie_rq_buf[2] = rce;
	       zombie_rq_buf[3] = zombie;
	       
	       if(-1 == ipmb_cmd(ctx,
				 ipmb_addr,
				 SLAC_COB_NETFN,
				 zombie_rq_buf, sizeof(zombie_rq_buf),
				 rs_buf, COB_ZOMBIE_SET_RS_SIZE))
	       {
		  ret = 1;
	       }
	       else
	       {
		  done = 1;
	       }
	    }
	 }
      }
   }

   if(1 == ret)
   {
      return NULL;
   }
   else if(0 == done)
   {
      zombie_rs->cmd = COB_CMD_SET_ZOMBIE_RCE;
      zombie_rs->ccode = 0;      
   }

   return zombie_rs;
}
