#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>

#include <freeipmi/freeipmi.h>
#include <freeipmi/api/ipmi-api.h>

#include "shelf_fru/shelf_ip_info.h"

#define PACKED __attribute__((packed))

static int verbose = 0;
static const char* dashes  = \
   "-----------------------------------------------------";
static const char* ddashes = \
   "=====================================================";


/*
  Here are some structures that are handy when interpreting 
  FRU information along with some handy SLAC record specific 
  enums
*/
typedef struct
{
   uint8_t header_format;
   uint8_t internal_use_area_offset;
   uint8_t chassis_area_offset;
   uint8_t board_area_offset;
   uint8_t product_area_offset;
   uint8_t multirecord_area_offset;
   uint8_t pad;
   uint8_t checksum;
} PACKED fru_info_common_header;

typedef struct {
   uint8_t record_type;
   uint8_t format_version;
   uint8_t record_length;
   uint8_t record_checksum;
   uint8_t header_checksum;
} PACKED ipmi_multirecord_header;

typedef struct
{
   uint8_t record_id;
   uint8_t eol_format;
   uint8_t record_len;
   uint8_t record_chk;
   uint8_t header_chk;
   uint8_t manufacturer_id[3];
   uint8_t picmg_record_id;
   uint8_t picmg_format_version;
} PACKED picmg_multirecord_header;

static const uint8_t SLAC_OEM_MANUFACTURER_ID[3] = {0x50, 0x9D, 0x00};
static const uint8_t SLAC_SHELF_IP_INFO_RECORD_ID = 0x01;

/*
  Structure of the data portion of the SLAC Shelf IP Info Record 
  This is the portion following the PICMG/SLAC header
*/
typedef struct
{
   uint32_t vlan;
   uint32_t ingress;
   uint32_t group_base;
   uint32_t group_end;
   uint32_t subnet_mask;
   uint32_t gateway;
} PACKED shelf_ip_info;

/*
  Since freeipmi doesn't define any PICMG stuff here are some
  helpful enums
*/
enum 
{
   PICMG_CMD_FRU_INVENTORY_DEVICE_LOCK_CONTROL = 0x1f,
   PICMG_CMD_FRU_INVENTORY_DEVICE_WRITE         = 0x20,
};

enum
{
   PICMG_DEFINING_BODY_ID = 0,
   PICMG_OEM_RECORD_ID    = 0xC0,
};

enum
{
   ACTIVE_SHELF_MANAGER_IPMB_ADDRESS = 0x20,
   SHELF_FRU_INFORMATION_FRU_ID      = 254,
};

/* 
   These are structures which can be passed to the freeipmi 
   ipmi_cmd_raw_ipmb command. They are just the IPMI/PICMG 
   Command/Response definitions prepended with the command 
   number
*/

typedef struct
{
   uint8_t  cmd;
   uint8_t  fru;
} PACKED fru_inventory_area_info_rq;

typedef struct
{
   uint8_t  cmd;
   uint8_t  completion_code;
   uint16_t size;
   uint8_t  by_words;
} PACKED fru_inventory_area_info_rs;


typedef struct
{
   uint8_t  cmd;
   uint8_t  picmg_id;
   uint8_t  fru;
   uint8_t  operation;
   uint16_t lock_id;
} PACKED fru_inventory_device_lock_rq;

typedef struct
{
   uint8_t  cmd;
   uint8_t  completion_code;
   uint8_t  picmg_id;
   uint16_t lock_id;
   uint32_t timestamp;
} PACKED fru_inventory_device_lock_rs;

typedef struct
{
   uint8_t  cmd;
   uint8_t  picmg_id;
   uint8_t  fru;
   uint16_t lock_id;
   uint16_t offset;
   uint8_t  data[1];
} PACKED fru_inventory_device_write_rq;

typedef struct
{
   uint8_t cmd;
   uint8_t completion_code;
   uint8_t picmg_id;
   uint8_t count;
} PACKED fru_inventory_device_write_rs;

typedef struct
{
   uint8_t  cmd;
   uint8_t  fru;
   uint16_t offset;
   uint8_t  count;
} PACKED read_fru_data_rq;

typedef struct
{
   uint8_t cmd;
   uint8_t completion_code;
   uint8_t count;
   uint8_t data[1];
} PACKED read_fru_data_rs;


/* 
   Here are forward declarations of the local functions in this file.
   Also some enumerations of useful parameters
*/

enum
{
   FRU_DEVICE_LOCK_OP_TIMESTAMP = 0,
   FRU_DEVICE_LOCK_OP_LOCK = 1,
   FRU_DEVICE_LOCK_OP_DISCARD = 2,
   FRU_DEVICE_LOCK_OP_COMMIT = 3,
};

