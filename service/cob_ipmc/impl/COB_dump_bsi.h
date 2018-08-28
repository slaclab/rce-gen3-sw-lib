#ifndef COB_DUMP_BSI_H
#define COB_DUMP_BSI_H

#include <stdint.h>

typedef int (*COB_dump_bsi_cb)(const char* shelf,
			         uint16_t  slot_mask,
			          uint8_t  bay_mask,
			          uint8_t  rce_mask,
			              int  verbose);

int COB_dump_bsi_raw(const char* shelf, 
		       uint16_t  slot_mask,
		        uint8_t  bay_mask,
		        uint8_t  rce_mask,
		            int  verbose);

int COB_dump_bsi_rce(const char* shelf, 
		       uint16_t  slot_mask,
		        uint8_t  bay_mask,
		        uint8_t  rce_mask,
		            int  verbose);

int COB_dump_bsi_cluster(const char* shelf, 
		           uint16_t  slot_mask,
		            uint8_t  bay_mask,
		            uint8_t  rce_mask,
		                int  verbose);

int COB_dump_bsi_switch_config(const char* shelf, 
		                 uint16_t  slot_mask,
		                  uint8_t  bay_mask,
		                  uint8_t  rce_mask,
		                      int  verbose);

int COB_dump_bsi_shelf_ip_info(const char* shelf, 
		                 uint16_t  slot_mask,
		                  uint8_t  bay_mask,
		                  uint8_t  rce_mask,
		                      int  verbose);

int COB_dump_bsi_fabric_map(const char* shelf, 
		              uint16_t  slot_mask,
		               uint8_t  bay_mask,
		               uint8_t  rce_mask,
		                   int  verbose);

#endif
