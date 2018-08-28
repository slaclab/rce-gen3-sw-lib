#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "cob_ipmc/impl/COB_cmds.h"

#include "cob_ipmc/impl/COB_util.h"
#include "cob_ipmc/impl/COB_dump.h"

static const char dashes[]  = ("----------------------------------------"
			       "----------------------------------------");
static const char ddashes[] = ("========================================"
			       "========================================");

static const char NY[] = "NY";

static const char* ipmc_head = ("| DevID.Rev "
				"| FW Rev "
				"| Man ID "
				"| Prod ID "
				"| Aux Rev  "
				"| FW Timestamp (if COB IPMC)|");

static const char* ipmc_format = ("|      %02x.%1x "
				  "| %2x.%1i.%1i "
				  "| %06x "
				  "|    %04x "
				  "| %08x "
				  "| %25s |\n");

static const char* bay_head = ("|   BAY "
			       "|Pre"
			       "|Ena"
			       "|Vok "
			       "|Voltage "
			       "| Current "
			       "|  Power  "
			       "| Alloc "
			       "|  ID                |");

static const char* bay_format =("|  %4s "
				"| %c "
				"| %c "
				"| %c "
				"| %7s "
				"|%8s "
				"| %7s "
				"| %5s "
				"| %18s |\n");

static const char* rce_head = ("|       RCE  "
			       "|Ena"
			       "|Rst"
			       "|Rdy"
			       "|Dne"
			       "|Vok"
			       "|BTemp"
			       "|JTemp"
			       "| RCE State                       |");

static const char* rce_format = ("| %10s "
				 "| %c "
				 "| %c "
				 "| %c "
				 "| %c "
				 "| %c "
				 "| %3s "
				 "| %3s "
				 "| %-31s |\n");

static const char* gpio_head   = ("| COB\\GPIO "
				  "| DET "
				  "| RST "
				  "| RDY "
				  "| DNE "
				  "|  VOK "
				  "|  PWR "
				  "|                             |");

static const char* gpio_format = ("| %8s "
				  "| %3s "
				  "| %3s "
				  "| %3s "
				  "| %3s "
				  "| %4s "
				  "| %4s "
				  "|                             |\n");

static const char* cen_head = ("| CEN "
			       "| Pre "
			       "| Ena "
			       "| VOK "
			       "| BTemp "
			       "| JTemp "
			       "|");

static const char* cen_format = ("| CEN "
				 "|  %c  "
				 "|  %c  "
				 "|  %c  "
				 "| %5s "
				 "| %5s "
				 "|%-38s|\n");

static const char* rtm_head = ("| RTM "
			       "| Pre "
			       "| Ena "
			       "| Alloc "
			       "|   Type   "
			       "| BTemp "
			       "| JTemp "
			       "|");

static const char* rtm_format = ("| %3s "
				 "|  %c  "
				 "|  %c  "
				 "| %5s "
				 "| %8s "
				 "| %5s "
				 "| %5s "
				 "|%-25s|\n");

static void dump_ipmc(const COB_ctx* ctx, int ipmb_addr);
static void dump_bay(const COB_ctx* ctx, int ipmb_addr, int bay);

static void dump_bay_gpio(const COB_ctx* ctx, int ipmb_addr, int  bay);

int COB_dump_ipmc(const char* shelf, 
		    uint16_t  slot_mask,
		     uint8_t  bay_mask,
		     uint8_t  rce_mask,
		         int  verbose)
{
   COB_ctx* ctx;

   uint8_t ipmb_addr;
   int slot, s;

   char loc[76];

   ctx = COB_open(shelf);
   if(NULL == ctx) 
   {
      printf("ERROR - Couldn't connect to shelf manager: %s\n", shelf);
      return -1;
   }

   if(verbose)
   {
      printf("%s: Dumping IPMC info for "
	     "Slot Mask: %04x\n", 
	     shelf, slot_mask);
   }
   
   printf("%s\n", ddashes);
   printf("%-80s\n", ipmc_head);
   printf("%s\n", ddashes);

   for(slot=ATCA_MIN_SLOT; slot<=ATCA_MAX_SLOT; ++slot)
   {
      s = slot-1;
      if(!(slot_mask & (1<<s))) continue;

      ipmb_addr = COB_ipmb_addr(ctx, slot);
      if(0xff == ipmb_addr) continue;

      if(!COB_slot_present(ctx, ipmb_addr)) continue;

      snprintf(loc, sizeof(loc), "%s/%i" ,shelf, slot);
      printf("| %-76s |\n", loc);
      printf("%s\n", dashes);

      dump_ipmc(ctx, ipmb_addr);

      printf("%s\n", dashes);
   }
   
   printf("%s\n", ddashes);
   COB_close(ctx);	 
   
   return 0;
}