static void       close_lan(ipmi_ctx_t ipmi_ctx);
static ipmi_ctx_t open_lan(const char* hostname);

static int reset_shelf_manager(ipmi_ctx_t ctx);

static int get_shelf_fru_info_size(  ipmi_ctx_t  ctx, 
				   unsigned int *size);

static int get_shelf_fru_info_lock(ipmi_ctx_t  ctx, 
				          int  op, 
				     uint16_t* lock_id, 
				       time_t* timestamp);

static int read_shelf_fru_data(    ipmi_ctx_t  ctx, 
			             uint16_t  offset, 
			              uint8_t  count, 
			              uint8_t* dest);

static int write_shelf_fru_data(   ipmi_ctx_t  ctx, 
				     uint16_t  lock_id,
				     uint16_t  offset, 
				      uint8_t  count, 
				const uint8_t* src);

static int fetch_shelf_fru_multirecord_area(  ipmi_ctx_t  ctx, 
					         uint8_t* buffer, 
					    unsigned int* mra_size);

static int write_shelf_fru_multirecord_area(   ipmi_ctx_t  ctx,
					           time_t  read_timestamp,
					    const uint8_t* buf, 
					     unsigned int  size);

static int is_slac_shelf_ip_record(const picmg_multirecord_header* hdr);

static uint8_t* find_shelf_ip_info_record(uint8_t* buf, uint16_t buf_size);

static int remove_slac_shelf_ip_record(uint8_t* buf, unsigned int* buf_size);

static int add_slac_shelf_ip_info(const shelf_ip_info* info, 
				              uint8_t* buf, 
				         unsigned int* buf_size);

static void dump_buffer(     uint8_t* buf, 
			unsigned int  size);

static void dump_slac_shelf_ip_record(         const char* tag, 
				      const shelf_ip_info* info);

static void dump_slac_shelf_ip_record_raw(         const char* tag, 
					  const shelf_ip_info* info);


/* 
   These are the external functions
*/

void set_verbose(int v)
{
   verbose = v;
}

