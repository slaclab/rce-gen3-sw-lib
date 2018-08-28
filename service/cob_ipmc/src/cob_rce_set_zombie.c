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

   uint8_t rs_buf[COB_ZOMBIE_SET_RS_SIZE];
   COB_zombie_set_rs* rs;

   uint8_t ipmb_addr;

   int i;
   
   char shelf[32];
   int slot, s;

   uint16_t slot_mask;
   uint8_t bay_mask;
   uint8_t rce_mask;

   uint8_t zombie = 1;
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
      ctx = COB_open(shelf);
      if(NULL == ctx) 
      {
	 printf("ERROR - Couldn't connect to shelf manager %s\n", shelf);
	 continue;
      }
      
      COB_util_get_masks(argv[i], shelf, 32, 
			 &slot_mask, &bay_mask, &rce_mask, verbose);
      
      for(slot=ATCA_MIN_SLOT; slot<ATCA_MAX_SLOT; ++slot)
      {
	 s = slot-1;
	 if(!(slot_mask & (1<<s))) continue;
	 
	 ipmb_addr = COB_ipmb_addr(ctx, slot);
	 if(0xff == ipmb_addr) continue;
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
	    printf("%s/%i: Clearing Zombie State in "
		   "Bay Mask %02x, RCE Mask %02x\n",
		   shelf, slot, bay_mask, rce_mask);
	 }
	 
	 rs = COB_zombie_set(ctx, ipmb_addr, bay_mask, rce_mask, 
			     zombie, rs_buf);
	 
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
	    printf("ERROR - IPMI command returned completion code %02x "
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
	  "Usage is: cob_rce_set_zombie [-v] <rce address> [<rce_address> ...]\n"
	  "\n"
	  " Set Zombie state for RCE(s)"
	  "\n"
	  " When in the Zombie state, the IPMC ignores the RCE. Which is to\n"
	  " say, it doesn't respond to changes in the Done state of the FPGS\n"
	  " or the absence of the BSI. \n"
	  "\n"
	  " NOTE: The COB will not light the green front panel LED if any \n"
	  "       RCEs are in the Zombie state\n"
	  "\n");

   COB_util_mask_usage();

   printf("Return Value:\n"
	  " 0 if OK\n"
	  " 1 Error\n"
	  "\n");
   

   return;
}

