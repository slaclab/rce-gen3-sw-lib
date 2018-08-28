// -*-Mode: C;-*-
/*!
*
* @brief   Functions for setting the boot mode
*
* @author  S. Maldonado - (smaldona@slac.stanford.edu)
*
* @date    April 22, 2014 -- Created
*
* $Revision: 2769 $
*
* @verbatim
*                               Copyright 2015
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef _BOOTMODE_H_
#define _BOOTMODE_H_

#if defined(__cplusplus)
extern "C" {
#endif

static const char LOADBIT0[] = "loadbit=0";
static const char LOADBIT1[] = "loadbit=1";
static const char LINUX[]    = "modeboot=sdboot_linux";
static const char RTEMS[]    = "modeboot=sdboot_rtems";
static const char RAMDISK[]  = "modeboot=sdboot_rdisk";

/** @brief

These are the available boot mode values
*/

typedef enum {
   BOOT_RTEMS      =  0,      // RTEMS OS boot mode
   BOOT_LINUX      =  1,      // LINUX OS boot mode
   BOOT_RAMDISK    =  2,      // RAMDISK OS boot mode
 } BOOT_MODES;

int set_bootmode( const char *modeboot, const char *loadbit);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif
