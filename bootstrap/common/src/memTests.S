/*!@file   memTests.S
*
* @brief   A collection of 3 memory tests
*
*    This code tests a memory block provided by the caller with patterns
*    of zeros, ones and an incrementing value.
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    October 28, 2011 -- Created
*
* $Revision: 1358 $
*
* @verbatim                    Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

        .file   "memTests.S"

#include "datCode.hh"
#include DAT_PUBLIC(bootstrap, common, ppc-asm.h)
#include DAT_PUBLIC(bootstrap, common, dcr.h)
#include DAT_PUBLIC(bootstrap, common, bugcheck.h)
#include DAT_PUBLIC(bootstrap, common, functionStates.h)
#include DAT_PUBLIC(bootstrap, common, memory.h)

#define FN FN_6


        .section .text,"ax"

        .global memTest


        // A call to memTest should have the following registers set:
        //    r3  Start address of the memory to test
        //    r4  Length of memory test block (longwords)
        //    r6  Block size, in bytes
        //    r7  End address of the memory to test
        // These registers are clobbered by memTest::
        //    r0  General purpose
        //    r5  Location (patch address, memory block) pointer
        //    r8  Address of the load instruction to patch
        //    r9  Address of the condition code setting instruction to patch
        //    r10 Mask for testing double bit errors
        //    r11 Used to save caller's return address
        //    r26 0x00000000
        //    r27 0xffffffff
        //    r28 Temporary storage for the link register

        // Global Entry Point Definitions
memTest:
        mflr    r28               // Save the return address
        mr      r12,r3            // Save the start address
        State   FN,FS_1           // Initial state <--- 1

        // Test with zeros
        li      r26,0             // A source of zeros
blk0:   la      r5,-4(r3)         // Offset starting address
        mtctr   r4                // Transfer word count to CTR
1:      stwu    r26,4(r5)         // Clear next word
        bdnz+   1b                // Loop over words
        State   FN,FS_2           // Next state <--- 2
        la      r5,-4(r3)         // Offset starting address
        mtctr   r4                // Transfer word count to CTR
        MemStRst r26,r10          // Clear the memory status register
2:      lwzu    r0,4(r5)          // Get next word
        mr.     r0,r0             // Check next word
        BugChk  bnel              // Bugcheck if word non-zero
        bdnz+   2b                // Loop over words
        State   FN,FS_3           // Next state <--- 3
        MemStTst r26,r10          // Bugcheck on bad RLDRAM read error status
        add     r3,r3,r6          // Increment for the next block
        cmplw   r3,r7             // Are we at the end?
        blt+    blk0              // Branch to next block
        State   FN,FS_4           // Next state <--- 4

        // Test with ones
        mr      r3,r12
        li      r27,-1            // A source of ones
blk1:   la      r5,-4(r3)         // Offset starting address
        mtctr   r4                // Transfer word count to CTR
1:      stwu    r27,4(r5)         // Store 1s in next word
        bdnz+   1b                // Loop over words
        State   FN,FS_5           // Next state <--- 5
        la      r5,-4(r3)         // Offset starting address
        mtctr   r4                // Transfer word count to CTR
        MemStRst r26,r10          // Clear the memory status register
2:      lwzu    r0,4(r5)          // Get next word
        not.    r0,r0             // Check next word
        BugChk  bnel              // Bugcheck if word non-zero
        bdnz+   2b                // Loop over words
        State   FN,FS_6           // Next state <--- 6
        MemStTst r26,r10          // Bugcheck on bad RLDRAM read error status
        add     r3,r3,r6          // Increment for the next block
        cmplw   r3,r7             // Are we at the end?
        blt+    blk1              // Branch to next block
        State   FN,FS_7           // Next state <--- 7

        // Test with addresses
        mr      r3,r12
blkA:   la      r5,-4(r3)         // Offset starting address
        mtctr   r4                // Transfer word count to CTR
1:      addi    r5,r5,4           // Prepare address to store
        stw     r5,0(r5)          // Store address at address
        bdnz+   1b                // Loop over words
        State   FN,FS_8           // Next state <--- 8
        la      r5,-4(r3)         // Offset starting address
        mtctr   r4                // Transfer word count to CTR
        MemStRst r26,r10          // Clear the memory status register
2:      lwzu    r0,4(r5)          // Get next word
        cmplw   r0,r5             // Check next word
        BugChk  bnel              // Bugcheck if word non-zero
        bdnz+   2b                // Loop over words
        State   FN,FS_9           // Next state <--- 9
        MemStTst r26,r10          // Bugcheck on bad RLDRAM read error status
        add     r3,r3,r6          // Increment for the next block
        cmplw   r3,r7             // Are we at the end?
        blt+    blkA              // Branch to next block
        State   FN,FS_10          // Next state <--- 10
        mtlr    r28               // Restore the return address
        blr                       // Return to caller
