/*!@file      bootLoader.S
*
*  @brief     Common portion of the boot loader for the PowerPC processors.
*
*             This code was developed using the PowerPC 405-S Embedded
*             Processor Core User's Manual, version 1.2 of June 16, 2010,
*             the PPC440x5 CPU Core User's Manual, number SA14-2613-03 of
*             July 15, 2003, and the application note "A Minimal PowerPC
*             Boot Sequence for Executing Compiled C Programs", number
*             AN1809/D of March 2000.
*
*  @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
*  @date      October 31, 2011 -- Created
*
*  $Revision: 1382 $
*
*  @verbatim                    Copyright 2011
*                                      by
*                         The Board of Trustees of the
*                       Leland Stanford Junior University.
*                              All rights reserved.
*  @endverbatim
*/

        .file   "bootLoader.S"

        // Global Constant Definitions
#include "datCode.hh"
#include DAT_PUBLIC(    bootstrap, common, ppc-asm.h)
#include DAT_PUBLIC(    bootstrap, common, params.h)
#include DAT_PUBLIC(    bootstrap, common, functionStates.h)
#include DAT_GENERATION(bootstrap, common, bootOpts.h)
#include DAT_PUBLIC(    bootstrap, common, bugcheck.h)

#define FN FN_1


        // External Routine Definitions
        .extern readTB
        .extern cpuInit
        .extern imageTest
        .extern imagePrepare
        .extern imageLoad
        .extern memTest
        .extern dCacheCfg

        // Global Entry Point Definitions
        .section .text,"ax"
        .global Boot0


        // This code is designed to accomodate warm restarts, in which the
        // application software decides to restart the system by branching
        // to the following code (either Boot or Boot0) without causing
        // one of the hardware resets: core, chip or system (table 3-1 in
        // the Power PC 405-S Embedded Processor Core User's Manual).

        // r3  - If, upon input, r3 is a legitimate address pointing to an ELF
        //       header, it is taken as the load address of the file to boot
        //       from.  In this case, the persistant store is not accessed.
        // r28 - Used to save and restore the LR around some subroutine calls
        // r29 - Scratch
        // r30 - Holds the latest state value; upper 16 bits = 0xdead when bugchecked
        // r31 - Address + 4 of a call to bugcheck()

        // Boot0 - Initialization point
Boot0:  li      r28,-1            // Clear out any stale value
        li      r29,-1            // Clear out any stale value
        li      r30,-1            // Clear out any stale value
        li      r31,-1            // Clear out any stale value
        State   FN,FS_1           // Initial state <--- 1

        mr      r11,r3            // Save the possible ELF load address

        bl      cpuInit           // Do processor model specific stuff
        State   FN,FS_2           // Next state <--- 2

        // Test whether to boot a file already in memory
        bl      imageTest
        State   FN,FS_3           // Next state <--- 3
        beq     1f                // Skip ahead if there is an image in memory

        bl      imagePrepare      // Prepare to load image -> r3 = boot flags
        State   FN,FS_4           // Next state <--- 4

        // Do a memory test based on the boot flags
        lis     r7,EXTEND@H       // Top of memory
        li      r4,EXTSIZE        // Test block length
        srwi    r4,r4,2           // Convert length to longword count
        andi.   r3,r3,1           // Check bit 0 in the boot flags
        slwi    r6,r4,2           // Block increment, in bytes
        bne     2f                // If boot flags bit not set...
        slwi    r6,r6,6           // ...check every 64th block

2:      bl      memTest           // Test memory: DATA CACHEING MUST BE DISABLED
        State   FN,FS_5           // Next state <--- 5

        li      r11,-1            // Remember that image is not yet loaded

        // Configure the data cache
1:      bl      dCacheCfg
        State   FN,FS_6           // Next state <--- 6

        // Load the application file into memory
        bl      imageLoad         // Load the binary image -> r3 = entry point
        State   FN,FS_7           // Next state <--- 7

        // Load the entry point and its first argument (the boot flags)
        mtsrr0  r3                // Entry address
        BootPrm r3                // The boot parameters
        mfmsr   r0                // Ensure srr1 holds a good value
        mtsrr1  r0                // The msr is loaded with srr1 upon rfi

        // Put something into the link register in case the application returns
        lis     r1,   done@h
        ori     r1,r1,done@l
        mtlr    r1

        // Indicate the application is starting
        lis     r30,    0xbabecafe@h
        ori     r30,r30,0xbabecafe@l
        SetDpy  r30

        rfi                       // Enter the (C) image


/*---------------------------------------------------------------------------*/
done:   lis     r30,    0xdeadbabe@h
        ori     r30,r30,0xdeadbabe@l
        SetDpy  r30

        // Go into an infinite loop, bugcheck or branch to the invalid op vector?
        BugChk  bnel              // Done!

