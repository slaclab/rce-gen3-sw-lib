#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "cob_ipmc/impl/COB_cmds.h"
#include "cob_ipmc/impl/COB_dump.h"
#include "cob_ipmc/impl/COB_util.h"

typedef enum
{
   DUMP_IPMC,
   DUMP_BAY,
   DUMP_RCE,
   //   DUMP_SFP,
   DUMP_GPIO,
   DUMP_RTM,
   DUMP_CEN,
   DUMP_ALL,
} COB_dump_idx;

static struct option long_options[] = 
{
   {"ipmc",    no_argument, NULL, DUMP_IPMC},
   {"bay",     no_argument, NULL, DUMP_BAY},
   {"rce",     no_argument, NULL, DUMP_RCE},
   //   {"sfp",     no_argument, NULL, DUMP_SFP},
   {"gpio",    no_argument, NULL, DUMP_GPIO},
   {"rtm",     no_argument, NULL, DUMP_RTM},
   {"cen",     no_argument, NULL, DUMP_CEN},
   {"all",     no_argument, NULL, DUMP_ALL},
   {"verbose", no_argument, NULL, 'v'},
   {0,0,0,0}
};

static COB_dump_cb dump_opt[] = 
{
   &COB_dump_ipmc,
   &COB_dump_bay,
   &COB_dump_rce,
   //   &COB_dump_sfp,
   &COB_dump_bay_gpio,
   &COB_dump_rtm,
   &COB_dump_cen,
};

static void report_usage(void);

int main (int argc, char* const argv[])
{

   char shelf[32];
   uint16_t slot_mask;
   uint8_t bay_mask;
   uint8_t rce_mask;

   int i;
   int d;

   int verbose = 0;
   unsigned int dump_options = 0;
   
   int option_index = 0;
   int c;

   while(-1 != (c = getopt_long(argc, argv, "v", long_options, &option_index)))
   {
      if('v' == c)
      {
	 verbose = 1;
      }
      else if(c < DUMP_ALL)
      {
	 dump_options |= 1<<c;
      }
      else if(c == DUMP_ALL)
      {
	 dump_options |= (1<<DUMP_ALL)-1;
      }
      else
      {
	 report_usage();
	 return 1;
      }
   }

   if(!dump_options)
   {
      printf("ERROR - Must specify at least one OPTION\n");
      report_usage();
      return 1;
   }
   
   if((argc-optind)<1)
   {
      printf("ERROR - Must specify at least one <rce_address>\n");
      report_usage();
      return 1;
   }

   for(d=0; d<DUMP_ALL; ++d)
   {
      if(!(dump_options & (1<<d))) continue;

      for(i=optind; i<argc; ++i)
      {
	 COB_util_get_masks(argv[i], shelf, 32,
			    &slot_mask, &bay_mask, &rce_mask, verbose);
	 dump_opt[d](shelf, slot_mask, bay_mask, rce_mask, verbose);
      }
   }	 
   return 0;
}

static void report_usage(void)
{
   printf("\n"
	  "Usage is: cob_dump [OPTIONS] <rce_address> [<rce_address> ...]\n"
	  "\n"
	  " Produces (sort-of-)human readable summaries of the quantities\n"
	  " monitored by the IPMC. \n"
	  "\n"
	  " Where OPTIONS are:\n"
	  "    --ipmc  Dump IPMC version Information\n"
	  "    --bay   Dump Bay Information\n"
	  "    --rce   Dump RCE Information\n"
	  "    --gpio  Dump Bay GPIO Information\n"
	  //	  "    --sfp   Dump front panel SFP information\n"
	  "    --rtm   Dump RTM information\n"
	  "    --cen   Dump CEN information\n"
	  "    --all   Dump ALL of the above information\n"
	  "\n");
   
   COB_util_mask_usage();
   
   printf("Return Value:\n"
	  " 0 if OK\n"
	  " 1 Error\n"
	  "\n");
   

   return;
}

