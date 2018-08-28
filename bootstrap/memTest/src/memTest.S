/*!@file   memTest.S
*
* @brief   A bootable program to test memory
*
*    This file was derived from bootstrap.s in order to make a program,
*    the sole purpose of which is to test memory.
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    August 28, 2011 -- Created
*
* $Revision: 1356 $
*
* @verbatim                    Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

        .file    "memTest.S"

        /* Global Constant Definitions */
#include "datCode.hh"
#include DAT_PUBLIC(bootstrap, common, ppc-asm.h)
#include DAT_PUBLIC(bootstrap, common, params.h)
#include DAT_PUBLIC(bootstrap, common, functionStates.h)

#define FN FN_1


        // External Routine Definitions
        .extern cpuInit
        .extern memTest
        .extern dCacheCfg

        // Global Entry Point Definitions
        .section .text,"ax"
        .global Boot0


        // r28 - Used to save and restore the LR around some subroutine calls
        // r29 - Scratch
        // r30 - Holds the latest state value; upper 16 bits = 0xdead when bugchecked
        // r31 - Address + 4 of a call to bugcheck()

        // Boot0 - Initialization point
Boot0:  li      r28,-1            // Clear out any stale value
        li      r29,0             // Clear out any stale value
        li      r30,-1            // Clear out any stale value
        li      r31,-1            // Clear out any stale value
        State(  FN,FS_1)          // Initial state <--- 1

        bl      cpuInit           // Do processor model specific stuff
        State(  FN,FS_2)          // Next state <--- 2

1:      lis     r5,   (Tb0-4)@h   // Get the start time stamp
        ori     r5,r5,(Tb0-4)@l
        bl      tbRead            // Get the finish time stamp

        // Do a memory test based on the boot flags
        lis     r7,   EXTEND@H    // Top of memory
        ori     r7,r7,EXTEND@L
        lis     r3,   EXTADDR@H   // Start address
        ori     r3,r3,EXTADDR@L
        li      r4,EXTSIZE        // Test block length
        srwi    r4,r4,2           // Convert length to longword count
        slwi    r6,r4,2           // Block increment, in bytes

        bl      memTest           // Test memory: DATA CACHEING MUST BE DISABLED

        lis     r5,   (Tb1-4)@h   // Get the finish time stamp
        ori     r5,r5,(Tb1-4)@l
        bl      tbRead

        // Loop to repeat tests
        addi    r29,r29,1         // Count loops in upper word
        State(  FN,FS_3)          // Next state <--- 3
        b       1b                // Go again


        // Save timebase registers (to measure application performance)
tbRead:
1:      mftbu   r3
        mftbl   r4
        mftbu   r0
        cmpw    r0,r3
        bne-    1b
        stwu    r3,4(r5)
        stw     r4,4(r5)
        blr


/* ---------------------------------------------------------------------
//
// Define space for data items needed by this code
//
//---------------------------------------------------------------------*/

        // Save time base to use for benchmarking numbers.
        .section .data,"aw"
        .global Tb0,TB1
Tb0:
        .quad 0
Tb1:
        .quad 0
