#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

#include "cob_ipmc/impl/COB_cmds.h"
#include "cob_ipmc/impl/COB_util.h"

static void report_usage(void);

int main (int argc, char* const argv[])
{
   COB_ctx* ctx;
   int ret = 0;

   uint8_t wr_rs_buf[COB_RTM_EEPROM_WRITE_RS_SIZE];
   COB_rtm_eeprom_write_rs* wr_rs;
   
   uint8_t ipmb_addr;

   int i;
   int offset;
   int len;


   char* endptr;
   double watts=0.0;
   int watts_valid;
   uint8_t power;

   uint32_t type;
   int type_valid;

   char shelf[32];
   int slot, s;
   
   uint16_t slot_mask;
   uint8_t bay_mask;
   uint8_t rce_mask;

   int verbose = 0;

   static struct option long_options[] =
   {
      {"power",    required_argument, 0, 'p'},
      {"type",     required_argument, 0, 't'},
      {"verbose", no_argument, 0, 'v'},
      {0,0,0,0}
   };
   
   int option_index = 0;
   int c;

   while(1)
   {
      c = getopt_long(argc, argv, "p:t:v", long_options, &option_index);

      if(-1 == c) break;

      switch(c)
      {
      case 'v':
	 verbose = 1;
	 break;
      case 'p':
	 watts = strtod(optarg, &endptr);
	 if((optarg == endptr) || (watts < 0) || (watts > RMB_MAX_WATTS))
	 {
	    printf("ERROR - Maximum power is %i\n", RMB_MAX_WATTS);
	    report_usage();
	    return 1;
	 }
	 watts_valid = 1;
	 break;
      case 't':
	 type = strtol(optarg, &endptr, 0);
	 if((optarg == endptr) || COB_invalid_rmb_type(type))
	 {
	    printf("ERROR - Invalid RMB type\n");
	    report_usage();
	    return 1;
	 }
	 type_valid = 1;
	 break;
      case '?':
	 report_usage();
	 return 1;
	 break;
      default:
	 report_usage();
	 return 1;
	 break;
      }
   }

   if((argc-optind)<1)
   {
      report_usage();
      return 1;
   }

   for(i=optind; i<argc; ++i)
   {

      COB_util_get_masks(argv[i], shelf, 32,
			 &slot_mask, &bay_mask, &rce_mask, verbose);

      ctx = COB_open(shelf);
      if(NULL == ctx)
      {
	 printf("ERROR - Couldn't connect to shelf manager %s\n", shelf);
	 continue;
      }

      for(slot=ATCA_MIN_SLOT; slot<ATCA_MAX_SLOT; ++slot)
      {
	 s = slot-1;
	 if(!(slot_mask & (1<<s))) continue;

	 ipmb_addr = COB_ipmb_addr(ctx, slot);
	 if(0xff == ipmb_addr)
	 {
	    if(0xffff != slot_mask)
	    {
	       printf("ERROR - Slot %i not present in shelf\n", slot);
	    }
	    continue;
	 }
	 
	 if(!COB_slot_present(ctx, ipmb_addr)) continue;

	 if(watts_valid)
	 {
	    power = watts*5;
	    offset = 0;
	    len = 1;

	    if(verbose) 
	    {
	       printf("%s/%i: Writing Power 0x%02x (%2.1fW) to RMB EEPROM\n", 
		      shelf, slot, power, power/5.);
	    }

	    wr_rs = COB_rtm_eeprom_write(ctx, ipmb_addr, 1,
					 offset, len, &power, wr_rs_buf);

	    if(wr_rs == NULL || wr_rs->ccode)
	    {
	       printf("ERROR - %s/%i: Failed to write Power to RMB EEPROM\n",
		      shelf, slot);
	       ret = 1;
	       break;
	    }

	    /*
	      We need to wait for the EEPROM commit to finish before
	      we can do anything else with it.
	    */
	    sleep(1);
	 }    
	 
	 if(type_valid)
	 {
	    offset = 1;
	    len    = 4;

	    if(verbose)
	    {
	       printf("%s/%i: Writing Type 0x%x to RMB EEPROM\n", 
		      shelf, slot, type);
	    }

	    wr_rs = COB_rtm_eeprom_write(ctx, ipmb_addr, 1,
					 offset, len, (uint8_t*)&type, 
					 wr_rs_buf);
	    
	    if(wr_rs == NULL || wr_rs->ccode)
	    {
	       printf("ERROR - %s/%i: Failed to write Type to RMB EEPROM\n",
		      shelf, slot);
	       ret = 1;
	       break;
	    }
	 }	 
      }
      
      COB_close(ctx);
   }
   return ret;
}

static void report_usage(void)
{
   printf("\n"
	  "Usage is: cob_rmb_init [OPTIONS] <rce_address> [<rce_address> ...]\n"
	  "\n"
	  " Initialize the Power and Type information in the RMB EEPROM\n."
	  "\n"
	  " Where the OPTIONS are:\n"
	  "\n"
	  "   -p, --power=WATTS The power used by the RMB in Watts\n"
	  "\n"
	  "   -t, --type=TYPE   The RMB type\n"
	  "\n"
	  "   -v, --verbose     Additional output\n"
	  "\n");

   COB_util_mask_usage();
   
   printf("  For this command, the bay and rce portions of <rce_address> are ignored\n\n");

   printf("Return Value:\n"
	  " 0 if OK\n"
	  " 1 Error\n"
	  "\n");
   return;
}

