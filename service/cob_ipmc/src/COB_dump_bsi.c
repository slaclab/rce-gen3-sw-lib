#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cob_ipmc/impl/COB_cmds.h"

#include "cob_ipmc/impl/COB_util.h"
#include "cob_ipmc/impl/COB_dump_bsi.h"

#include "cob_ipmc/impl/BSI_structs.h"

static const char dashes[]  = ("----------------------------------------"
			       "----------------------------------------");
static const char ddashes[] = ("========================================"
			       "========================================");

static const char NY[] = "NY";

typedef void (*bsi_dump_fn)(const COB_ctx* ctx, 
			    int ipmb_addr, 
			    int bay, 
			    int rce, 
			    const BSI_version_info* info);


static const BSI_version_info* get_bsi_version_info(const COB_ctx* ctx,
						    int ipmb_addr,
						    int bay,
						    int rce);



static int cob_dump_bsi(const char* shelf, 
			uint16_t  slot_mask,
			uint8_t  bay_mask,
			uint8_t  rce_mask,
		        int  verbose,
			bsi_dump_fn fn);

static void bsi_dump_raw(const COB_ctx* ctx, int ipmb_addr, int bay, int rce, const BSI_version_info* info);

static int bsi_read(const COB_ctx* ctx, int ipmb_addr, int bay, int rce,
		    unsigned int offset, unsigned int length, 
		    void* dest_buffer);

static void bsi_dump_rce(const COB_ctx* ctx, int ipmb_addr, int bay, int rce, const BSI_version_info* info);

static void bsi_dump_cluster_data(const COB_ctx* ctx, int ipmb_addr, int bay, int rce, const BSI_version_info* info);

static void bsi_dump_switch_config(const COB_ctx* ctx, int ipmb_addr, int bay, int rce, const BSI_version_info* info);

static void bsi_dump_shelf_ip_info(const COB_ctx* ctx, int ipmb_addr, int bay, int rce, const BSI_version_info* info);

static void bsi_dump_fabric_map(const COB_ctx* ctx, int ipmb_addr, int bay, int rce, const BSI_version_info* info);

/*
static void bsi_dump_status(const COB_ctx* ctx, int ipmb_addr, int bay, int rce, const BSI_version_info* info);
*/

static const char* lane_type_name(int type);
static const char* port_type_name(int type);



int COB_dump_bsi_raw(const char* shelf, 
		       uint16_t  slot_mask,
		        uint8_t  bay_mask,
		        uint8_t  rce_mask,
		            int  verbose)
{
   cob_dump_bsi(shelf, slot_mask, bay_mask, rce_mask, verbose, bsi_dump_raw);
   return 0;
}

int COB_dump_bsi_rce(const char* shelf, 
		       uint16_t  slot_mask,
		        uint8_t  bay_mask,
		        uint8_t  rce_mask,
		            int  verbose)
{
   cob_dump_bsi(shelf, slot_mask, bay_mask, rce_mask, verbose, bsi_dump_rce);
   return 0;
}

int COB_dump_bsi_cluster(const char* shelf, 
		           uint16_t  slot_mask,
		            uint8_t  bay_mask,
		            uint8_t  rce_mask,
		                int  verbose)
{
   cob_dump_bsi(shelf, slot_mask, bay_mask, rce_mask, verbose, 
		bsi_dump_cluster_data); 
   return 0;
}

int COB_dump_bsi_switch_config(const char* shelf, 
		                 uint16_t  slot_mask,
		                  uint8_t  bay_mask,
		                  uint8_t  rce_mask,
		                      int  verbose)
{
   // Switch config is DTM only
   bay_mask &= 1<<DTM;
   cob_dump_bsi(shelf, slot_mask, bay_mask, rce_mask, verbose, 
		bsi_dump_switch_config); 
   return 0;
}

int COB_dump_bsi_shelf_ip_info(const char* shelf, 
		                 uint16_t  slot_mask,
		                  uint8_t  bay_mask,
		                  uint8_t  rce_mask,
		                      int  verbose)
{
   // Shelf IP Info is DTM only
   bay_mask &= 1<<DTM;
   cob_dump_bsi(shelf, slot_mask, bay_mask, rce_mask, verbose, 
		bsi_dump_shelf_ip_info); 
   return 0;
}

int COB_dump_bsi_fabric_map(const char* shelf, 
		              uint16_t  slot_mask,
		               uint8_t  bay_mask,
		               uint8_t  rce_mask,
		                   int  verbose)
{
   // Fabric Map is DTM only
   bay_mask &= 1<<DTM;
   cob_dump_bsi(shelf, slot_mask, bay_mask, rce_mask, verbose, 
		bsi_dump_fabric_map); 
   return 0;
}

   

static int cob_dump_bsi(const char* shelf, 
			uint16_t  slot_mask,
			uint8_t  bay_mask,
			uint8_t  rce_mask,
		        int  verbose,
			bsi_dump_fn fn)
{
   COB_ctx* ctx;

   uint8_t cmb_rs_buf[COB_GET_CMB_GPIO_RS_SIZE];
   COB_get_cmb_gpio_rs* cmb_rs;

   uint8_t ipmb_addr;

   int slot, s;

   char loc[80];

   int bay;
   int rce;

   const BSI_version_info* info;

   ctx = COB_open(shelf);
   if(NULL == ctx) 
   {
      printf("ERROR - Couldn't connect to shelf manager: %s\n", shelf);
      return -1;
   }

   if(verbose)
   {
      printf("%s: Dumping BSI info for "
	     "Slot Mask: %04x, Bay Mask: %02x, RCE Mask: %02x\n", 
	     shelf, slot_mask, bay_mask, rce_mask);
   }
   

   for(slot=ATCA_MIN_SLOT; slot<=ATCA_MAX_SLOT; ++slot)
   {
      s = slot-1;
      if(!(slot_mask & (1<<s))) continue;

      ipmb_addr = COB_ipmb_addr(ctx, slot);
      if(0xff == ipmb_addr) continue;
      
      if(!COB_slot_present(ctx, ipmb_addr)) continue;

      for(bay=0; bay<COB_MAX_BAYS; ++bay)
      {
	 if(!(bay_mask & (1<<bay))) continue;
	 
	 cmb_rs = COB_get_cmb_gpio(ctx, ipmb_addr, bay, cmb_rs_buf);
	 if(cmb_rs == NULL || cmb_rs->ccode) continue;

	 for(rce=0; rce<COB_BAY_MAX_RCES; ++rce)
	 {
	    if(!(rce_mask & (1<<rce))) continue;
	    if(!(cmb_rs->rce_det & (1<<rce))) continue;
	    if(!(cmb_rs->rce_dne & (1<<rce))) continue;

	    snprintf(loc, sizeof(loc), "BSI: %s/%i/%s/RCE%i "
		     "(%s/%i/%i/%i)",
		     shelf, slot, COB_util_get_bay_name(bay), rce,
		     shelf, slot, bay, rce);
	    printf("%s\n", ddashes);
	    printf("| %-76s |\n", loc);

	    if(fn == &bsi_dump_raw)
	    {
	       bsi_dump_raw(ctx, ipmb_addr, bay, rce, NULL);
	    }
	    else
	    {
	       info = get_bsi_version_info(ctx, ipmb_addr, bay, rce);
	       if(NULL == info) continue;
	       fn(ctx, ipmb_addr, bay, rce, info);
	    }

	    printf("%s\n", dashes);
	 }	   
      }
   }
   
   COB_close(ctx);	 
   
   return 0;
}

static int bsi_read(const COB_ctx* ctx, int ipmb_addr, int bay, int rce,
		    unsigned int offset, unsigned int length, 
		    void* dest_buffer)
{
   uint8_t bsi_rs_buf[COB_BSI_READ_RS_SIZE];
   COB_bsi_read_rs* bsi_rs;

   uint8_t* dest = dest_buffer;
   
   int bytes_read = 0;
   int to_read = length;
   int addr = offset;
   int len;

   static const int chunk_size = 0x10;

   if((length <= 0) || (offset <0) || (offset > RCE_BSI_SIZE)) return 0;

   do
   {
      len = (to_read - chunk_size)>0 ? chunk_size : to_read;

      bsi_rs = COB_bsi_read(ctx, ipmb_addr, bay, rce, addr, len, bsi_rs_buf);

      if((NULL == bsi_rs) || (bsi_rs->ccode)) return bytes_read;

      memcpy(&(dest[bytes_read]), bsi_rs->data, len);

      bytes_read += len; 
      to_read -= len;
      addr += len;

   } while(to_read>0);
   
   return bytes_read;
}

static const BSI_version_info* get_bsi_version_info(const COB_ctx* ctx,
						    int ipmb_addr,
						    int bay,
						    int rce)
{
   uint32_t version;
   int len;
   int v;

   static const int nversions = ( sizeof(bsi_version_info)
				 /sizeof(BSI_version_info));
   const BSI_version_info* info;

   len = bsi_read(ctx, ipmb_addr, bay, rce, 0x00, sizeof(uint32_t), &version);
   
   if(len != sizeof(uint32_t)) return NULL;
   
   for(v=0; v<nversions; ++v)
   {
      if(bsi_version_info[v].version == version) 
      {
	 info = &(bsi_version_info[v]);
	 break;
      }
   }

   if(v == nversions)
   {
      printf("ERROR - Unsupported BSI version %i\n", version);
      return NULL;
   }

   return info;
}

static void bsi_dump_raw(const COB_ctx* ctx, int ipmb_addr, int bay, int rce, const BSI_version_info* info)
{
   int addr;
   int byte;

   uint8_t bsi_rs_buf[COB_BSI_READ_RS_SIZE];
   COB_bsi_read_rs* bsi_rs;

   uint8_t last_line[16];
   int first_line = 1;
   int match;
   int first_match = 0;

   printf("| RAW Dump %67s |\n","");
   printf("%s\n", dashes);

   for(addr=0; addr<RCE_BSI_SIZE; addr+=0x10)
   {
      bsi_rs = COB_bsi_read(ctx, ipmb_addr, bay, rce, 
			    addr, 0x10, bsi_rs_buf); 
      
      if((bsi_rs != NULL) && (0x00 == bsi_rs->ccode))
      {
	 if(first_line)
	 {
	    first_line = 0;
	    match = 0;
	 }
	 else
	 {
	    match = memcmp(last_line, bsi_rs->data, 16) ? 0 : 1;
	    if(match && first_match)
	    {
	       printf(" %03x:  ...\n", addr);
	       first_match = 0;
	    }
	 }

	 if(!match)
	 {
	    first_match = 1;
	    printf(" %03x:", addr);
	    for(byte=0; byte<bsi_rs->len; ++byte)
	    {
	       if(0 == byte%8) printf(" ");
	       printf(" %02x", bsi_rs->data[byte]);
	    }
	    printf("\n");
	 }

	 memcpy(last_line, bsi_rs->data, 16);

      }
   }

   printf(" %03x:", addr-16);
   for(byte=0; byte<bsi_rs->len; ++byte)
   {
      if(0 == byte%8) printf(" ");
      printf(" %02x", last_line[byte]);
   }
   printf("\n");
   
   return;
}

