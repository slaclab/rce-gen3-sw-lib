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

   uint8_t wr_rs_buf[COB_FRU_INFO_WRITE_RS_SIZE];
   COB_fru_info_write_rs* wr_rs;
   
   uint8_t id_rs_buf[COB_FRU_INFO_ID_FIXUP_RS_SIZE];
   COB_fru_info_id_fixup_rs* id_rs;

   uint8_t tag_rs_buf[COB_FRU_INFO_ASSET_TAG_SET_RS_SIZE];
   COB_fru_info_asset_tag_set_rs* tag_rs;

   uint8_t ipmb_addr;

   int i;

   char shelf[32];
   int slot, s;
   
   uint16_t slot_mask;
   uint8_t bay_mask;
   uint8_t rce_mask;

   char tag[32];
   char filename[128]; 
   FILE* file = NULL;

   uint8_t data[16];
   
   int offset;
   int len;

   memset(tag, 0, sizeof(tag));
   memset(filename, 0, sizeof(filename));

   int verbose = 0;
   int serial = 0;

   static struct option long_options[] =
   {
      {"file",    required_argument, 0, 'f'},
      {"tag",     required_argument, 0, 't'},
      {"serial",  no_argument, 0, 's'},
      {"verbose", no_argument, 0, 'v'},
      {0,0,0,0}
   };
   
   int option_index = 0;
   int c;
   int anyopt = 0;

   while(1)
   {
      c = getopt_long(argc, argv, "f:t:sv", long_options, &option_index);

      if(-1 == c) break;

      switch(c)
      {
      case 'v':
	 verbose = 1;
	 break;
      case 's':
	 serial = 1;
	 ++anyopt;
	 break;
      case 'f':
	 strncpy(filename, optarg, sizeof(filename));
	 ++anyopt;
	 break;
      case 't':
	 strncpy(tag, optarg, sizeof(tag));
	 ++anyopt;
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

   if(0 == anyopt)
   {
      printf("ERROR - Must specify at least one OPTION\n");
      report_usage();
      return 1;
   }

   if(filename[0])
   {
      file = fopen(filename, "r");
      if(NULL == file)
      {
	 printf("ERROR - Can't open file %s\n", filename);
	 return 1;
      }
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
	 
	 if(file)
	 {
	    rewind(file);

	    if(verbose)
	    {
	       printf("%s/%i: Writing FRU Information\n", shelf, slot);
	    }
	    
	    offset = 0;
	    while((len = fread(data, sizeof(uint8_t), sizeof(data), file)))
	    {
	      wr_rs = COB_fru_info_write(ctx, ipmb_addr, COB_FRU_ID,
					 offset, len, data, wr_rs_buf);

	       if(wr_rs == NULL || wr_rs->ccode)
	       {
		  printf("ERROR - %s/%i: Failed to write to FRU Info "
			 "offset 0x%x, length 0x%x\n",
			 shelf, slot, offset, len);
		  printf("ERROR - COB FRU Information may be "
			 "in an inconsistent state!\n");
		  ret = 1;
		  break;
	       }
	       offset += len;
	    }    

	    /*
	      We need to wait for the EEPROM commit to finish before
	      we can do anything else with it.
	    */
	    sleep(1);
	 }

	 if(serial)
	 {
	    if(verbose) 
	    {
	       printf("%s/%i: Filling FRU Info serial number from ID PROM\n",
		      shelf, slot);
	    }
	    
	    id_rs = COB_fru_info_id_fixup(ctx, ipmb_addr, 
					  COB_FRU_ID, id_rs_buf);
	    
	    if(id_rs == NULL || id_rs->ccode)
	    {
	       printf("ERROR - %s/%i: Failed to fill COB FRU Info "
		      "serial number from ID PROM\n", shelf, slot);
	       ret = 1;
	    }
	 }
	 
	 if(tag[0])
	 {
	    if(verbose)
	    {
	       printf("%s/%i: Writing Asset Tag %s to FRU Information\n",
		      shelf, slot, tag);
	    }

	    tag_rs = COB_fru_info_asset_tag_set(ctx, ipmb_addr, COB_FRU_ID,
						tag, tag_rs_buf);

	    if(tag_rs == NULL || tag_rs->ccode)
	    {
	       printf("ERROR - %s/%i: Failed to fill COB FRU Info Asset Tag\n",
		      shelf, slot);
	       ret = 1;
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
	  "Usage is: cob_fru_init [OPTIONS] <rce_address> [<rce_address> ...]\n"
	  "\n"
	  " Initialize the FRU information on an COB.\n"
	  "\n"
	  " Where the OPTIONS are:\n"
	  "\n"
	  "   -f, --file=FILE The binary file containing the FRU information\n"
	  "                   In order for the ID PROM and Asset Tags to be\n"
	  "                   filled both fields should be initialized with\n"
	  "                   8 and 32 bytes of zeros respectively.\n"
	  "\n"
	  "   -t, --tag=TAG   The Asset Tag is a NULL terminated ASCII string\n"
	  "                   of up to 24 bytes. It is intended to be a human\n"
	  "                   readable unique identifier which can be used \n"
	  "                   for inventory control. If there is a label on\n"
	  "                   the board, the asset tag should match that label\n"
	  "\n"
	  "   -s, --serial    When this flag is used, the serial number is\n"
	  "                   read from the ID PROM and written into the \n"
	  "                   FRU Information.\n"
	  "\n"
	  "   -v, --verbose   Additional output\n"
	  "\n"
	  "  When the --serial option is specified, the contents of the ID \n"
	  "  prom will be written into the Serial Number fields in the FRU \n"
	  "  information. If the --tag option is specified, the Asset Tag \n"
	  "  field of the FRU Information will be filled. If the --file\n"
	  "  option is used without the --tag option, the tag will be NULL\n"
	  "  If the --tag or --serial options are used without the --file \n"
	  "  option, the contents of the FRU information in the RTM EEPROM \n"
	  "  must be the correct length already.\n"
	  "\n"
	  "  Obviously, one probably shouldn't use the --tag option when\n"
	  "  addressing multiple slots. (Unless those slots share an asset \n"
	  "  tag, in which case shame on you!)\n"
	  "\n");
   
   COB_util_mask_usage();

   printf("  For this command, the bay and rce portions of <rce_address> are ignored\n\n");

   printf("Return Value:\n"
	  " 0 if OK\n"
	  " 1 Error\n"
	  "\n");
   return;
}