int set_shelf_ip_info(const set_shelf_ip_info_args* args)
{
   ipmi_ctx_t ipmi_ctx = NULL;
   time_t read_timestamp;
   uint8_t buf[IPMI_FRU_AREA_SIZE_MAX+1];
   unsigned int mra_size;
   uint8_t* record_header;
   uint16_t lock_id = 0;
   shelf_ip_info* rec_data;
   
   shelf_ip_info initial_copy;
   shelf_ip_info working_copy;

   if(NULL == (ipmi_ctx = open_lan(args->dest)))
   {
      printf("ERROR - Couldn't open lan connection to %s\n", args->dest);
      return 2;
   }
   
   if(get_shelf_fru_info_lock(ipmi_ctx, FRU_DEVICE_LOCK_OP_TIMESTAMP, 
			      &lock_id, &read_timestamp))
   {
      printf("ERROR - Couldn't read previous commit timestamp\n");
      close_lan(ipmi_ctx);
      return 2;
   }
   
   if(verbose)
   {
      printf("Shelf FRU Info previous commit timestamp is %s", 
	     ctime(&read_timestamp));
   }

   if(fetch_shelf_fru_multirecord_area(ipmi_ctx, buf, &mra_size))
   {
      close_lan(ipmi_ctx);
      return 2;
   }

   if(verbose>2)
   {
      printf("\n%s\nOriginal Multirecord area\n%s\n", ddashes, dashes);
      dump_buffer(buf, mra_size);
   }
   
   record_header = find_shelf_ip_info_record(buf, mra_size);

   if(NULL == record_header)
   {
      printf("No Existing Shelf IP Info Record found on %s\n", args->dest);
      if( !args->vlan_valid       || 
	  !args->untagged_valid   || 
	  !args->tagged_valid     || 
	  !args->violation_valid  || 
	  !args->group_base_valid ||
	  !args->group_end_valid  ||
	  !args->subnet_mask_valid  )
      {
	 if(args->force)
	 {
	    printf("Forcing the creation of a new empty record\n");
	 }
	 else
	 {
	    printf("ERROR - Can't create new record with incomplete "
		   "set of parameters\n");
	    close_lan(ipmi_ctx);
	    return 1;
	 }
      }
      memset(&working_copy, 0, sizeof(shelf_ip_info));
   }
   else if(!args->force && !(args->vlan_valid        || 
			     args->untagged_valid    || 
			     args->tagged_valid      || 
			     args->violation_valid   || 
			     args->group_base_valid  ||
			     args->group_end_valid   ||
			     args->subnet_mask_valid ||
			     args->gateway_valid ))
   {
      printf("ERROR - All parameters have been selected as unchanged, "
	     "no point in proceeding\n");
      return 1;
   }
   else  
   {
      rec_data = (shelf_ip_info*)(record_header
				  +sizeof(picmg_multirecord_header));
      memcpy(&initial_copy, rec_data, sizeof(shelf_ip_info));
      memcpy(&working_copy, rec_data, sizeof(shelf_ip_info));
   }

   if(verbose)
   {
      printf("\n%s\n",ddashes);
      printf(" Initial Shelf IP Info\n");
      dump_slac_shelf_ip_record(args->dest, &working_copy);
   }
   if(verbose>1)
   {
      dump_slac_shelf_ip_record_raw(args->dest, &working_copy);
   }

   if(args->vlan_valid)
   {
      working_copy.vlan = args->vlan;
   }

   if(args->untagged_valid)
   {
      working_copy.ingress &= 0x6;
      working_copy.ingress |= args->untagged;
   }

   if(args->tagged_valid)
   {
      working_copy.ingress &= 0x5;
      working_copy.ingress |= (args->tagged)<<1;
   }

   if(args->violation_valid)
   {
      working_copy.ingress &= 0x3;
      working_copy.ingress |= (args->violation)<<2;
   }

   if(args->group_base_valid)
   {
      working_copy.group_base = args->group_base;
   }

   if(args->group_end_valid)
   {
      working_copy.group_end = args->group_end;
   }

   if(args->subnet_mask_valid)
   {
      working_copy.subnet_mask = args->subnet_mask;
   }

   if(args->gateway_valid)
   {
      working_copy.gateway = args->gateway;
   }

   if((0 == (working_copy.group_base & ~working_copy.subnet_mask))
      && (0 != working_copy.group_base))
   {
      printf("ERROR - Group Base IP cannot be lowest network address\n");
      close_lan(ipmi_ctx);
      return 1;
   }
   
   if(0xFFFFFFFF == (working_copy.group_base | working_copy.subnet_mask))
   {
      printf("ERROR - Group Base IP cannot be broadcast address\n");
      close_lan(ipmi_ctx);
      return 1;
   }
   
   if((working_copy.group_base & working_copy.subnet_mask) !=
      (working_copy.group_end  & working_copy.subnet_mask))
   {
      printf("ERROR - Group Base and Group End must be in same network\n");
      close_lan(ipmi_ctx);
      return 1;
   }
      
   if(ntohl(working_copy.group_base) > ntohl(working_copy.group_end))
   {
      printf("ERROR - Group Base must be less than Group End\n");
      close_lan(ipmi_ctx);
      return 1;
   }

   if(((working_copy.gateway & working_copy.subnet_mask) !=
       (working_copy.group_base & working_copy.subnet_mask)) &&
      (0 != ntohl(working_copy.gateway)))
   {
      printf("ERROR - Gateway IP must be in the same network as Group Base\n");
      close_lan(ipmi_ctx);
      return 1;
   }

   if((0 != ntohl(working_copy.gateway)) &&
      ((ntohl(working_copy.gateway) < ntohl(working_copy.group_base)) ||
       (ntohl(working_copy.gateway) > ntohl(working_copy.group_end))))
   {
      printf("ERROR - Gateway IP must be between Group Base and Group End (inclusive)\n");
      close_lan(ipmi_ctx);
      return 1;
   }       

   if(verbose)
   {
      printf("\n%s\n",ddashes);
      printf(" New Shelf IP Info\n");
      dump_slac_shelf_ip_record(args->dest, &working_copy);
   }
   if(verbose>1)
   {
      dump_slac_shelf_ip_record_raw(args->dest, &working_copy);
   }
   if(verbose)
   {
      printf("\n");
   }

   if(!args->force)
   {
      if(0 == memcmp(&initial_copy, &working_copy, sizeof(shelf_ip_info)))
      {
	 printf("New Shelf FRU information is the same as the old, "
		"not writing to shelf\n");
	 close_lan(ipmi_ctx);
	 return 1;
      }
   }

   remove_slac_shelf_ip_record(buf, &mra_size);

   if(verbose>2)
   {
      printf("\n%s\nMultiRecord Area w/ SLAC Record Removed\n%s\n", 
	     ddashes, dashes);
      dump_buffer(buf, mra_size);
   }

   add_slac_shelf_ip_info(&working_copy, buf, &mra_size);

   if(verbose>2)
   {
      printf("\n%s\nMultiRecord Area w/ New SLAC Record\n%s\n",
	     ddashes, dashes);
      dump_buffer(buf, mra_size);
      printf("\n");
   }

   if(args->dry_run)
   {
      printf("This was a DRY-RUN, not writing to Shelf\n");
   }
   else if(write_shelf_fru_multirecord_area(ipmi_ctx, 
					    read_timestamp, 
					    buf, mra_size))
   {
      printf("ERROR - Couldn't write Shelf FRU Info to shelf\n");
      close_lan(ipmi_ctx);
      return 2;
   }

   close_lan(ipmi_ctx);

   return 0;
}

