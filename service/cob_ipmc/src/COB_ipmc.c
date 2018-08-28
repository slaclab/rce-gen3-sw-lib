/*------------------------------------------------------------------------*//*!
@file
@brief   Implementation of COB_ipmc.h
*//*-------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "dslutil/Base.h"
#include "cob_ipmc/COB_ipmc.h"
#include "cob_ipmc/impl/COB_util.h"
#include "cob_ipmc/impl/COB_cmds.h"

/*------------------------------------------------------------------------*//*!
  @enum ATCA_MAX_NSLOTS
  @brief Maximum number of slots in an ATCA shelf

  @typedef COB_slot_rce_state
  @struct  COB_slot_rce_state
  @brief   Structure containing bitmasks of RCE state bits

  @typedef COB_slot_rce_status
  @struct  COB_slot_rce_status
  @brief   Structure containing an array of COB_slot_rce_state 
           representing an entire shelf.

  @typedef COB_reset_slot_rce_err
  @struct  COB_reset_slot_rce_err
  @brief   Structure containing the return values of the RCE reset command

*//*-------------------------------------------------------------------------*/


/*------------------------------------------------------------------------*//*!
  @fn COB_slot_rce_status* COB_get_slot_rce_status(const char* shelf,
                                                          int  slot,
					                 void* buf);
  @brief Request the state of the specified RCEs

  @param[in] shelf The IP address or hostname of the Shelf Manager
  @param[in] slot  The slot in the shelf being queried, can be wildcard value
  @param[in] buf   Buffer to hold returned data

  @return A pointer to the COB_slot_rce_status structure if successful,
          NULL otherwise.
*//*-------------------------------------------------------------------------*/

COB_slot_rce_status* COB_get_slot_rce_status(const char* shelf,
					            int  slot,
					           void* buf)
{
   COB_slot_rce_status* status = (COB_slot_rce_status*)buf;

   COB_ctx* ctx;
   int s;
   int bay;
   int rce;

   unsigned int slot_mask;

   uint8_t ipmb_addr;

   COB_get_cmb_gpio_rs gpio;
   COB_bsi_read_rs bsi;
   
   ctx = COB_open(shelf);
   if(NULL == ctx)
   {
      return NULL;
   }

   memset(status, 0, sizeof(COB_slot_rce_status));

   if(slot == SLOT_ALL_WC)
   {
      slot_mask = 0xffff;
   }
   else if((slot <= ATCA_MAX_SLOT) && (slot >= ATCA_MIN_SLOT))
   {
      slot_mask = 1<<(slot-1);
   }
   else 
   {
      return NULL;
   }

   for(slot=ATCA_MIN_SLOT; slot<=ATCA_MAX_SLOT; ++slot)
   {
      ipmb_addr = COB_ipmb_addr(ctx, slot);
      if(0xff == ipmb_addr) break; // If there is no map to slot, we're done
      status->nslots += 1;

      s = slot-1;
      if(!(slot_mask & (1<<s))) continue;

      if(!COB_slot_present(ctx, ipmb_addr)) continue;

      status->slot_valid_mask |= 1<<s;

      for(bay=0; bay<=DTM; ++bay)
      {
	 if(!COB_bay_present(ctx, ipmb_addr, bay)) continue;

	 if(NULL != COB_get_cmb_gpio(ctx, ipmb_addr, bay, &gpio))
	 {
	    status->slot[s].present |= ((gpio.rce_det&0xf) << (bay*4));
	    if(COB_bay_enabled(ctx, ipmb_addr, bay))
	    {
	       status->slot[s].enabled = status->slot[s].present;
	    }
	    status->slot[s].reset |= ((gpio.rce_rst&0xf) << (bay*4));
	    status->slot[s].ready |= ((gpio.rce_rdy&0xf) << (bay*4));
	    status->slot[s].done  |= ((gpio.rce_rdy&0xf) << (bay*4));
	    if(COB_VOK_MASK == gpio.rce_vok) 
	    {
	       status->slot[s].vok |= status->slot[s].present;
	    }
	    
	    for(rce=0; rce<4; ++rce)
	    {
	       if(gpio.rce_rdy & (1<<rce))
	       {
		  if(NULL != COB_bsi_read(ctx, ipmb_addr, bay, rce,
					  RCE_BSI_STATUS_ADDR, 0x01, &bsi))
		  {
		     if((0x00 == bsi.ccode) && (0x00 == bsi.data[0]))
		     {
			status->slot[s].running |= 1<<((bay*4)+rce);
		     }
		  }
	       }
	    }
	 }
      }
   }

   COB_close(ctx);
   return status;
}