static void bsi_dump_rce(const COB_ctx* ctx, int ipmb_addr, int bay, int rce, const BSI_version_info* info)
{
   BSI_rce_data rce_data;

   int length = info->rce_data_size;
   int offset = info->rce_data_offset;
   int len;

   int lane;
   int type;

   if((offset == BSI_OFFSET_NULL) || (length == 0)) return;

   len = bsi_read(ctx, ipmb_addr, bay, rce, offset, length, &rce_data);

   if(len != length) return;

   printf("| RCE Information %60s |\n","");
   printf("%s\n", dashes);
   switch(info->version)
   {
   case 2:
      printf("|  BSI version number: 0x%08x\n", rce_data.v2.bsi_version);
      printf("|  UBOOT version:      %s\n", rce_data.v2.uboot_version);
      printf("|  RPT SW Tag:        %s\n", rce_data.v2.dat_version);
      printf("|  Zynq Device DNA: 0x%02x%02x%02x%02x %02x%02x%02x%02x\n",
	     rce_data.v2.device_dna[7],
	     rce_data.v2.device_dna[6],
	     rce_data.v2.device_dna[5],
	     rce_data.v2.device_dna[4],
	     rce_data.v2.device_dna[3],
	     rce_data.v2.device_dna[2],
	     rce_data.v2.device_dna[1],
	     rce_data.v2.device_dna[0]);
      printf("|  Zynq eFUSE User: 0x%02x%02x%02x%02x %02x%02x%02x%02x\n",
	     rce_data.v2.efuse_user[7],
	     rce_data.v2.efuse_user[6],
	     rce_data.v2.efuse_user[5],
	     rce_data.v2.efuse_user[4],
	     rce_data.v2.efuse_user[3],
	     rce_data.v2.efuse_user[2],
	     rce_data.v2.efuse_user[1],
	     rce_data.v2.efuse_user[0]);

      /* MAC address is in network order, so print swapped */
      printf("|  RCE MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
	     rce_data.v2.rce_mac[0],
	     rce_data.v2.rce_mac[1],
	     rce_data.v2.rce_mac[2],
	     rce_data.v2.rce_mac[3],
	     rce_data.v2.rce_mac[4],
	     rce_data.v2.rce_mac[5]);

      printf("|  PHY Type: 0x%08x\n", rce_data.v2.phy_type.u32);
      for(lane=0; lane<4; ++lane)
      {
	 type = rce_data.v2.phy_type.lane[lane];
	 printf("|    Lane %i: (%i) %s\n", lane, type, lane_type_name(type));
      }

      break;
   case 1:
      printf("|  BSI version number: 0x%08x\n", rce_data.v1.bsi_version);
      
      printf("|  PHY Type: 0x%08x\n", rce_data.v1.phy_type.u32);
      for(lane=0; lane<4; ++lane)
      {
	 type = rce_data.v1.phy_type.lane[lane];
	 printf("|    Lane %i: (%i) %s\n", lane, type, lane_type_name(type));
      }

      printf("|  RCE MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
	     rce_data.v1.rce_mac[5],
	     rce_data.v1.rce_mac[4],
	     rce_data.v1.rce_mac[3],
	     rce_data.v1.rce_mac[2],
	     rce_data.v1.rce_mac[1],
	     rce_data.v1.rce_mac[0]);

      break;
   default:
      printf("BSI Version %i not supported.\n", rce_data.v0.bsi_version);
      break;
   }

   return;
}

static void bsi_dump_cluster_data(const COB_ctx* ctx, int ipmb_addr, int bay, int rce, const BSI_version_info* info)
{
   BSI_cluster_data data;

   int length = info->cluster_data_size;
   int offset = info->cluster_data_offset;

   int len;
   
   if((offset == BSI_OFFSET_NULL) || (length == 0)) return;

   len = bsi_read(ctx, ipmb_addr, bay, rce, offset, length, &data);

   if(len != length) return;

   printf("| Cluster Information %56s |\n","");
   printf("%s\n", dashes);

   printf("|  CMB ID: %02x%02x%02x%02x%02x%02x%02x%02x\n",
	  data.id[7],
	  data.id[6],
	  data.id[5],
	  data.id[4],
	  data.id[3],
	  data.id[2],
	  data.id[1],
	  data.id[0]);

   printf("|  Cluster Address: 0x%08x\n", data.addr);

   printf("|    Version     : 0x%02x\n", 
	  BSI_VERSION_FROM_CLUSTER_ADDR(data.addr));
   printf("|    Cluster No. : 0x%02x\n", 
	  BSI_CLUSTER_FROM_CLUSTER_ADDR(data.addr));
   printf("|    Bay No.     : 0x%02x\n", 
	  BSI_BAY_FROM_CLUSTER_ADDR(data.addr));
   printf("|    Element No. : 0x%02x\n", 
	  BSI_ELEMENT_FROM_CLUSTER_ADDR(data.addr));
   
   printf("|  Cluster Group Name: %s\n", data.group_name);
   
   printf("|  External Interconnect Type: 0x%08x\n", data.ext_interconnect);

}

