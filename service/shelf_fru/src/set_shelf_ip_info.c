
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>

#include "shelf_fru/shelf_ip_info.h"

static void report_usage(void);
static int get_args(set_shelf_ip_info_args* args, 
		    int argc, char* const argv[]);

int main (int argc, char* const argv[])
{
   set_shelf_ip_info_args args;
   get_args(&args, argc, argv);
   return set_shelf_ip_info(&args);
}

static int get_args(set_shelf_ip_info_args* args, int argc, char* const argv[])
{
   int c;
   unsigned long int lint;
   char* endptr;

   memset(args, 0, sizeof(set_shelf_ip_info_args));
   
   while(1)
   {
      static struct option long_options[] = 
      {
	 {"shelf",               required_argument, 0, 's'},
	 {"vlan",                optional_argument, 0, 'i'},
	 {"untagged",            optional_argument, 0, 'u'},
	 {"tagged",              optional_argument, 0, 't'},
	 {"boundary-violations", optional_argument, 0, 'x'},
	 {"group-base",          optional_argument, 0, 'b'},
	 {"group-end",           optional_argument, 0, 'e'},
	 {"subnet-mask",         optional_argument, 0, 'm'},
	 {"gateway",             optional_argument, 0, 'g'},
	 {"verbose",             optional_argument, 0, 'v'},
	 {"dry-run",                   no_argument, 0, 'd'},
	 {"force",                     no_argument, 0, 'f'},
	 {0,0,0,0}
      };

      int option_index = 0;

      c = getopt_long(argc, argv, "s:i::u::t::x::b::e::g::m::v::df", 
		      long_options, &option_index);

      if(c == -1) break;

      switch(c)
      {
      case 's':
	 strncpy(args->dest, optarg, 19);
	 args->dest_type = 1;
	 break;

      case 'i':
	 if(NULL == optarg)
	 {
	    //printf("Using default for VLAN ID\n");
	    args->vlan = 0;
	    args->vlan_valid = 1;
	 }
	 else
	 {
	    lint = strtol(optarg, &endptr, 0);
	    if(endptr == optarg)
	    {
	       printf("ERROR - Invalid VLAN ID %s\n",optarg);
	       report_usage();
	       exit(1);
	    }
	    else if(lint > 0xFFF)
	    {
	       printf("ERROR - Invalid VLAN ID %s\n",optarg);
	       report_usage();
	       exit(1);
	    }
	    else
	    {
	       args->vlan = lint | 0x80000000;
	       args->vlan_valid = 1;
	    }
	 }
	 break;

      case 'u':
	 if(NULL == optarg)
	 {
	    //printf("Using default for untagged frames\n");
	    args->untagged = 0;
	    args->untagged_valid = 1;
	 }
	 else
	 {
	    lint = strtol(optarg, &endptr, 0);
	    if(endptr == optarg)
	    {
	       printf("ERROR - Invalid Untagged Discard option %s\n",optarg);
	       exit(1);
	    }
	    else if(lint > 1)
	    {
	       printf("ERROR - Untagged Discard must be 0 or 1, not %s\n",optarg);
	       report_usage();
	       exit(1);
	    }
	    else
	    {
	       args->untagged = lint&1;
	       args->untagged_valid = 1;
	    }
	 }
	 break;

      case 't':
	 if(NULL == optarg)
	 {
	    //printf("Using default for tagged frames\n");
	    args->tagged = 0;
	    args->tagged_valid = 1;
	 }
	 else
	 {
	    lint = strtol(optarg, &endptr, 0);
	    if(endptr == optarg)
	    {
	       printf("ERROR - Invalid Tagged Discard option %s\n",optarg);
	       exit(1);
	    }
	    else if(lint > 1)
	    {
	       printf("ERROR - Tagged Discard must be 0 or 1, not %s\n",optarg);
	       report_usage();
	       exit(1);
	    }
	    else
	    {
	       args->tagged = lint&1;
	       args->tagged_valid = 1;
	    }
	 }
	 break;

      case 'x':
	 if(NULL == optarg)
	 {
	    //printf("Using default for Boundary Violating frames\n");
	    args->violation = 0;
	    args->violation_valid = 1;
	 }
	 else
	 {
	    lint = strtol(optarg, &endptr, 0);
	    if(endptr == optarg)
	    {
	       printf("ERROR - Invalid Boundary Violation Discard option %s\n",optarg);
	       exit(1);
	    }
	    else if(lint > 1)
	    {
	       printf("ERROR - Boundary Violation Discard must be 0 or 1, not %s\n",optarg);
	       report_usage();
	       exit(1);
	    }
	    else
	    {
	       args->violation = lint&1;
	       args->violation_valid |= 1;
	    }
	 }
	 break;

      case 'b':
	 if(NULL == optarg)
	 {
	    //printf("Using default for Group Base IP Address\n");
	    args->group_base = 0;
	 }
	 else if(!inet_pton(AF_INET, optarg, &args->group_base))
	 {
	    printf("ERROR - Invalid Group Base IP %s\n", optarg);
	    report_usage();
	    exit(1);
	 }
	 args->group_base_valid = 1;
	 break;

      case 'e':
	 if(NULL == optarg)
	 {
	    //printf("Using default for Group Base IP Address\n");
	    args->group_end = 0;
	 }	    
	 else if(!inet_pton(AF_INET, optarg, &args->group_end))
	 {
	    printf("ERROR - Invalid Group End IP %s\n", optarg);
	    report_usage();
	    exit(1);
	 }
	 args->group_end_valid = 1;
	 break;

      case 'g':
	 if(NULL == optarg)
	 {
	    //printf("Using default for Gateway IP Address\n");
	    args->gateway = 0;
	 }	    
	 else if(!inet_pton(AF_INET, optarg, &args->gateway))
	 {
	    printf("ERROR - Invalid Gateway IP %s\n", optarg);
	    report_usage();
	    exit(1);
	 }
	 args->gateway_valid = 1;
	 break;

      case 'm':
	 if(NULL == optarg)
	 {
	    //printf("Using default for Subnet Mask\n");
	    args->subnet_mask = 0;
	 }	    
	 else if(!inet_pton(AF_INET, optarg, &args->subnet_mask))
	 {
	    printf("ERROR - Invalid Subnet Mask IP %s\n", optarg);
	    report_usage();
	    exit(1);
	 }
	 args->subnet_mask_valid = 1;
	 break;
	 
      case 'd':
	 args->dry_run = 1;
	 break;

      case 'f':
	 args->force = 1;
	 break;

      case 'v':
	 if(NULL == optarg)
	 {
	    set_verbose(1);
	 }
	 else
	 {
	    lint = strtol(optarg, &endptr, 0);
	    if(endptr == optarg)
	    {
	       printf("ERROR - Invalid verbosity %s\n", optarg);
	       exit(1);
	    }
	    else
	    {
	       set_verbose(lint);
	    }
	 }
	 break;
	 
      case '?':
	 report_usage();
	 exit(1);
	 break;

      default:
	 printf("Unknown option\n");
	 report_usage();
	 exit(1);
      }
   }

   if(0 == args->dest_type)
   {
      printf("ERROR - Must specify a Shelf IP address\n");
      report_usage();
      exit(1);
   }

   if(optind < argc)
   {
      printf("ERROR - unknown arguments supplied: ");
      while(optind < argc)
      {
	 printf("%s ",argv[optind++]);
      }
      printf("\n");
      report_usage();
      exit(1);
   }
	       
   return 0;
}

