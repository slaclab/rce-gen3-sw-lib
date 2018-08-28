#ifndef COB_DUMP_H
#define COB_DUMP_H

#include <stdint.h>

typedef int (*COB_dump_cb)(const char* shelf,
			     uint16_t  slot_mask,
			      uint8_t  bay_mask,
			      uint8_t  rce_mask,
			          int  verbose);

int COB_dump_ipmc(const char* shelf, 
		    uint16_t  slot_mask,
		     uint8_t  bay_mask,
		     uint8_t  rce_mask,
		         int  verbose);

int COB_dump_bay(const char* shelf, 
		   uint16_t  slot_mask,
		    uint8_t  bay_mask,
		    uint8_t  rce_mask,
		        int  verbose);

int COB_dump_rce(const char* shelf, 
		   uint16_t  slot_mask,
		    uint8_t  bay_mask,
		    uint8_t  rce_mask,
		        int  verbose);

int COB_dump_sfp(const char* shelf, 
		    uint16_t slot_mask, 
		    uint8_t  bay_mask,
		    uint8_t  rce_mask,
		        int  verbose);

int COB_dump_bay_gpio(const char* shelf, 
		        uint16_t  slot_mask,
		         uint8_t  bay_mask,
		         uint8_t  rce_mask,
		             int  verbose);

int COB_dump_rtm(const char* shelf, 
		   uint16_t  slot_mask, 
		    uint8_t  bay_mask,
		    uint8_t  rce_mask,
		        int  verbose);

int COB_dump_cen(const char* shelf, 
		   uint16_t  slot_mask, 
		    uint8_t  bay_mask,
		    uint8_t  rce_mask,
		        int  verbose);

#endif