int delete_shelf_ip_info(const delete_shelf_ip_info_args* args)
{
   ipmi_ctx_t ipmi_ctx = NULL;
   time_t read_timestamp;
   uint8_t buf[IPMI_FRU_AREA_SIZE_MAX+1];
   unsigned int mra_size;
   uint16_t lock_id = 0;

   if(NULL == (ipmi_ctx = open_lan(args->shelf)))
   {
      printf("ERROR - Couldn't open lan connection to %s\n", args->shelf);
      return -1;
   }
   
   if(get_shelf_fru_info_lock(ipmi_ctx, FRU_DEVICE_LOCK_OP_TIMESTAMP, 
			      &lock_id, &read_timestamp))
   {
      printf("ERROR - Couldn't read previous commit timestamp\n");
      close_lan(ipmi_ctx);
      return 2;
   }
   
   if(verbose)
   {
      printf("Shelf FRU Info previous commit timestamp is %s", 
	     ctime(&read_timestamp));
   }

   
   if(fetch_shelf_fru_multirecord_area(ipmi_ctx, buf, &mra_size))
   {
      close_lan(ipmi_ctx);
      return 2;
   }
   
   if(verbose>2)
   {
      printf("\n%s\nOriginal Multirecord area\n%s\n", ddashes, dashes);
      dump_buffer(buf, mra_size);
      printf("\n");
   }

   if(verbose)
   {
      printf("MultiRecord Area size before removal of "
	     "SLAC Shelf IP Record %x\n", mra_size);
   }

   if(remove_slac_shelf_ip_record(buf, &mra_size))
   {
      printf("ERROR - SLAC Shelf IP Record not found\n");
      close_lan(ipmi_ctx);
      return 1;
   }
   
   if(verbose)
   {
      printf("MultiRecord Area size after removal of "
	     "SLAC Shelf IP Record, %x\n", mra_size);
   }
   
   if(verbose>2)
   {
      printf("\n%s\nMultiRecord Area w/ SLAC Record Removed\n%s\n", 
	     ddashes, dashes);
      dump_buffer(buf, mra_size);
      printf("\n");
   }

   if(args->dry_run)
   {
      printf("This was a DRY-RUN, not writing to Shelf\n");
      close_lan(ipmi_ctx);

      return 0;
   }
   else if(write_shelf_fru_multirecord_area(ipmi_ctx, 
					    read_timestamp, 
					    buf, mra_size))
   {
      printf("ERROR - Couldn't write Shelf FRU Info to shelf\n");
      close_lan(ipmi_ctx);
      return 2;
   }

   reset_shelf_manager(ipmi_ctx);
   
   close_lan(ipmi_ctx);

   return 0;
}

int display_shelf_ip_info(const display_shelf_ip_info_args* args)
{
   ipmi_ctx_t ipmi_ctx = NULL;
   time_t read_timestamp;
   shelf_ip_info info;
   uint16_t lock_id = 0;
   uint8_t buf[IPMI_FRU_AREA_SIZE_MAX+1];
   unsigned int mra_size;
   uint8_t* record_header;
   shelf_ip_info* rec_data;

   if(NULL == (ipmi_ctx = open_lan(args->shelf)))
   {
      printf("ERROR - Couldn't open lan connection to %s\n", args->shelf);
      return 2;
   }
   
   if(get_shelf_fru_info_lock(ipmi_ctx, FRU_DEVICE_LOCK_OP_TIMESTAMP, 
			      &lock_id, &read_timestamp))
   {
      printf("ERROR - Couldn't read previous commit timestamp\n");
      close_lan(ipmi_ctx);
      return 2;
   }
   
   printf("\n");
   printf("Shelf FRU Info previous commit timestamp is %s", 
	  ctime(&read_timestamp));

   if(fetch_shelf_fru_multirecord_area(ipmi_ctx, buf, &mra_size))
   {
      close_lan(ipmi_ctx);
      return 2;
   }


   if(args->mr_area)
   {
      printf("\n%s\nMultirecord area\n%s\n", ddashes, dashes);
      dump_buffer(buf, mra_size);
      printf("\n");
   }

   record_header = find_shelf_ip_info_record(buf, mra_size);
   
   if(NULL == record_header)
   {
      printf("No Existing Shelf IP Info Record found on %s\n", args->shelf);
      printf("\n");
      close_lan(ipmi_ctx);
      return 1;
   }

   rec_data = (shelf_ip_info*)(record_header
			       +sizeof(picmg_multirecord_header));
   memcpy(&info, rec_data, sizeof(shelf_ip_info));

   printf("\n%s\n",ddashes);
   printf(" Shelf IP Info\n");
   dump_slac_shelf_ip_record(args->shelf, &info);
   if(args->raw)
   {
      dump_slac_shelf_ip_record_raw(args->shelf, &info);
   }
   printf("\n");
      
   close_lan(ipmi_ctx);

   return 0;

}

