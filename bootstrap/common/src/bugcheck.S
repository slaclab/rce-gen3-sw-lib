/*!@file    bugcheck.h
*
*  @brief   Code used to bugcheck the system
*
*  @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
*  @date    November 9, 2011 -- Created
*
*  $Revision: 1358 $
*
*  @verbatim                    Copyright 2011
*                                      by
*                         The Board of Trustees of the
*                       Leland Stanford Junior University.
*                              All rights reserved.
*  @endverbatim
*/
        .file   "bugcheck.S"

        // Global Constant Definitions
#include "datCode.hh"
#include DAT_PUBLIC(bootstrap, common, ppc-asm.h)
#include DAT_PUBLIC(bootstrap, common, display.h)
#include DAT_PUBLIC(bootstrap, common, bugcheck.h)

        // Global Entry Point Definitions
        .section .text,"ax"

        .global bugcheck

        .eject

        // Error processing
bugcheck:
        lis     r31,0xdead
        GetDpy  r30               // Get the current display value
        or      r30,r30,r31       // Merge 0xdead with display value
        SetDpy  r30               // Update display
        mflr    r31               // Save the caller's address
bugBkPt:
        mfmsr   r29               // Disable processing
        oris    r29,r29,WE@h      // Set wait state mask
        mtmsr   r29               // Disable processing
1:      b       1b                // Loop forever

.LbcUser:
        .size   bugcheck,.LbcUser-bugcheck
