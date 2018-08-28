/*!@file      timebase.S
*
*  @brief     Timebase general functions.
*
*  @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
*  @date      August 20, 2012 -- Created
*
*  $Revision: 1384 $
*
*  @verbatim                    Copyright 2012
*                                      by
*                         The Board of Trustees of the
*                       Leland Stanford Junior University.
*                              All rights reserved.
*  @endverbatim
*/

        .file   "timebase.S"

        // Global Constant Definitions
#include "datCode.hh"
#include DAT_PUBLIC(bootstrap, common, ppc-asm.h)



        // External Routine Definitions
        .extern readTB

        // Global Entry Point Definitions
        .section .text,"ax"

        // Function to read the time base register and save the result
        //
        // r3 - Address of a quadword location to store the timebase value
        .global readTB
readTB:
1:      mftbu   r0                // Upper word
        mftbl   r1                // Lower word
        mftbu   r2                // Upper word again
        cmplw   r2,r0             // Was there a roll-over?
        bne-    1b                // Retry, if so
        stw     r0,0(r3)          // Store the result where asked
        stw     r1,4(r3)
        blr