int COB_dump_bay(const char* shelf, 
		   uint16_t  slot_mask,
		    uint8_t  bay_mask,
		    uint8_t  rce_mask,
		        int  verbose)
{
   COB_ctx* ctx;

   uint8_t ipmb_addr;
   int slot, s;
   int bay;

   char loc[76];

   ctx = COB_open(shelf);
   if(NULL == ctx) 
   {
      printf("ERROR - Couldn't connect to shelf manager: %s\n", shelf);
      return -1;
   }

   if(verbose)
   {
      printf("%s: Dumping BAY info for "
	     "Slot Mask: %04x, Bay Mask: %02x\n", 
	     shelf, slot_mask, bay_mask);
   }
   
   printf("%s\n", ddashes);
   printf("%-80s\n", bay_head);
   printf("%s\n", ddashes);

   for(slot=ATCA_MIN_SLOT; slot<=ATCA_MAX_SLOT; ++slot)
   {
      s = slot-1;
      if(!(slot_mask & (1<<s))) continue;

      ipmb_addr = COB_ipmb_addr(ctx, slot);
      if(0xff == ipmb_addr) continue;

      if(!COB_slot_present(ctx, ipmb_addr)) continue;

      snprintf(loc, sizeof(loc), "%s/%i" ,shelf, slot);
      printf("| %-76s |\n", loc);
      printf("%s\n", dashes);

      for(bay=0; bay<COB_MAX_BAYS; ++bay)
      {
	 if(!(bay_mask & (1<<bay))) continue;
	 dump_bay(ctx, ipmb_addr, bay);
      }
      printf("%s\n", dashes);
   }
   
   printf("%s\n", ddashes);
   COB_close(ctx);	 
   
   return 0;
}

int COB_dump_rce(const char* shelf, 
		   uint16_t  slot_mask,
		    uint8_t  bay_mask,
		    uint8_t  rce_mask,
		        int  verbose)
{
   COB_ctx* ctx;

   uint8_t cmb_rs_buf[COB_GET_CMB_GPIO_RS_SIZE];
   COB_get_cmb_gpio_rs* cmb_rs;

   uint8_t temp_rs_buf[COB_BAY_TEMPS_RS_SIZE];
   COB_bay_temp_rs* temp_rs;

   uint8_t bsi_rs_buf[COB_BSI_READ_RS_SIZE];
   COB_bsi_read_rs* bsi_rs;

   uint8_t ipmb_addr;
   int slot, s;

   int bay;
   int rce;
   int first_rce;

   char loc[80];
   
   char rce_name[11];
   char enabled;
   char reset;
   char ready;
   char done;
   char vok;
   char btemp[4];
   char jtemp[4];
   char state[26];

   ctx = COB_open(shelf);
   if(NULL == ctx) 
   {
      printf("ERROR - Couldn't connect to shelf manager: %s\n", shelf);
      return -1;
   }

   if(verbose)
   {
      printf("%s: Dumping RCE info for "
	     "Slot Mask: %04x, Bay Mask: %02x, RCE Mask: %02x\n", 
	     shelf, slot_mask, bay_mask, rce_mask);
   }
   
   printf("%s\n", ddashes);
   printf("%-80s\n", rce_head);
   printf("%s\n", ddashes);

   for(slot=ATCA_MIN_SLOT; slot<=ATCA_MAX_SLOT; ++slot) 
   {
      s = slot-1;
      if(!(slot_mask & (1<<s))) continue;

      ipmb_addr = COB_ipmb_addr(ctx, slot);
      if(0xff == ipmb_addr) continue;
      
      if(!COB_slot_present(ctx, ipmb_addr)) continue;

      snprintf(loc, sizeof(loc), "%s/%i" ,shelf, slot);
      printf("| %-76s |\n", loc);
      printf("%s\n", dashes);

      for(bay=0; bay<COB_MAX_BAYS; ++bay)
      {
	 if(!(bay_mask & (1<<bay))) continue;
	 
	 cmb_rs = COB_get_cmb_gpio(ctx, ipmb_addr, bay, cmb_rs_buf);
	 if(cmb_rs == NULL || cmb_rs->ccode) continue;
	 
	 first_rce = 1;
	 for(rce=0; rce<COB_BAY_MAX_RCES; ++rce)
	 {
	    if(!(rce_mask & (1<<rce))) continue;
	    if(!(cmb_rs->rce_det & (1<<rce))) continue;
	    
	    if(first_rce)
	    {
	       snprintf(rce_name, sizeof(rce_name), "%4s: RCE%i",
			COB_util_get_bay_name(bay), rce);
	       first_rce = 0;
	    }
	    else
	    {
	       snprintf(rce_name, sizeof(rce_name), "%4s  RCE%i ", "", rce);
	    }
	    
	    enabled = NY[COB_bay_enabled(ctx, ipmb_addr, bay)];
	    reset   = NY[(cmb_rs->rce_rst&(1<<rce))>>rce];
	    ready   = NY[(cmb_rs->rce_rdy&(1<<rce))>>rce];
	    done    = NY[(cmb_rs->rce_dne&(1<<rce))>>rce];
	    vok     = NY[(cmb_rs->rce_vok == COB_VOK_MASK)];

	    temp_rs = COB_bay_temp(ctx, ipmb_addr, bay, rce, temp_rs_buf);

	    snprintf(btemp, sizeof(btemp), "%4s ", "");
	    snprintf(jtemp, sizeof(btemp), "%4s ", "");
	    if((temp_rs != NULL) && (0x00 == temp_rs->ccode))
	    {
	       if(COB_INVALID_TEMP != temp_rs->btemp)
	       {
		  snprintf(btemp, sizeof(btemp), "%3iC", temp_rs->btemp);
	       }

	       if(COB_INVALID_TEMP != temp_rs->jtemp)
	       {
		  snprintf(jtemp, sizeof(jtemp), "%3iC", temp_rs->jtemp);
	       }
	    }


	    bsi_rs = COB_bsi_read(ctx, ipmb_addr, bay, rce, 
				  RCE_BSI_STATUS_ADDR, 0x01, bsi_rs_buf);
	    if((bsi_rs != NULL) && (0x00 == bsi_rs->ccode))
	    {
	       snprintf(state, sizeof(state), "0x%02x", bsi_rs->data[0]);
	    }
	    else
	    {
	       snprintf(state, sizeof(state), "%s", "");
	    }

	    printf(rce_format, rce_name, enabled, reset, ready, done, vok, 
		   btemp, jtemp, state);
	 }	   
      }
      printf("%s\n", dashes);
   }
   
   printf("%s\n", ddashes);
   COB_close(ctx);	 
   
   return 0;
}

int COB_dump_sfp(const char* shelf, 
		    uint16_t slot_mask, 
		    uint8_t  bay_mask,
		    uint8_t  rce_mask,
		        int  verbose)
{ 
   //COB_ctx* ctx;
   printf("Dump SFP is not yet supported.\n");

   return 1;
}

int COB_dump_bay_gpio(const char* shelf, 
		        uint16_t  slot_mask,
		         uint8_t  bay_mask,
		         uint8_t  rce_mask,
		             int  verbose)
{
   COB_ctx* ctx;

   uint8_t ipmb_addr;
   int slot, s;
   int bay;

   char loc[32];

   ctx = COB_open(shelf);
   if(NULL == ctx) 
   {
      printf("ERROR - Couldn't connect to shelf manager: %s\n", shelf);
      return -1;
   }

   if(verbose)
   {
      printf("%s: Dumping GPIO for Slot Mask: %04x, Bay Mask: %02x\n", 
	     shelf, slot_mask, bay_mask);
   }
   
   printf("%s\n", ddashes);
   printf("%-80s\n", gpio_head);
   printf("%s\n", ddashes);

   for(slot=ATCA_MIN_SLOT; slot<=ATCA_MAX_SLOT; ++slot)
   {
      s = slot-1;
      if(!(slot_mask & (1<<s))) continue;
      
      //lines = 0;
      ipmb_addr = COB_ipmb_addr(ctx, slot);
      if(0xff == ipmb_addr) continue;

      if(!COB_slot_present(ctx, ipmb_addr)) continue;

      snprintf(loc, sizeof(loc), "%s/%i", shelf, slot);
      printf("| %-76s |\n", loc);
      printf("%s\n", dashes);

      for(bay=0; bay<COB_MAX_BAYS; ++bay)
      {
	 if(!(bay_mask & (1<<bay))) continue;
	 dump_bay_gpio(ctx, ipmb_addr, bay);
      }
      printf("%s\n", dashes);
   }
   
   printf("%s\n", ddashes);
   COB_close(ctx);	 
   
   return 0;
}

