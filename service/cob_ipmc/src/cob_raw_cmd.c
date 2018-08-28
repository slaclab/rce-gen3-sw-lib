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

   uint8_t ipmb_addr;

   int i;

   char shelf[32];
   int slot, s;
   
   uint16_t slot_mask;
   uint8_t bay_mask;
   uint8_t rce_mask;

   uint8_t rs_buf[64];
   uint8_t data[64];
   int data_len = 0;
   int rs_len = sizeof(rs_buf);

   char* endptr;
   char data_s[256];
   char* datum_s;
   unsigned int datum;
   int len;

   int b;

   memset(rs_buf, 0, sizeof(rs_buf));
   memset(data, 0, sizeof(data));

   int verbose = 0;
   
   unsigned int netfn = 0x100;
   unsigned int cmd   = 0x100;

   static struct option long_options[] =
   {
      {"netfn",   required_argument, 0, 'n'},
      {"cmd",     required_argument, 0, 'c'},
      {"data",    required_argument, 0, 'd'},
      {"verbose", no_argument, 0, 'v'},
      {0,0,0,0}
   };
   
   int option_index = 0;
   int c;

   while(1)
   {
      c = getopt_long(argc, argv, "n:c:d:v", long_options, &option_index);

      if(-1 == c) break;

      switch(c)
      {
      case 'v':
	 verbose = 1;
	 break;
      case 'n':
	 netfn = strtol(optarg, &endptr, 0);
	 if((optarg == endptr) || (netfn > 0xff))
	 {
	    printf("ERROR - Invalid netFn 0x%02x\n", netfn);
	    report_usage();
	    return -1;
	 }
	 break;
      case 'c':
	 cmd = strtol(optarg, &endptr, 0);
	 if((optarg == endptr) || (cmd > 0xff))
	 {
	    printf("ERROR - Invalid command 0x%02x\n", cmd);
	    report_usage();
	    return -1;
	 }
	 break;
      case 'd':
	 strncpy(data_s, optarg, sizeof(data_s));
	 datum_s = strtok(data_s, " ");
	 i = 0;
	 if(datum_s)
	 {
	    do
	    {
	       datum   = strtol(datum_s, &endptr, 0);
	       if((endptr == datum_s) || (datum > 0xff))
	       {
		  printf("ERROR - Invalid data %s\n", datum_s);
		  return -1;
	       }
	       data[i] = datum;
	       ++i;
	    } while ((datum_s = strtok(NULL, " ")));
	 }
	 data_len = i;
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

   if((netfn > 0xFF) || (cmd > 0xFF))
   {   
      printf("ERROR - Must specify netFn and command\n");
      report_usage();
      return 1;
   }

   if(verbose)
   {
      printf("Command: netFn 0x%02x, cmd 0x%02x", netfn, cmd);
      
      if(data_len)
      {
	 printf(", data ");
	 for(i=0; i<data_len; ++i)
	 {
	    printf("0x%02x ",data[i]);
	 }
      }
      printf("\n");
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

	len = COB_raw_cmd(ctx, ipmb_addr, netfn, cmd, 
			   data, data_len,
			   rs_buf, rs_len);

	 if(-1 == len)
	 {
	    printf("ERROR - Unable to send raw command\n");
	    continue;
	 }
	 
	 if(len)
	 {
	    printf("%s/%i: Response ", shelf, slot);
	    
	    for(b=0; b<len; ++b)
	    {
	       printf(" %02x", rs_buf[b]);
	    }
	    printf("\n");
	 }
      }
      
      COB_close(ctx);
   }
   return ret;
}

static void report_usage(void)
{
   printf("\n"
	  "Usage is: cob_raw_cmd [OPTIONS] <rce_address> [<rce_address> ...]\n"
	  "\n"
	  " Send a raw IPMI command to one or more COBs.\n"
	  "\n"
	  " Where the OPTIONS are:\n"
	  "\n"
	  "   -n, --netfn=NETFN The netFn for the command\n"
	  "\n"
	  "   -c, --cmd=CMD     The command index\n"
	  "\n"
	  "   -d, --data=DATA    The data to send as a list of bytes. \n" 
	  "                      If more than one byte is sent, the bytes \n"
	  "                      should be enclosed in quotes\n"
	  "\n"
	  "   -v, --verbose   Additional output\n"
	  "\n"
	  " This command can be used to send a raw IPMI command to one or \n"
	  " more COBs, any data returned will be printed to the stdout \n"
	  " Specifying more than one <rce_address> or a composite \n"
	  " <rce_address> will result in multiple lines of output\n"
	  "\n");

   COB_util_mask_usage();
   
   printf("Return Value:\n"
	  " 0 if OK\n"
	  " 1 Error\n"
	  "\n");
   return;
}

