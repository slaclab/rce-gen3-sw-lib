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

   uint8_t rs_buf[COB_COLD_DATA_RESET_RS_SIZE];
   COB_cold_data_reset_rs* rs;

   uint8_t ipmb_addr;

   int i;

   char shelf[32];
   int slot;
   
   uint16_t slot_mask;
   uint8_t bay_mask;
   uint8_t rce_mask;

   int verbose = 0;

   static struct option long_options[] =
   {
      {"verbose", no_argument, 0, 'v'},
      {0,0,0,0}
   };
   
   int option_index = 0;
   int c;

   while(1)
   {
      c = getopt_long(argc, argv, "v", long_options, &option_index);

      if(-1 == c) break;

      switch(c)
      {
      case 'v':
	 verbose = 1;
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
	 if(!(slot_mask & (1<<(slot-1)))) continue;

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
	    printf("%s/%i: Sending Cold Data Reset\n", shelf, slot);
	 }
	 
	 rs = COB_cold_data_reset(ctx, ipmb_addr, rs_buf);
	 
	 if(NULL == rs)
	 {
	    if((0xffff != slot_mask) || verbose)
	    {
	       printf("ERROR - Couldn't connect to IPMC %s/%i\n", 
		      shelf, slot);
	    }
	    ret = 1;
	 }
	 else if(rs->ccode)
	 {
	    printf("ERROR - IPMI command returned completion code %02x"
		   "for %s Slot %i\n", rs->ccode, shelf, slot);
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
	  "Usage is: cob_cold_data_reset <rce address> [<rce_address> ...]\n"
	  "\n"
	  " Send a Cold Data Reset command to a COB.\n"
	  "\n"
	  " This command causes the same effect as physically removing the \n"
	  " COB from the slot except that Management Power is not removed.\n"
	  " Upon receiving this command, the IPMC will remove payload power\n"
	  " from all bays, clear all internal cached state informaion which\n"
	  " includes all Shelf IP Information and the Shelf Name, and then\n"
	  " resets the IPMC itself. If the conditions for activation are\n"
	  " satisfied (handle switch closed) the COB will proceed normally\n"
	  " exactly as if it had just been inserted in the shelf.\n"
	  "\n"
	  " If you are just looking to refresh the cached Shelf information\n"
	  " in the IPMC, consider using the cob_refresh_shelf_info command.\n"
	  "\n");
   
   COB_util_mask_usage();

   printf("  For this command, the bay and rce portions of <rce_address> are ignored\n\n");

   printf("Return Value:\n"
	  " 0 if OK\n"
	  " 1 Error\n"
	  "\n");
   return;
}

