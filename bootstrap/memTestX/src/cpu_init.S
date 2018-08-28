/*!@file      cpu_init.S
*
*  @brief     Common portion of the boot loader for the PowerPC processors.
*
*  @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
*  @date      October 31, 2011 -- Created
*
*  $Revision: 1356 $
*
*  @verbatim                    Copyright 2011
*                                      by
*                         The Board of Trustees of the
*                       Leland Stanford Junior University.
*                              All rights reserved.
*  @endverbatim
*/

        .file   "cpu_init.S"

        // Global Constant Definitions
#include "datCode.hh"
#include DAT_PUBLIC(bootstrap, common, ppc-asm.h)

#define FN FN_1

        // Global Entry Point Definitions
        .section .text,"ax"
        .global __cpu_init
        .global cpuInit


        // External Routine Definitions
        .extern cpuInit

        .eject

        // __cpu_init -- CPU specific initialization. Invoked from main CRT
__cpu_init:
        b       cpuInit