static void bsi_dump_switch_config(const COB_ctx* ctx, int ipmb_addr, int bay, int rce, const BSI_version_info* info)
{
   BSI_cluster_switch_cfg data[BSI_N_CLUSTER_PORTS];

   int length = info->cluster_switch_data_size;
   int offset = info->cluster_switch_data_offset;

   int len;
   int port;
   int type;
   int group;
   uint32_t raw;

   if(bay != DTM) return;

   if((offset == BSI_OFFSET_NULL) || (length == 0)) return;

   len = bsi_read(ctx, ipmb_addr, bay, rce, offset, length, &data);

   if(len != length) return;

   printf("| Cluster Switch Configuration %47s |\n","");
   printf("%s\n",dashes);
   printf("| %4s | (%8s) - %5s %15s | %32s |\n",
	  "Port", "Raw", "Group", "Type", "");
   printf("%s\n",dashes);
   for(port=0; port<BSI_N_CLUSTER_PORTS; ++port)
   {
      raw = data[port];
      type = TYPE_FROM_CLUSTER_SWITCH_PORT(raw);
      group = GROUP_FROM_CLUSTER_SWITCH_PORT(raw);
      printf("|  %2i  | (%08x) - %5x %15s | %32s |\n", 
	     port, raw, group, port_type_name(type), "");
   }
   
}


/*
static void bsi_dump_status(const COB_ctx* ctx, int ipmb_addr, int bay, int rce, const BSI_version_info* info)
{
   uint32_t status;

   int length = info->boot_response_size;
   int offset = info->boot_response_offset;
   int len;

   if((offset == BSI_OFFSET_NULL) || (length == 0)) return;

   len = bsi_read(ctx, ipmb_addr, bay, rce, offset, length, &status);

   if(len != length) return;
   
   printf("RCE Boot Status: 0x%08x\n", status);
}
*/



static void bsi_dump_shelf_ip_info(const COB_ctx* ctx, int ipmb_addr, int bay, int rce, const BSI_version_info* info)
{
   COB_shelf_ip_info ip_info;

   int length = info->shelf_ip_info_size;
   int offset = info->shelf_ip_info_offset;

   int len;

   if(bay != DTM) return;

   if((offset == BSI_OFFSET_NULL) || (length == 0)) return;

   len = bsi_read(ctx, ipmb_addr, bay, rce, offset, length, &ip_info);

   if(len != length) return;

   printf("| Shelf IP Information - Done: %1i %45s |\n",
	  (unsigned int)ip_info.status.bf.done, "");
   printf("%s\n", dashes);
   printf("|      Port Membership: %08x\n",
	       (unsigned int)ip_info.port_membership);
   printf("|  Port Ingress Policy: %08x\n", 
	       (unsigned int)ip_info.port_ingress_policy);
   printf("|           Group Base: %i.%i.%i.%i\n",
	       ((unsigned int)ip_info.group_base&0x000000FF)>>0,
	       ((unsigned int)ip_info.group_base&0x0000FF00)>>8,
	       ((unsigned int)ip_info.group_base&0x00FF0000)>>16,
	       ((unsigned int)ip_info.group_base&0xFF000000)>>24);
   printf("|            Group End: %i.%i.%i.%i\n",
	       ((unsigned int)ip_info.group_end&0x000000FF)>>0,
	       ((unsigned int)ip_info.group_end&0x0000FF00)>>8,
	       ((unsigned int)ip_info.group_end&0x00FF0000)>>16,
	       ((unsigned int)ip_info.group_end&0xFF000000)>>24);
   printf("|          Subnet Mask: %i.%i.%i.%i\n",
	       ((unsigned int)ip_info.subnet_mask&0x000000FF)>>0,
	       ((unsigned int)ip_info.subnet_mask&0x0000FF00)>>8,
	       ((unsigned int)ip_info.subnet_mask&0x00FF0000)>>16,
	       ((unsigned int)ip_info.subnet_mask&0xFF000000)>>24);
   printf("|              Gateway: %i.%i.%i.%i\n",
	       ((unsigned int)ip_info.gateway&0x000000FF)>>0,
	       ((unsigned int)ip_info.gateway&0x0000FF00)>>8,
	       ((unsigned int)ip_info.gateway&0x00FF0000)>>16,
	       ((unsigned int)ip_info.gateway&0xFF000000)>>24);
   printf("%s\n", dashes);

}