/*------------------------------------------------------------------------*//*!
  @fn COB_reset_slot_rce_err* COB_reset_slot_rce(const char* shelf,
  					                int  slot,
					           uint32_t  slot_rce_mask,
					               void* buf)
  @brief Request the reset of the specified RCEs

  @param[in] shelf The IP address or hostname of the Shelf Manager
  @param[in] slot  The slot in the shelf being reset, can be wildcard value
  @param[in] slot_rce_mask Mask of RCEs in slot to reset
  @param[in] buf   Buffer to hold returned data

  @return A pointer to the COB_reset_slot_rce_err structure if successful,
          NULL otherwise.
*//*-------------------------------------------------------------------------*/
COB_reset_slot_rce_err* COB_reset_slot_rce(const char* shelf,
					          int  slot,
					     uint32_t  slot_rce_mask,
					         void* buf)
{
   COB_rce_reset_rs reset_rs;
   COB_reset_slot_rce_err* error = (COB_reset_slot_rce_err*) buf;

   COB_ctx* ctx;
   int s;
   int bay;

   unsigned int slot_mask;
   unsigned int bay_mask;
   unsigned int rce_mask;

   uint8_t ipmb_addr;

   ctx = COB_open(shelf);
   if(NULL == ctx)
   {
      return NULL;
   }

   if(slot == SLOT_ALL_WC)
   {
      slot_mask = 0xffff;
   }
   else if((slot <= ATCA_MAX_SLOT) && (slot >= ATCA_MIN_SLOT))
   {
      slot_mask = 1<<(slot-1);
   }
   else 
   {
      return NULL;
   }

   memset(error, 0, sizeof(COB_reset_slot_rce_err));

   for(slot=ATCA_MIN_SLOT; slot<=ATCA_MAX_SLOT; ++slot)
   {
      ipmb_addr = COB_ipmb_addr(ctx, slot);
      if(0xff == ipmb_addr) break; // If there is no map to slot, we're done
      error->nslots += 1;

      s = slot-1;
      if(!(slot_mask & (1<<s))) continue;

      if(!COB_slot_present(ctx, ipmb_addr)) continue;

      error->slot_valid_mask |= 1<<s;
      
      for(bay=0; bay<=DTM; ++bay)
      {
	 if(!COB_bay_present(ctx, ipmb_addr, bay)) continue;
	 
	 bay_mask = 1<<bay;

	 rce_mask = (slot_rce_mask & (0xf<<(4*bay)))>>(4*bay);

	 printf("%s/%i/%i - Reset RCEs %x\n", shelf, slot, bay, rce_mask);
	 if(NULL != COB_rce_reset(ctx, ipmb_addr, 
				  bay_mask, rce_mask, &reset_rs))
	 {
	    if(reset_rs.ccode)
	    {
	       error->slot[s] |= rce_mask << (bay*4);
	    }
	    else
	    {
	       error->slot[s] |= reset_rs.err_mask << (bay*4);
	    }
	 }
      }
   }

   COB_close(ctx);
   return error;
}


/*------------------------------------------------------------------------*//*!
  @fn void COB_reset_rce(const char* shelf,
                                int  slot,
                                int  bay,
                                int  rce);

  @brief Request the state of the specified RCEs

  @param[in] shelf The IP address or hostname of the Shelf Manager
  @param[in] slot  The slot in the shelf being reset, can be wildcard value
  @param[in] bay   The bay in the shelf being reset, can be wildcard value
  @param[in] rce   The rce in the shelf being reset, can be wildcard value
  @param[in] buf   Buffer to hold returned data

  @return A pointer to the COB_reset_slot_rce_err structure if successful,
          NULL otherwise.

*//*-------------------------------------------------------------------------*/
COB_reset_slot_rce_err* COB_reset_rce(const char* shelf,
				             int  slot,
				             int  bay,
				             int  rce,
				            void* buf)
{
   uint32_t rce_mask;
   uint32_t slot_rce_mask = 0;
   int b;

   if(rce == ELEM_ALL_WC)
   {
      rce_mask = COB_BAY_ALL_RCES_MASK;
   }
   else if(rce < COB_BAY_MAX_RCES)
   {
      rce_mask = 1<<rce;
   }
   else
   {
      return NULL;
   }

   if(bay == CMB_ALL_WC)
   {
      for(b=DPM0; b<=DTM; ++b)
      {
	 slot_rce_mask |= rce_mask<<(COB_BAY_MAX_RCES*b);
      }
   }
   else if(bay == CMB_DPM_WC)
   {
      for(b=DPM0; b<DTM; ++b)
      {
	 slot_rce_mask |= rce_mask<<(COB_BAY_MAX_RCES*b);
      }
   }
   else if(bay == CMB_DTM_WC)
   {
      slot_rce_mask = rce_mask<<(COB_BAY_MAX_RCES*DTM);
   }
   else if(bay<= DTM)
   {
      slot_rce_mask |= rce_mask<<(COB_BAY_MAX_RCES*bay);
   }
   else
   {
      return NULL;
   }

   return COB_reset_slot_rce(shelf, slot, slot_rce_mask, buf);
}
