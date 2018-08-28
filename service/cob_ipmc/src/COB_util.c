#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "cob_ipmc/impl/COB_cmds.h"
#include "cob_ipmc/impl/COB_util.h"

static uint8_t parse_rce(const char* rce_s);
static uint8_t parse_bay(const char* bay_s);
static uint16_t parse_slot(const char* slot_s);

static const char* bay_names[] = {"DPM0", 
				  "DPM1", 
				  "DPM2",
				  "DPM3",
				  "DTM",
				  "RTM",
				  "CEN"};


const char* COB_util_get_bay_name(int bay)
{
   if(bay<0 || bay>7) return NULL;
   return bay_names[bay];
}


int COB_invalid_rmb_type(unsigned int type)
{
   if((0 == type) || (0xffffffff == type))
   {
      return 1;
   }
   
   return 0;
}

RTM_info* COB_get_rtm_info(const COB_ctx* ctx, 
			   int ipmb_addr, 
			   int rmb, 
			   void* buf)
{
   uint8_t rs_buf[COB_RTM_EEPROM_READ_RS_SIZE];
   COB_rtm_eeprom_read_rs* rs;

   uint8_t iua_addr;
   
   RTM_info* info = (RTM_info*)buf;

   if(!rmb)
   {
      rs = COB_rtm_eeprom_read(ctx, ipmb_addr, rmb, 0x01, 0x01, rs_buf);
      
      if((NULL == rs) || rs->ccode) return NULL;
      
      iua_addr = 8*rs->data[0];

      rs = COB_rtm_eeprom_read(ctx, ipmb_addr, rmb, iua_addr+2, 0x01, rs_buf);
      
      if((NULL == rs) || rs->ccode) return NULL;
      
      if(0x01 != rs->data[0]) 
      {
	 printf("ERROR - Unknown RTM Internal Use Area version: 0x%02x\n", 
		rs->data[0]);
	 return NULL;
      }
	 
      rs = COB_rtm_eeprom_read(ctx, ipmb_addr, rmb, 
			       iua_addr+3, RTM_INFO_SIZE, rs_buf);

      if((NULL == rs) || rs->ccode) return NULL;

      memcpy(buf, rs->data, rs->len);

      return info;
   }
   else
   {
      rs = COB_rtm_eeprom_read(ctx, ipmb_addr, rmb, 
			       0x00, RTM_INFO_SIZE, rs_buf);
      
      if((NULL == rs) || rs->ccode) return NULL;
     
      memcpy(buf, rs->data, rs->len);
      return info;
   }
   
   return NULL;
}

void COB_util_get_masks(char* input, 
			char* sm_name, int sm_name_len,
			uint16_t* slot_mask,  
			uint8_t* bay_mask, 
			uint8_t* rce_mask,
			int verbose)
{
   
   char* name;
   char* slot_s;
   char* bay_s;
   char* rce_s;

   char temp_input[80];

   int i;

   strncpy(temp_input, input, 80);
   
   name = strtok(temp_input, "/");

   if((slot_s = strtok(NULL, "/")))
   {
      for(i=0; i<strlen(slot_s); ++i)
      {
	 slot_s[i] = toupper(slot_s[i]);
      }
   }

   if((bay_s = strtok(NULL, "/")))
   {
      for(i=0; i<strlen(bay_s); ++i)
      {
	 bay_s[i] = toupper(bay_s[i]);
      }
   }

   if((rce_s = strtok(NULL, "/")))
   {
      for(i=0; i<strlen(rce_s); ++i)
      {
	 rce_s[i] = toupper(rce_s[i]);
      }
   }
   
   strncpy(sm_name, name, sm_name_len);

   *rce_mask = parse_rce(rce_s);      
   *bay_mask = parse_bay(bay_s);
   *slot_mask = parse_slot(slot_s);

   if(verbose)
   {
      printf("Location %s/%s/%s/%s -> "
	     "Slot Mask %04x, Bay Mask %02x, RCE Mask %02x\n", 
	     name, slot_s, bay_s, rce_s, *slot_mask, *bay_mask, *rce_mask); 
   }
}