int COB_dump_rtm(const char* shelf, 
		   uint16_t  slot_mask, 
		    uint8_t  bay_mask,
		    uint8_t  rce_mask,
		        int  verbose)
{
   COB_ctx* ctx;

   uint8_t ipmb_addr;
   int slot, s;

   uint8_t gpio_rs_buf[COB_GET_RTM_GPIO_RS_SIZE];
   COB_get_rtm_gpio_rs* gpio_rs;

   uint8_t temp_rs_buf[COB_BAY_TEMPS_RS_SIZE];
   COB_bay_temp_rs* temp_rs;

   uint8_t info_buf[RTM_INFO_SIZE];
   RTM_info* info;

   char present;
   char enabled;
   char alloc[8];
   char type[11];
   char btemp[8];
   char jtemp[8];

   char loc[81];

   ctx = COB_open(shelf);
   if(NULL == ctx)
   {
      printf("ERROR - Couldn't connect to shelf manager: %s\n", shelf);
      return -1;
   }

   if(verbose)
   {
      printf("%s: Dumping RTM bay information, Slot Mask %04x\n", 
	     shelf, slot_mask);
   }

   printf("%s\n", ddashes);
   printf("%-80s\n", rtm_head);
   printf("%s\n", ddashes);
   
   for(slot=ATCA_MIN_SLOT; slot<=ATCA_MAX_SLOT; ++slot)
   {
      s = slot-1;
      if(!(slot_mask & (1<<s))) continue;
      
      ipmb_addr = COB_ipmb_addr(ctx, slot);
      if(0xff == ipmb_addr) continue;

      if(!COB_slot_present(ctx, ipmb_addr)) continue;

      present = NY[COB_bay_present(ctx, ipmb_addr, RTM)];
      enabled = NY[COB_bay_enabled(ctx, ipmb_addr, RTM)];

      if('N' == present)  continue;

      snprintf(loc, 80, "%s/%i", shelf, slot);
      printf("| %-76s |\n", loc);
      printf("%s\n", dashes);
      
      info = COB_get_rtm_info(ctx, ipmb_addr, 0, info_buf);
      if(NULL == info) 
      {
	 snprintf(alloc, sizeof(alloc), "%s", "");
	 snprintf(type, sizeof(type), "%s", "");
      }
      else
      {
	 snprintf(alloc, sizeof(alloc), "%2i.%1i", 
		  (info->power*2)/10, 
		  (info->power*2)%10);

	 snprintf(type, sizeof(type), "%08x", info->type);
      }

      temp_rs = COB_bay_temp(ctx, ipmb_addr, RTM, 0, temp_rs_buf);
      if((NULL == temp_rs) || temp_rs->ccode) 
      {
	 snprintf(btemp, sizeof(btemp), "%s", "");
	 snprintf(jtemp, sizeof(jtemp), "%s", "");
      }
      else
      {
	 if(128 == temp_rs->btemp)
	 {
	    snprintf(btemp, sizeof(btemp), "%s", "");
	 }
	 else
	 {
	    snprintf(btemp, sizeof(btemp), "%02iC", temp_rs->btemp);
	 }

	 if(128 == temp_rs->jtemp)
	 {
	    snprintf(jtemp, sizeof(jtemp), "%s", "");
	 }
	 else
	 {
	    snprintf(jtemp, sizeof(jtemp), "%02iC", temp_rs->jtemp);
	 }
      }     

      printf(rtm_format, "RTM", present, enabled, 
	     alloc, type, btemp, jtemp, "");

      gpio_rs = COB_get_rtm_gpio(ctx, ipmb_addr, gpio_rs_buf);
      if((NULL == gpio_rs) || gpio_rs->ccode) continue;

      if(!gpio_rs->rmb_present) continue;

      info = COB_get_rtm_info(ctx, ipmb_addr, 1, info_buf);
      if(NULL == info) 
      {
	 snprintf(alloc, sizeof(alloc), "%s", "");
	 snprintf(type, sizeof(type), "%s", "");
      }
      else
      {
	 snprintf(alloc, sizeof(alloc), "%2i.%1i", 
		  (info->power*2)/10, 
		  (info->power*2)%10);

	 snprintf(type, sizeof(type), "%08x", info->type);
      }

      snprintf(btemp, sizeof(btemp), "%s", "");
      snprintf(jtemp, sizeof(jtemp), "%s", "");

      printf(rtm_format, "RMB", 
	     NY[gpio_rs->rmb_present], NY[gpio_rs->rmb_enabled],
	     alloc, type, btemp, jtemp, "");

      printf("%s\n", dashes);
   }   

   printf("%s\n", ddashes);

   return 0;
}