/* 
   Here are the definitions of the local functions
*/

static void close_lan(ipmi_ctx_t ipmi_ctx)
{
   ipmi_ctx_close(ipmi_ctx);
   ipmi_ctx_destroy(ipmi_ctx);
}

static ipmi_ctx_t open_lan(const char* hostname)
{
   ipmi_ctx_t ipmi_ctx = NULL;
   
   if(!(ipmi_ctx = ipmi_ctx_create()))
   {
      printf("ERROR - ipmi_ctx = NULL\n");
      close_lan(ipmi_ctx);
      return(NULL);
   }

   if(0 != ipmi_ctx_open_outofband(ipmi_ctx,
				   hostname,
				   "", // username
				   "", // password
				   IPMI_AUTHENTICATION_TYPE_NONE,
				   IPMI_PRIVILEGE_LEVEL_ADMIN,
				   IPMI_SESSION_TIMEOUT_DEFAULT,
				   15000,
				   IPMI_WORKAROUND_FLAGS_DEFAULT,
				   IPMI_FLAGS_DEFAULT))
   {
      close_lan(ipmi_ctx);
      return(NULL);
   }
      
   return ipmi_ctx;
}

static int reset_shelf_manager(ipmi_ctx_t ctx)
{
   uint8_t cmd = IPMI_CMD_COLD_RESET;
   uint8_t netfn = IPMI_NET_FN_APP_RQ;
   uint8_t rs[5];
   int ret;

   printf("Resetting the Shelf Manager\n");
   ret = ipmi_cmd_raw_ipmb(ctx,
			   0, // channel number
			   ACTIVE_SHELF_MANAGER_IPMB_ADDRESS,
			   0, // rs_lun
			   netfn,
			   &cmd, 1,
			   &rs, 5);
   
   if(ipmi_ctx_errnum(ctx) || rs[1])
   {
      printf("ERROR - Couldn't Reset Shelf Manager\n");
      return -1;
   }

   return 0;
}

static int get_shelf_fru_info_size(ipmi_ctx_t ctx, unsigned int *size)
{
   fru_inventory_area_info_rq rq;
   fru_inventory_area_info_rs rs;
   uint8_t netfn;
   int ret;

   memset(&rs, 0, sizeof(fru_inventory_area_info_rs));
   
   netfn  = IPMI_NET_FN_STORAGE_RQ;
   rq.cmd = IPMI_CMD_GET_FRU_INVENTORY_AREA_INFO;
   rq.fru = SHELF_FRU_INFORMATION_FRU_ID; 
   
   ret = ipmi_cmd_raw_ipmb(ctx,
			   0,    // channel number
			   ACTIVE_SHELF_MANAGER_IPMB_ADDRESS, 
			   0,    // rs_lun
			   netfn,
			   &rq, sizeof(rq),
			   &rs, sizeof(rs));

   if(ipmi_ctx_errnum(ctx) || rs.completion_code)
   {
      printf("ERROR - Couldn't fetch Shelf FRU Info Size\n");
      return -1;
   }

   *size = rs.size;
   return 0;
}

static int get_shelf_fru_info_lock(ipmi_ctx_t ctx, 
				   int op, 
				   uint16_t* lock_id, 
				   time_t* timestamp)
{
   fru_inventory_device_lock_rq rq;
   fru_inventory_device_lock_rs rs;
   uint8_t netfn;
   int ret;

   netfn = IPMI_NET_FN_GROUP_EXTENSION_RQ;
   rq.cmd = PICMG_CMD_FRU_INVENTORY_DEVICE_LOCK_CONTROL;
   rq.picmg_id = PICMG_DEFINING_BODY_ID;
   rq.fru = SHELF_FRU_INFORMATION_FRU_ID;
   rq.operation = op;
   rq.lock_id = *lock_id;
   
   ret = ipmi_cmd_raw_ipmb(ctx,
			   0,    //channel number
			   ACTIVE_SHELF_MANAGER_IPMB_ADDRESS,
			   0,    //rs_lun
			   netfn,
			   &rq, sizeof(rq),
			   &rs, sizeof(rs));
   
   if(ipmi_ctx_errnum(ctx) || rs.completion_code)
   {
      printf("ERROR - Couldn't execute FRU Info Lock\n");
      return -1;
   }

   *timestamp = (time_t)rs.timestamp;
   *lock_id = rs.lock_id;

   return 0;
}

