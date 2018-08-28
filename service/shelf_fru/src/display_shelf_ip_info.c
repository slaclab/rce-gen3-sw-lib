
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>

#include "shelf_fru/shelf_ip_info.h"

static void report_usage(void);
static int get_args(display_shelf_ip_info_args* args, 
		    int argc, char* const argv[]);

int main (int argc, char* const argv[])
{
   display_shelf_ip_info_args args;
   get_args(&args, argc, argv);
   return display_shelf_ip_info(&args);
}

static int get_args(display_shelf_ip_info_args* args, 
		    int argc, char* const argv[])
{
   int c;
   unsigned long int lint;
   char* endptr;
   int shelf = 0;

   memset(args, 0, sizeof(display_shelf_ip_info_args));
   
   while(1)
   {
      static struct option long_options[] = 
      {
	 {"shelf",               required_argument, 0, 's'},
	 {"mr-area",                   no_argument, 0, 'm'},
	 {"raw",                       no_argument, 0, 'r'},
	 {"verbose",             optional_argument, 0, 'v'},
	 {0,0,0,0}
      };

      int option_index = 0;

      c = getopt_long(argc, argv, "s:mrv::", 
		      long_options, &option_index);

      if(c == -1) break;

      switch(c)
      {
      case 's':
	 strncpy(args->shelf, optarg, 19);
	 shelf =  1;
	 break;

      case 'm':
	 args->mr_area = 1;
	 break;
	 
      case 'r':
	 args->raw = 1;
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

   if(!shelf)
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
	  "Usage is: delete_shelf_ip_info --shelf <shelf_ip> [OPTIONS]\n"
	  "\n"
	  "Required arguments:\n"
	  " -s,--shelf=SHELF_IP                 IP address of shelf with FRU info to set\n"
	  "\n"
	  "Optional arguments:\n"
	  " When specified w/o parameters, the optional arguments will assume \n"
	  " their default values. When omitted, the values present in the Shelf\n"
	  " FRU Information will be retained.\n"
	  "\n"
	  " -r, --raw                           Prints out raw Shelf IP Info\n"
	  " -m, --mr-area                       Prints out raw MultiRecord Area\n"
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

