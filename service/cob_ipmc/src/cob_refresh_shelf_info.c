#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "cob_ipmc/impl/COB_cmds.h"
#include "cob_ipmc/impl/COB_util.h"

static void report_usage(void);

int main (int argc, char* const argv[])
{
   COB_ctx* ctx;
   int ret = 0;

   uint8_t reset_rs_buf[COB_RCE_RESET_RS_SIZE];
   COB_rce_reset_rs* reset_rs;
   
   uint8_t refresh_rs_buf[COB_SHELF_INFO_REFRESH_RS_SIZE];
   COB_shelf_info_refresh_rs* refresh_rs;

   uint8_t ipmb_addr;

   int i;

   char shelf[32];
   int slot, s;
   
   uint16_t slot_mask;
   uint8_t bay_mask;
   uint8_t rce_mask;

   int verbose = 0;
   int reset_rces = 1;

   static struct option long_options[] =
   {
      {"no-reset", no_argument, 0, 'n'},
      {"verbose",  no_argument, 0, 'v'},
      {0,0,0,0}
   };
   
   int option_index = 0;
   int c;

   while(1)
   {
      c = getopt_long(argc, argv, "nv", long_options, &option_index);

      if(-1 == c) break;

      switch(c)
      {
      case 'v':
	 verbose = 1;
	 break;
      case 'n':
	 reset_rces = 0;
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

	 if(verbose)
	 {
	    printf("%s/%i: Refreshing Shelf Info in IPMC\n", shelf, slot);
	 }	    
	 
	 refresh_rs = COB_shelf_info_refresh(ctx, ipmb_addr, refresh_rs_buf);

	 if((NULL == refresh_rs) || refresh_rs->ccode)
	 {
	    printf("ERROR - %s/%i: Couldn't refresh Shelf Info in IPMC\n",
		   shelf, slot);
	    ret = 1;
	    continue;
	 }

	 if(!reset_rces) continue;

	 // Override the argument for the bay and rce masks
	 // That way we'll reset them all if the reset flag is set
	 bay_mask = 0xff;
	 rce_mask = 0xff;

	 if(verbose)
	 {
	    printf("%s/%i: Resetting RCEs Bay Mask %02x, RCE Mask %02x\n",
		   shelf, slot, bay_mask, rce_mask);
	 }
	 
	 reset_rs = COB_rce_reset(ctx, ipmb_addr, 
				  bay_mask, rce_mask, reset_rs_buf);
	 
	 if(NULL == reset_rs)
	 {
	    printf("ERROR - %s/%i: Couldn't connect to IPMC\n", 
		   shelf, slot);
	    ret = 1;
	 }
	 else if(reset_rs->ccode)
	 {
	    printf("ERROR - %s/%i: "
		   "IPMI command returned completion code %02x\n",
		   shelf, slot, reset_rs->ccode);
	    ret = 1;
	 }
	 else if(reset_rs->err_mask)
	 {
	    printf("ERROR - %s/%i: "
		   "COB IPMC failed to execute reset commands in "
		   "bays in mask %02x\n",
		   shelf, slot, reset_rs->err_mask);
	    ret = 1;
	 }
      }
      COB_close(ctx);
   }
   return ret;
}

static void report_usage(void)
{
   printf("\n"
	  "Usage is: cob_refresh_shelf_info [OPTIONS] <rce address> [<rce_address> ...]\n"
	  "\n"
	  " Refreshes the Shelf FRU information cached in the IPMC. \n"
	  " By default, this command will reset all the RCEs in\n"
	  " the COB after the the refresh. \n"
	  "\n"
	  " The most common use of this command is to reset the Shelf IP\n"
	  " Information in the IPMCs after updating it in the Shelf FRU\n"
	  " EEPROM through the Shelf Manager.\n"
	  "\n"
   	  " Where the [OPTIONS] can be:\n"
	  "    --no-reset, -n   Don't reset the RCEs after refreshing the IPMC\n"
	  "    --verbose,  -v   Print out additional output\n"
	  "\n");

   COB_util_mask_usage();

   printf("  For this command, the bay and rce portions of <rce_address> are ignored\n\n");

   printf("Return Value:\n"
	  " 0 if OK\n"
	  " 1 Error\n"
	  "\n");
   return;
}