static int read_shelf_fru_data(ipmi_ctx_t ctx, 
			       uint16_t offset, uint8_t count, 
			       uint8_t* const dest)
{
   read_fru_data_rq rq;
   read_fru_data_rs* rs;
   int ret;
   uint8_t buf[255];
   uint8_t netfn;

   memset(buf, 0, 255);

   netfn     = IPMI_NET_FN_STORAGE_RQ;
   rq.cmd    = IPMI_CMD_READ_FRU_DATA;
   rq.fru    = SHELF_FRU_INFORMATION_FRU_ID;
   rq.offset = offset;
   rq.count  = count;

   ret = ipmi_cmd_raw_ipmb(ctx,
			   0,    // channel number
			   ACTIVE_SHELF_MANAGER_IPMB_ADDRESS,
			   0,    // rs_lun
			   netfn,
			   &rq, sizeof(rq),
			   buf, 255);


   rs = (read_fru_data_rs*)buf;

   if(ipmi_ctx_errnum(ctx) || rs->completion_code)
   {
      printf("ERROR - Could not read Shelf FRU Information\n");
      return -1;
   }

   memcpy(dest, rs->data, rs->count);

   return 0;
}

static int write_shelf_fru_data(ipmi_ctx_t ctx, uint16_t lock_id,
				uint16_t offset, uint8_t count, 
				const uint8_t* src)
{
   fru_inventory_device_write_rq* rq;
   fru_inventory_device_write_rs  rs;
   int ret;
   uint8_t buf[255];
   uint8_t netfn;

   netfn = IPMI_NET_FN_GROUP_EXTENSION_RQ;

   rq = (fru_inventory_device_write_rq*)buf;
   rq->cmd = PICMG_CMD_FRU_INVENTORY_DEVICE_WRITE;
   rq->picmg_id = PICMG_DEFINING_BODY_ID;
   rq->fru = SHELF_FRU_INFORMATION_FRU_ID;
   rq->lock_id = lock_id;
   rq->offset = offset;
   memcpy(rq->data, src, count);

   ret = ipmi_cmd_raw_ipmb(ctx,
			   0,    // channel number
			   ACTIVE_SHELF_MANAGER_IPMB_ADDRESS,
			   0,    // rs_lun
			   netfn,
			   rq,  sizeof(rq)+count-1,
			   &rs, sizeof(rs));


   if(ipmi_ctx_errnum(ctx) || rs.completion_code)
   {
      printf("ERROR - Could not write Shelf FRU Information\n");
      return -1;
   }

   return 0;
}

static int fetch_shelf_fru_multirecord_area(ipmi_ctx_t ctx, 
					    uint8_t* buffer, 
					    unsigned int* mra_size)
{
   uint8_t buf[9];
   fru_info_common_header* hdr;

   unsigned int src_offset;
   unsigned int dst_offset;
   unsigned int to_read;
   unsigned int fru_size;
   static const unsigned int read_len = 16;

   memset(buf, 0, 9);

   if(get_shelf_fru_info_size(ctx, &fru_size))
   {
      return -1;
   }

   if(read_shelf_fru_data(ctx, 0, 8, buf))
   {
      return -1;
   }
   
   hdr = (fru_info_common_header*)buf;

   src_offset = hdr->multirecord_area_offset*8;
   dst_offset = 0;

   
   while(src_offset<fru_size)
   {
      if(src_offset+read_len < fru_size)
      {
	 to_read = read_len;
      }
      else
      {
	 to_read = fru_size - src_offset;
      }

      if(read_shelf_fru_data(ctx, src_offset, 
			     to_read, &buffer[dst_offset]))
      {
	 return -1;
      }
      src_offset += to_read;
      dst_offset += to_read;
   }
   
   *mra_size = dst_offset;

   return 0;
}

