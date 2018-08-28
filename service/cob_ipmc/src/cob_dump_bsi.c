#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "cob_ipmc/impl/COB_cmds.h"
#include "cob_ipmc/impl/COB_dump_bsi.h"
#include "cob_ipmc/impl/COB_util.h"

typedef enum
{
   DUMP_RAW,
   DUMP_RCE,
   DUMP_CLUSTER,
   DUMP_SWITCH,
   DUMP_SHELF_IP,
   DUMP_FABRIC_MAP,
   DUMP_ALL
} COB_dump_idx;

static struct option long_options[] = 
{
   {"raw",     no_argument, NULL, DUMP_RAW},
   {"rce",     no_argument, NULL, DUMP_RCE},
   {"cluster", no_argument, NULL, DUMP_CLUSTER},
   {"switch",  no_argument, NULL, DUMP_SWITCH},
   {"shelf",   no_argument, NULL, DUMP_SHELF_IP},
   {"fabric",  no_argument, NULL, DUMP_FABRIC_MAP},
   {"all",     no_argument, NULL, DUMP_ALL},
   {"verbose", no_argument, NULL, 'v'},
   {0,0,0,0}
};

static COB_dump_bsi_cb dump_opt[] = 
{
   &COB_dump_bsi_raw,
   &COB_dump_bsi_rce,
   &COB_dump_bsi_cluster,
   &COB_dump_bsi_switch_config,
   &COB_dump_bsi_shelf_ip_info,
   &COB_dump_bsi_fabric_map,
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
	 dump_options |= (1<<DUMP_ALL)-2;
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
	  "Usage is: cob_dump_bsi [OPTIONS] <rce_address> [<rce_address> ...]\n"
	  "\n"
	  " Produces (sort-of-)human readable representations of the \n"
	  " RCE BootStrap Interface. Not all BSI areas are available \n"
	  " on DPMs, but requesting those produces no output.\n "
	  "\n"
	  " Where the OPTIONS are:\n"
	  "     --raw     Dump of raw BSI contents\n"
	  "     --rce     Dump RCE information\n"
	  "     --cluster Dump Cluster information\n"
	  "     --switch  Dump COB switch configuration information\n"
	  "     --shelf   Dump shelf IP information\n"
	  "     --fabric  Dump shelf fabric map\n"
	  "     --all     Dump all of the above information except raw\n"
	  "               (can be issued with --raw to get raw and all)\n"
	  "\n");
   
   COB_util_mask_usage();
   
   printf("Return Value:\n"
	  " 0 if OK\n"
	  " 1 Error\n"
	  "\n");
   

   return;
}

