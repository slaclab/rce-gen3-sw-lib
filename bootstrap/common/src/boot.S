/*!@file      boot.S
*
*  @brief     Boot vector and version storage location
*
*  @author    Anonymous
*
*  @date      March 21, 2003 -- Created
*
*  $Revision: 1385 $
*
*  @verbatim                    Copyright 2011
*                                      by
*                         The Board of Trustees of the
*                       Leland Stanford Junior University.
*                              All rights reserved.
*  @endverbatim
*/

        .file   "boot.S"

        // Global Constant Definitions
#include "datCode.hh"
#include DAT_PUBLIC(    bootstrap, common, ppc-asm.h)
#include DAT_GENERATION(bootstrap, common, bootOpts.h)
#include DAT_PUBLIC(    bootstrap, common, bugcheck.h)

        .extern Boot0

/*---------------------------------------------------------------------------*/

        //! Boot - Reset vector
        .section .boot,"ax"
        .global Boot

Boot:
        b       Boot0             // 1. Branch to initialization sequence

.Lboot:
        .size   Boot,.Lboot-Boot


#if !tgt_board_rceG1 && !tgt_board_cmG1
/*---------------------------------------------------------------------------*/
        //! restart - Specify boot restart options and reboot processor
        .section .text,"ax"
        .global restart

restart:
        BootOpt r3                // Load restart options
        lis     r3,0x3000         // Set reboot options
        mtdbcr0 r3                // Reboot the system
        BugChk  bnel              // (Hopefully meaningless) bugcheck

/*---------------------------------------------------------------------------*/
#ifndef BLD_DATE
#define BLD_DATE __DATE__" "__TIME__
#endif

#ifndef BLD_REVISION
#define BLD_REVISION "$Revision: 1385 $"
#endif

        //! Build - Build information
        .section .build,"a"
Build:
        .align  6
        .ascii  BLD_DATE
        .align  5
        .ascii  BLD_REVISION
#endif

        //! Version - Version number
        .section .version,"a"
Version:
        .long   version

.Lversion:
        .size   Version,.Lversion-Version