static int write_shelf_fru_multirecord_area(ipmi_ctx_t ctx,
					    const time_t read_timestamp,
					    const uint8_t* buf, 
					    unsigned int size)
{
   uint8_t hdr_buf[sizeof(fru_info_common_header)];
   time_t timestamp;

   unsigned int src_offset;
   unsigned int dst_offset;
   unsigned int to_write;
   uint16_t lock_id = 0;
   static const unsigned int write_len = 16;

   memset(hdr_buf, 0, sizeof(fru_info_common_header));

   if(read_shelf_fru_data(ctx, 0, sizeof(fru_info_common_header), hdr_buf))
   {
      return -1;
   }

   dst_offset = ((fru_info_common_header*)hdr_buf)->multirecord_area_offset*8;
   src_offset = 0;
   
   if(get_shelf_fru_info_lock(ctx, 
			      FRU_DEVICE_LOCK_OP_LOCK, 
			      &lock_id, 
			      &timestamp))
   {
      printf("ERROR - Couldn't acquire Shelf FRU Info Lock\n");
      return -1;
   }

   if(verbose)
   {
      printf("Shelf FRU Info Lock timestamp is %s", ctime(&timestamp));
   }

   if(timestamp != read_timestamp)
   {
      printf("ERROR - Shelf FRU Info modification timestamp has changed since being read, "
	     "DISCARDING CHANGES.\n");
      get_shelf_fru_info_lock(ctx, 
			      FRU_DEVICE_LOCK_OP_DISCARD, 
			      &lock_id, 
			      &timestamp);
      return -1;
   }

   while(src_offset<size)
   {
      if(src_offset+write_len < size)
      {
	 to_write = write_len;
      }
      else
      {
	 to_write = size - src_offset;
      }

      if(write_shelf_fru_data(ctx, lock_id, dst_offset, 
			      to_write, &buf[src_offset]))
      {
	 return -1;
      }
      
      src_offset += to_write;
      dst_offset += to_write;
   }

   if(get_shelf_fru_info_lock(ctx, FRU_DEVICE_LOCK_OP_COMMIT, 
			      &lock_id, &timestamp))
   {
      printf("ERROR - Did not succesfully commit changes to "
	     "Shelf FRU Information\n");
      return -1;
   }

   printf("Shelf FRU Info Commit timestamp is %s", ctime(&timestamp));

   return 0;
}

static void dump_buffer(uint8_t* buf, unsigned int size)
{
   unsigned int i;

   for(i=0; i<size; ++i)
   {
      if(i%16 == 0)
      {
	 if(i != 0) printf("\n");
	 printf("%03x: ",i);
      }
      else if(i%8 == 0)
      {
	 printf(" ");
      }
      
      printf("%02x ",buf[i]);
   }
   printf("\n");
}

static int is_slac_shelf_ip_record(const picmg_multirecord_header* hdr)
{
   if(verbose>3)
   {
      printf("Found record_id %02x\n", hdr->record_id);
   }
   if(hdr->record_id == PICMG_OEM_RECORD_ID)
   {
      if(verbose>3)
      {
	 printf(" Manufacturer ID  %02x%02x%02x\n", 
		hdr->manufacturer_id[2],
		hdr->manufacturer_id[1],
		hdr->manufacturer_id[0]);
      }
      if(0 == memcmp(SLAC_OEM_MANUFACTURER_ID, hdr->manufacturer_id, 3))
      {
	 if(verbose>3)
	 {
	    printf("  PICMG Record ID %02x\n", hdr->picmg_record_id);
	 }
	 if(SLAC_SHELF_IP_INFO_RECORD_ID == hdr->picmg_record_id)
	 {
	    return 1;
	 }
      }
   }
   return 0;
}

static uint8_t* find_shelf_ip_info_record(uint8_t* buf, uint16_t buf_size)
{
   unsigned int offset = 0;
   picmg_multirecord_header* hdr;

   while(offset<buf_size)
   {
      hdr = (picmg_multirecord_header*)&buf[offset];
      if(is_slac_shelf_ip_record(hdr))
      {
	 return &buf[offset];	       
      }

      if(hdr->eol_format&0x80)
      {
	 break;
      }
      
      offset += hdr->record_len+sizeof(ipmi_multirecord_header);
   }
 
   return NULL;
}

static int remove_slac_shelf_ip_record(uint8_t* buf, unsigned int* buf_size)
{
   unsigned int cur_offset = 0;
   unsigned int dst_offset = 0;
   unsigned int prv_offset;
   int move = 0;
   int new_end = 0;
   picmg_multirecord_header* hdr;
   unsigned int new_buf_size = *buf_size;
   unsigned int record_size;
   
   while(cur_offset<*buf_size)
   {
      hdr = (picmg_multirecord_header*)&buf[cur_offset];
      record_size = hdr->record_len+sizeof(ipmi_multirecord_header);
      
      if(is_slac_shelf_ip_record(hdr))
      {
	 if(verbose>3)
	 {
	    printf("Found SLAC Record to remove at offset %x\n", cur_offset);
	 }
	 dst_offset = cur_offset;
	 new_buf_size = *buf_size - (hdr->record_len
				     +sizeof(ipmi_multirecord_header));
	 move = 1;
	 if(hdr->eol_format&0x80)
	 {
	    new_end = 1;
	    break;
	 }
      }
      else if(move)
      {
	 if(verbose>3)
	 {
	    printf("Moving record at offset %x to offset %x\n",
		   cur_offset, dst_offset);
	 }
	 memcpy(&buf[dst_offset], &buf[cur_offset], record_size);

	 dst_offset += record_size;
	 cur_offset += record_size;
	 continue;
      }

      if(hdr->eol_format&0x80)
      {
	 break;
      }

      prv_offset = cur_offset;
      cur_offset += record_size;
   }

   if(new_end)
   {
      if(verbose>3)
      {
	 printf("Setting new end of MultiRecord Area at offset %x\n", 
		prv_offset);
      }
      hdr = (picmg_multirecord_header*)&buf[prv_offset];
      hdr->eol_format |= 0x80;
      hdr->header_chk -= 0x80;
   }

   *buf_size = new_buf_size;
   if(new_end || move)
   {
      return 0;
   }

   return 1;
}