int COB_dump_cen(const char* shelf, 
		   uint16_t  slot_mask, 
		    uint8_t  bay_mask,
		    uint8_t  rce_mask,
		        int  verbose)
{
   COB_ctx* ctx;

   uint8_t ipmb_addr;
   int slot, s;

   uint8_t gpio_rs_buf[COB_GET_CEN_GPIO_RS_SIZE];
   COB_get_cen_gpio_rs* gpio_rs;

   uint8_t temp_rs_buf[COB_BAY_TEMPS_RS_SIZE];
   COB_bay_temp_rs* temp_rs;

   char present;
   char enabled;
   char vok;
   char jtemp[6];
   char btemp[6];

   char loc[81];

   ctx = COB_open(shelf);
   if(NULL == ctx)
   {
      printf("ERROR - Couldn't connect to shelf manager: %s\n", shelf);
      return -1;
   }

   if(verbose)
   {
      printf("%s: Dumping CEN bay information, Slot Mask %04x\n", 
	     shelf, slot_mask);
   }

   printf("%s\n", ddashes);
   printf("%-80s\n", cen_head);
   printf("%s\n", ddashes);

   for(slot=ATCA_MIN_SLOT; slot<=ATCA_MAX_SLOT; ++slot)
   {
      s = slot-1;
      if(!(slot_mask & (1<<s))) continue;
      
      ipmb_addr = COB_ipmb_addr(ctx, slot);
      if(0xff == ipmb_addr) continue;

      if(!COB_slot_present(ctx, ipmb_addr)) continue;
      
      present = 'Y';

      snprintf(loc, 80, "%s/%i", shelf, slot);
      printf("| %-76s |\n", loc);
      printf("%s\n", dashes);

      enabled = NY[COB_bay_enabled(ctx, ipmb_addr, CEN)];

      gpio_rs = COB_get_cen_gpio(ctx, ipmb_addr, gpio_rs_buf);
      if((NULL == gpio_rs) || gpio_rs->ccode) 
      {
	 vok = ' ';
      }
      else
      {
	 vok = (gpio_rs->cen_vok == COB_VOK_MASK) ? 'Y' : 'N';
      }

      temp_rs = COB_bay_temp(ctx, ipmb_addr, CEN, 0, temp_rs_buf);
      if((NULL == temp_rs) || temp_rs->ccode)
      {
	 sprintf(jtemp, "%5s","Error");
	 sprintf(btemp, "%5s","Error");
      }
      else
      {
	 sprintf(jtemp, "%2iC ", temp_rs->jtemp);
	 sprintf(btemp, "%2iC ", temp_rs->btemp);
      }

      printf(cen_format, present, enabled, vok, btemp, jtemp, "");
      printf("%s\n", dashes);

   }

   printf("%s\n",ddashes);

   return 0;
}

