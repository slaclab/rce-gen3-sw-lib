#ifndef COB_IPMC_H
#define COB_IPMC_H

/*------------------------------------------------------------------------*//*!
@file
@brief Definition of external COB IPMC interface
*//*-------------------------------------------------------------------------*/

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif
  
enum {ATCA_MAX_NSLOTS = 16};

typedef struct COB_slot_rce_state
{
   uint32_t present;
   uint32_t enabled;
   uint32_t   reset;
   uint32_t   ready;
   uint32_t    done;
   uint32_t     vok;
   uint32_t running;
} COB_slot_rce_state;

typedef struct COB_slot_rce_status
{
   uint16_t           nslots;
   uint16_t           slot_valid_mask;
   COB_slot_rce_state slot[ATCA_MAX_NSLOTS];
} COB_slot_rce_status;

typedef struct COB_reset_slot_rce_err
{
   uint16_t nslots;
   uint16_t slot_valid_mask;
   uint32_t slot[ATCA_MAX_NSLOTS];
} COB_reset_slot_rce_err;

COB_slot_rce_status* COB_get_slot_rce_status(const char* shelf, 
					            int slot, 
					           void* buf);

COB_reset_slot_rce_err* COB_reset_slot_rce(const char* shelf,
		 			          int  slot,
					     uint32_t  slot_rce_mask,
					         void* buf);

COB_reset_slot_rce_err* COB_reset_rce(const char* shelf,
				             int  slot,
				             int  bay,
				             int  rce,
				            void* buf);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif /* COB_IPMC_H */