static int add_slac_shelf_ip_info(const shelf_ip_info* info, 
				  uint8_t* buf, unsigned int* buf_size)
{
   unsigned int cur_offset=0;
   picmg_multirecord_header* hdr;
   uint8_t* record_data;
   int i;
   int chk;

   while(cur_offset<*buf_size )
   {
      hdr = (picmg_multirecord_header*)&buf[cur_offset];
      if(hdr->eol_format&0x80)
      {
	 break;
      }
      cur_offset += hdr->record_len+sizeof(ipmi_multirecord_header);
   }
   
   hdr->eol_format &= 0x7f;
   hdr->header_chk += 0x80;

   cur_offset += hdr->record_len+sizeof(ipmi_multirecord_header);
   hdr = (picmg_multirecord_header*)&buf[cur_offset];
   record_data = &buf[cur_offset+sizeof(ipmi_multirecord_header)];

   memcpy(&buf[cur_offset+sizeof(picmg_multirecord_header)], info, 
	  sizeof(shelf_ip_info));
   memcpy(hdr->manufacturer_id, SLAC_OEM_MANUFACTURER_ID, 3);

   hdr->picmg_record_id = 0x01;
   hdr->picmg_format_version = 0x00;

   hdr->record_id = PICMG_OEM_RECORD_ID;
   hdr->eol_format = 0x82;
   hdr->record_len = ( sizeof(shelf_ip_info) 
		      + ( sizeof(picmg_multirecord_header)
			 - sizeof(ipmi_multirecord_header) ) );

   chk = 0;
   for(i=0; i<hdr->record_len; ++i)
   {
      chk += record_data[i];
   }
   hdr->record_chk = 0x100 - (chk&0xff);

   chk = 0;
   for(i=0; i<4; ++i)
   {
      chk += buf[cur_offset+i];
   }
   hdr->header_chk = 0x100 - (chk&0xff);
   
   *buf_size += sizeof(picmg_multirecord_header) + sizeof(shelf_ip_info);
   return 0;
}



static void dump_slac_shelf_ip_record(const char* tag, 
				      const shelf_ip_info* info)
{
   char str[80];

   printf("%s\n",dashes);
   printf(" SLAC Shelf IP Information (%s)\n", tag);
   printf("%s\n",dashes);
   printf("       VLAN ID (Valid): %03i (%1i)\n", 
	  info->vlan&0xfff, (info->vlan & 0x80000000)>>31);
   printf("      Discard Untagged: %i\n", info->ingress&0x1);
   printf("        Discard Tagged: %i\n", (info->ingress&0x2)>>1);
   printf(" Discard Boundary Viol: %i\n", (info->ingress&0x4)>>2);
   printf("            Group Base: %s\n",
	  inet_ntop(AF_INET, &info->group_base, str, sizeof(str)));
   printf("             Group End: %s\n",
	  inet_ntop(AF_INET, &info->group_end, str, sizeof(str)));
   printf("           Subnet Mask: %s\n",
	  inet_ntop(AF_INET, &info->subnet_mask, str, sizeof(str)));
   printf("               Gateway: %s\n",
	  inet_ntop(AF_INET, &info->gateway, str, sizeof(str)));
   printf("%s\n",dashes);
}

static void dump_slac_shelf_ip_record_raw(const char* tag, 
					  const shelf_ip_info* info)
{
   printf("%s\n",dashes);
   printf(" SLAC Shelf IP Information (%s)\n", tag);
   printf("%s\n",dashes);
   printf("       Port Membership: %08x\n", info->vlan);
   printf("   Port Ingress Policy: %08x\n", info->ingress);
   printf("            Group Base: %08x\n", info->group_base);
   printf("             Group End: %08x\n", info->group_end);
   printf("           Subnet Mask: %08x\n", info->subnet_mask);
   printf("               Gateway: %08x\n", info->gateway);
   printf("%s\n",dashes);
}
