// -*-Mode: C;-*-
/*!
*
* @brief   Functions for bootstrapping the core
*
* @author  S. Maldonado - (smaldona@slac.stanford.edu)
*
* @date    Feb 18, 2014 -- Created
*
* $Revision: 2769 $
*
* @verbatim
*                               Copyright 2014
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef _PLATFORM_STARTUP_INIT_H_
#define _PLATFORM_STARTUP_INIT_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "bsi/Bsi_Cfg.h"       
#include "bsi/Bsi.h"           
#include "map/Lookup.h"        

#define PRINT_BOOT_STATE(state) printk("BOOTSTATE %d\n",state)

/* #define DEBUG */

#ifdef DEBUG
#define BOOT_STATE(state) BsiWrite32(LookupBsi(), BSI_BOOT_RESPONSE_OFFSET, state); \
                          PRINT_BOOT_STATE(state)
#else
#define BOOT_STATE(state) BsiWrite32(LookupBsi(), BSI_BOOT_RESPONSE_OFFSET, state)
#endif

/** @brief

These are the available boot state values
*/

typedef enum {
   //RTEMS INIT TASK STATES
   BOOT_SUCCESS            =  BSI_BOOT_RESPONSE_SUCCESS,
                                       // system completed boot successfully
   BOOT_DSLEXE             =   8,      // dsl exe started
   BOOT_DSLPREFS           =  12,      // dsl exe preferences loaded

   BOOT_STARTSVC           =  16,      // startup services started
   BOOT_LINKTRAIN          =  18,      // network phy link training
   BOOT_STARTPREFS         =  20,      // startup service preferences loaded
   BOOT_APPSVT             =  24,      // application svt installed
   BOOT_SYSSVT             =  26,      // system svt installed
   BOOT_CFGNAMESPACE       =  30,      // confg namespace installed
   BOOT_SYSNAMESPACE       =  32,      // system namespace installed
   BOOT_APP_SDMOUNT        =  36,      // application sd partition mounted
   BOOT_SYS_SDMOUNT        =  38,      // system sd partition mounted
   BOOT_SDINIT             =  40,      // sd driver initialized
   BOOT_RTEMSLDR           =  42,      // Ldr_Install_Rtems completed
   BOOT_DBGINIT            =  44,      // Debug package init completed
   
   //RTEMS BOOT STATES
   BOOT_WRAPPOSTDRVHOOK    =  48,      // __wrap_bsp_postdriver_hook executed
   BOOT_POSTDRVHOOK        =  52,      // brd_postdriver_hook executed
   BOOT_WRAPPREDRVHOOK     =  56,      // __wrap_bsp_predriver_hook executed
   BOOT_PREDRVHOOK         =  60,      // brd_predriver_hook executed
   BOOT_WRAPPRETASKHOOK    =  64,      // __wrap_bsp_pretasking_hook executed
   BOOT_PRETASKHOOK        =  68,      // brd_pretasking_hook executed
   BOOT_WRAPBSPSTART       =  72,      // __wrap_bsp_start executed
   BOOT_BRDSTART           =  76,      // brd_start executed
   BOOT_BSPSTARTHOOK1      =  80,      // bsp_start_hook_1 executed
   BOOT_BSPSTARTHOOK0      =  84,      // bsp_start_hook_0 executed
 } BOOT_STATES;

#if defined(__cplusplus)
} // extern "C"
#endif

#endif