static void dump_ipmc(const COB_ctx* ctx, int ipmb_addr)
{
  uint8_t id_rs_buf[IPMI_GET_DEVICE_ID_RS_SIZE];
  IPMI_get_device_id_rs* id_rs;

  time_t time;
  char timebuf[26];

  id_rs = IPMI_get_device_id(ctx, ipmb_addr, id_rs_buf);

  if((id_rs != NULL) && (0 == id_rs->ccode))
  {
    time = ( (id_rs->aux_firmware_rev[0]<<24)
	    |(id_rs->aux_firmware_rev[1]<<16)
	    |(id_rs->aux_firmware_rev[2]<<8)
	     |(id_rs->aux_firmware_rev[3]<<0));
    strftime(timebuf, 26, "%Y-%m-%d %H:%M:%S", gmtime(&time));

    printf(ipmc_format,
	   id_rs->dev_id,
	   (id_rs->dev_rev&0xf),
	   id_rs->firmware_rev1,
	   ((id_rs->firmware_rev2&0xf0)>>4),
	   ((id_rs->firmware_rev2&0x0f)>>0),
	   ( (id_rs->manufacturer_id[2]<<16)
	    |(id_rs->manufacturer_id[1]<<8)
	    |(id_rs->manufacturer_id[0]<<0)),
	   ( (id_rs->product_id[1]<<8)
	    |(id_rs->product_id[0]<<0)),
	   time, timebuf);
  }
  else if(id_rs != NULL)
  {
    printf("ERROR - cmd returned %02x\n", id_rs->ccode);
  }
  else
  {
    printf("ERROR - cmd returned NULL\n");
  }

  return;
} 

static void dump_bay(const COB_ctx* ctx, int ipmb_addr, int bay)
{
   uint8_t id_rs_buf[COB_ID_PROM_READ_RS_SIZE];
   COB_id_prom_read_rs* id_rs;

   uint8_t cmb_rs_buf[COB_GET_CMB_GPIO_RS_SIZE];
   COB_get_cmb_gpio_rs* cmb_rs;

   uint8_t cen_rs_buf[COB_GET_CEN_GPIO_RS_SIZE];
   COB_get_cen_gpio_rs* cen_rs;

   uint8_t power_rs_buf[COB_BAY_POWER_RS_SIZE];
   COB_bay_power_rs* power_rs;

   uint8_t rtm_buf[RTM_INFO_SIZE];
   RTM_info* rtm_info;
   
   char bay_name[5];
   char present;
   char enabled;
   char vok = ' ';
   char alloc[6];
   char voltage[8];
   char current[9];
   char power[8];
   char id[17];

   int rtm_fifths = 0;
   int cen_fifths = 185; // This is hardcoded for now to match the IPMC

   if(bay<COB_MAX_BAYS)
   {
      strncpy(bay_name, COB_util_get_bay_name(bay), sizeof(bay_name));

      present = NY[COB_bay_present(ctx, ipmb_addr, bay)];
     
      enabled = NY[COB_bay_enabled(ctx, ipmb_addr, bay)];

      if(bay <= DTM)
      {
	 cmb_rs = COB_get_cmb_gpio(ctx, ipmb_addr, bay, cmb_rs_buf);
	 if((cmb_rs != NULL) && (0 == cmb_rs->ccode))
	 {
 	    vok = (cmb_rs->rce_vok == COB_VOK_MASK) ? 'Y' : 'N';
	    snprintf(alloc, sizeof(alloc), "%2i.%1iW", 
		     (cmb_rs->power)*2/10,
		     (cmb_rs->power)*2%10);
	 }
	 else
	 {
	    snprintf(alloc, sizeof(alloc), "%s",  "");
	 }
      }
      else if(bay == RTM)
      {
	 rtm_info = COB_get_rtm_info(ctx, ipmb_addr, 0, rtm_buf);
	 if(NULL != rtm_info)
	 {
	    rtm_fifths += rtm_info->power;
	 }
	 
	 rtm_info = COB_get_rtm_info(ctx, ipmb_addr, 1, rtm_buf);
	 if(NULL != rtm_info)
	 {
	    rtm_fifths += rtm_info->power;
	 }

	 if('Y' == present)
	 {
	    snprintf(alloc, sizeof(alloc), "%2i.%1iW",
		     (rtm_fifths*2)/10,
		     (rtm_fifths*2)%10);
	 }
	 else
	 {
	    snprintf(alloc, sizeof(alloc), "%s", "");
	 }
      }
      else if(bay == CEN)
      {
	 cen_rs = COB_get_cen_gpio(ctx, ipmb_addr, cen_rs_buf);
 
	 if((cen_rs != NULL) && (0 == cen_rs->ccode))
	 {
	    vok = (cen_rs->cen_vok == COB_VOK_MASK) ? 'Y' : 'N';
	    if(0 != cen_rs->power)
	    {
	      cen_fifths = cen_rs->power;
	    }
	 }
	 snprintf(alloc, sizeof(alloc), "%2i.%1iW",
		  (cen_fifths*2)/10,
		  (cen_fifths*2)%10);
      }
      else
      {
	 vok   = ' ';
	 snprintf(alloc, 6, "%s", "");
      }

      power_rs = COB_bay_power(ctx, ipmb_addr, bay, power_rs_buf);
      if((NULL != power_rs) && (0 == power_rs->ccode)) 
      {
	 snprintf(voltage, sizeof(voltage), "%2i.%03iV", 
		  power_rs->voltage/1000,
		  power_rs->voltage%1000);
	 snprintf(current, sizeof(current), "%6imA", 
		  power_rs->current);
	 snprintf(power, sizeof(power), "%2i.%03iW",
		  (power_rs->voltage*power_rs->current)/1000000,
		  (power_rs->voltage*power_rs->current)%1000000/1000);
      }
      else
      {
	 snprintf(voltage, sizeof(voltage), "%s","");
	 snprintf(current, sizeof(current), "%s","");      
	 snprintf(power,   sizeof(power),   "%s","");      
      }

      id_rs = COB_id_prom_read(ctx, ipmb_addr, bay, id_rs_buf);
      if((NULL == id_rs) || id_rs->ccode)
      {
	 snprintf(id,   sizeof(id),   "%s","");
      }
      else
      {
	 snprintf(id, sizeof(id), "%02x%02x%02x%02x%02x%02x%02x%02x",
		  id_rs->id[0],
		  id_rs->id[1],
		  id_rs->id[2],
		  id_rs->id[3],
		  id_rs->id[4],
		  id_rs->id[5],
		  id_rs->id[6],
		  id_rs->id[7]);
      }

      printf(bay_format, bay_name, present, enabled, vok, 
	     voltage, current, power, alloc, id);
   }

   return;
}