void COB_util_mask_usage()
{

   printf(" Each <rce address> is of the form: \n"
	  "\n"
	  "  shelfmanager_ip[/slot[/bay[/rce]]]\n"
	  "\n"
	  "  Where:\n"
	  "    shelfmanager_ip: the hostname of the shelf manager or its IP address\n"
	  "                     WARNING: THIS IS NOT THE SHELF NAME!\n"
	  "\n"
	  "               slot: the physical slot number (1-16)\n"
	  "                     or the case-insensitive keyword ALL\n"
	  "\n"
	  "                bay: the bay number (0-7) \n"
	  "                     or one of the case-insensitive keywords: \n"
	  "                     ALL, CMB, DTM, DPM, RTM, CEN\n"
	  "\n"
	  "                rce: the RCE number (0-3)\n"
	  "                     or the case-insensitive keyword ALL.\n"
	  "\n"
	  "    Leaving off the end of the address is the same \n"
	  "    as using the keyword ALL for the omitted arguments.\n"
	  "\n"
	  "    Examples:\n"
	  "      egbert-sm/4/3/2\n"
	  "      egbert-sm/4/3\n"
	  "      egbert-sm/4/\n"
	  "      egbert-sm/\n"
	  "      egbert-sm/4/ALL/2\n"
	  "      egbert-sm/4/dpm\n"
	  "      egbert-sm/4/DPM/0\n"
	  "      172.21.6.112/4/3/2\n"
	  "\n");

}

static uint8_t parse_rce(const char* rce_s)
{
   char* endptr;
   int rce;

   if(NULL == rce_s)
   {
      return 0x0f;
   }
   
   rce = strtol(rce_s, &endptr, 0);
   if(rce_s == endptr)
   {
      if(0 == strcmp(rce_s, "ALL"))
      {
	 return 0x0f;
      }
      else
      {
	 printf("ERROR - Invalid RCE %s\n", rce_s);
	 return 0x00;
      }
   }
   else if(rce > 3)
   {
	 printf("ERROR - Invalid RCE %s\n", rce_s);
	 return 0x00;
   }
   else
   {
      return 1<<rce;
   }

   return 0x00;
}

static uint8_t parse_bay(const char* bay_s)
{
   char* endptr;
   int bay;

   if(NULL == bay_s)
   {
      return 0xff;
   }
   
   bay = strtol(bay_s, &endptr, 0);
   if(bay_s == endptr)
   {
      if(0 == strcmp(bay_s, "DPM"))
      {
	 return 0x0f;
      }
      else if(0 == strcmp(bay_s, "DTM"))
      {
	 return 0x10;
      }
      else if(0 == strcmp(bay_s, "CMB"))
      {
	 return 0x1f;
      }
      else if(0 == strcmp(bay_s, "RTM"))
      {
	 return 0x20;
      }
      else if(0 == strcmp(bay_s, "CEN"))
      {
	 return 0x40;
      }
      else if(0 == strcmp(bay_s, "ALL"))
      {
	 return 0xff;
      }
      else
      {
	 printf("ERROR - Invalid BAY %s\n", bay_s);
	 return 0x00;
      }
   }
   else if(bay > 6)
   {
      printf("ERROR - Invalid BAY %s\n", bay_s);
      return 0x00;
   }
   else
   {
      return 1<<bay;
   }

   return 0x00;
}

static uint16_t parse_slot(const char* slot_s)
{
   char* endptr;
   int slot;

   if(NULL == slot_s)
   {
      return 0xffff;
   }
   
   slot = strtol(slot_s, &endptr, 0);
   if(slot_s == endptr)
   {
      if(0 == strcmp(slot_s, "ALL"))
      {
	 return 0xffff;
      }
      else
      {
	 printf("ERROR - Invalid SLOT %s\n", slot_s);
	 return 0x0000;
      }
   }

   else if(slot<1 || slot > 16)
   {
      printf("ERROR - Invalid SLOT %s\n", slot_s);
      return 0x0000;
   }
   else
   {
      return 1<<(slot-1);
   }

   return 0x0000;
}