static void bsi_dump_fabric_map(const COB_ctx* ctx, int ipmb_addr, int bay, int rce, const BSI_version_info* info)
{
   COB_fabric_map map;

   int length = info->fabric_map_size;
   int offset = info->fabric_map_offset;

   int len;

   int slot;
   int chan;
   char err;
   int empty;
   char* hub;

   if(bay != DTM) return;

   if((offset == BSI_OFFSET_NULL) || (length == 0)) return;

   len = bsi_read(ctx, ipmb_addr, bay, rce, offset, length, &map);

   if(len != length) return;

   hub = (1 == map.logical_slot.bf.hub) ? "Yes" : "No";

   printf("%s\n", dashes);
   printf("| Fabric Map - Local Slot %-2i - HUB: %-3s               |\n", 
		(int)map.logical_slot.bf.slot+1,
		hub);
   printf("|---------------------------");
   printf("--------------------------|\n");
   printf("| Local Slot-Ch -> Remote  | Local Slot-Ch -> Remote  |\n");
   printf("|--------------------------|");
   printf("--------------------------|\n");

   for(slot=0; slot<16; ++slot)
   {
      empty = 1;
      for(chan=0; chan<16; ++chan)
      {
	 err = map.slot[slot].chan[chan].bf.error ? '*' : ' ';
	 if(map.slot[slot].chan[chan].ui == 0x0) continue;
	 empty = 0;
	 printf("|%cS%02i-C%02i (P%02i) -> S%02i-C%02i ", 
		      err,
		      slot+1, chan+1, 
		      map.slot[slot].chan[chan].bf.port,
		      map.slot[slot].chan[chan].bf.slot+1,
		      map.slot[slot].chan[chan].bf.chan+1);
	 if(chan%2) printf("|\n");
      }
      if(!empty)
      {
	 printf("|\n");
	 printf("|--------------------------|");
	 printf("--------------------------|\n");
      }
   }


}

static const char* lane_type_name(int type)
{
   switch(type)
   {
   case RCE_NET_PHY_TYPE_UNUSED:
      return "Unused";
   case RCE_NET_PHY_TYPE_1000BASEX:
      return "1G";
   case RCE_NET_PHY_TYPE_10GBASEX:
      return "10G";
   case RCE_NET_PHY_TYPE_XAUI:
      return "XAUI";
   default:
      return "Undefined";
   }
}

static const char* port_type_name(int type)
{
   switch(type)
   {
   case CMB_SWITCH_PORT_TYPE_NC:
      return "Not Connected";
   case CMB_SWITCH_PORT_TYPE_DISABLED:
      return "Disabled";
   case CMB_SWITCH_PORT_TYPE_1000BASEX_LANE0:
      return "1G-Lane0";
   case CMB_SWITCH_PORT_TYPE_1000BASEX_LANE1:
      return "1G-Lane1";
   case CMB_SWITCH_PORT_TYPE_1000BASEX_LANE2:
      return "1G-Lane2";
   case CMB_SWITCH_PORT_TYPE_1000BASEX_LANE3:
      return "1G-Lane3";
   case CMB_SWITCH_PORT_TYPE_XAUI:
      return "XAUI";
   case CMB_SWITCH_PORT_TYPE_10GBASEX_LANE0:
      return "10G-Lane0";
   case CMB_SWITCH_PORT_TYPE_10GBASEX_LANE1:
      return "10G-Lane1";
   case CMB_SWITCH_PORT_TYPE_10GBASEX_LANE2:
      return "10G-Lane2";
   case CMB_SWITCH_PORT_TYPE_10GBASEX_LANE3:
      return "10G-Lane3";
   default:
      return "Undefined";
   }
}