static void report_usage(void)
{
   printf("\n"
	  "Usage is: set_shelf_ip_info --shelf <shelf_ip> [OPTIONS]\n"
	  "\n"
	  "Required arguments:\n"
	  " -s,--shelf=SHELF_IP                 IP address of shelf with FRU info to set\n"
	  "\n"
	  "Optional arguments:\n"
	  " When specified w/o parameters, the optional arguments will assume \n"
	  " their default values. When omitted, the values present in the Shelf\n"
	  " FRU Information will be retained.\n"
	  "\n"
	  " -i, --vlan=VLAN_ID                  VLAN ID (12-bits)\n"
	  " -u, --untagged=DISCARD              When DISCARD is 1, incoming\n"
	  "                                     untagged frames will be discarded.\n"
	  " -t, --tagged=DISCARD                When DISCARD is 1, incoming\n"
	  "                                     tagged frames will be discarded.\n"
	  " -x, --boundary-violations=DISCARD   When DISCARD is 1, incoming\n"
	  "                                     boundary violations are discarded.\n"
	  " -b, --group-base=BASE_IP            BASE_IP is the base address for\n"
	  "                                     the block of addresses available\n"
	  "                                     to the shelf. Host ID of all ones\n"
	  "                                     or all zeros are not permitted.\n"
	  " -e, --group-end=END_IP              END_IP is the end address for\n"
	  "                                     the block of addresses available\n"
	  "                                     to the shelf. Host ID of all ones\n"
	  "                                     or all zeros are not permitted.\n"
	  " -m, --subnet-mask=SUBNET_MASK       SUBNET_MASK is the subnet mask\n"
	  "                                     for the shelf. It defines the\n"
	  "                                     network assigned to the shelf.\n"
	  " -g, --gateway=GATEWAY_IP            GATEWAY_IP is the IP address\n"
	  "                                     of the  gateway\n"
	  "\n"
	  " -d, --dry-run                       When this option is used, the\n"
	  "                                     nothing is written back to the shelf.\n"
	  " -f, --force                         When this option is used, write to\n"
	  "                                     the shelf even if there are no changes.\n"
	  "                                     When there is no record present in the\n"
	  "                                     shelf, this can be used w/o any other\n"
	  "                                     configuration options to create a \n"
	  "                                     default record\n"
	  "\n"
	  " -v, --verbose=VERBOSITY             VEBOSITY can range from 1 to 3.\n"
	  "\n"
	  "Return Value:\n"
	  " 0 if OK\n"
	  " 1 if there is a problem with the arguments provided\n"
	  " 2 if there are problems communicating with the shelf\n"
	  "\n"
	  );
   

   return;
}