static void dump_bay_gpio(const COB_ctx* ctx, int ipmb_addr, int bay)
{
   uint8_t cmb_rs_buf[COB_GET_CMB_GPIO_RS_SIZE];
   COB_get_cmb_gpio_rs* cmb_rs;

   uint8_t cen_rs_buf[COB_GET_CEN_GPIO_RS_SIZE];
   COB_get_cen_gpio_rs* cen_rs;

   const char* label = COB_util_get_bay_name(bay);

   char det[4];
   char rst[4];
   char rdy[4];
   char dne[4];
   char vok[5];
   char pwr[5];
   
   sprintf(det, "%s", "");
   sprintf(rst, "%s", "");
   sprintf(rdy, "%s", "");
   sprintf(dne, "%s", "");
   sprintf(vok, "%s", "");
   sprintf(pwr, "%s", "");

   if(bay<=DTM)
   {
      cmb_rs = COB_get_cmb_gpio(ctx, ipmb_addr, bay, cmb_rs_buf);
      if((NULL == cmb_rs) || cmb_rs->ccode) return;

      snprintf(det, sizeof(det), "0x%1x", cmb_rs->rce_det);
      snprintf(rst, sizeof(rst), "0x%1x", cmb_rs->rce_rst);
      snprintf(rdy, sizeof(rdy), "0x%1x", cmb_rs->rce_rdy);
      snprintf(dne, sizeof(dne), "0x%1x", cmb_rs->rce_dne);
      snprintf(vok, sizeof(vok), "0x%02x", cmb_rs->rce_vok);
      snprintf(pwr, sizeof(pwr), "0x%02x", cmb_rs->power);
   }
   else if(bay == RTM)
   {
      return;
   }
   else if(bay == CEN)
   {
      cen_rs = COB_get_cen_gpio(ctx, ipmb_addr, cen_rs_buf);
      if((NULL == cen_rs) || cen_rs->ccode) return;
      
      snprintf(vok, sizeof(vok), "0x%02x", cen_rs->cen_vok);
   }

   printf(gpio_format, label, det, rst, rdy, dne, vok, pwr);
   
   return;
}

