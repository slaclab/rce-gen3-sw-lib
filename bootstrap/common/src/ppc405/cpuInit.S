/*!@file      cpuInit.S
*
*  @brief     CPU initialization for the PowerPC 405 processor
*
*  @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
*  @date      October 31, 2011 -- Created
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

        .file   "bootLoader.S"

         // Global Constant Definitions
#include "datCode.hh"
#include DAT_PUBLIC(bootstrap, common, ppc-asm.h)
#include DAT_PUBLIC(bootstrap, common, params.h)
#include DAT_PUBLIC(bootstrap, common, functionStates.h)

#define FN FN_2


        // Global Entry Point Definitions
        .section .text,"ax"
        .global cpuInit

        // This code is derived from the listing given in section 3.4 of
        // the PowerPC 405-S Embedded Processor Core User's Manual,
        // version 1.2 of June 16, 2010.

       /* -----------------------------------------------------------------
        * Setup CPU
        *-----------------------------------------------------------------*/
first:
        State   FN,FS_2           // Update state

        // Clear the Machine State Register's Critical & External interrupt enables
        mfmsr   r3
        lis     r0,   0x00028000@h
        ori     r0,r0,0x00028000@l
        andc    r3,r3,r0
        mtmsr   r3
        sync
        State   FN,FS_3           // Update state

        // Configure the storage guarded register attribute for performance
        lis     r0,0x00000000@h   // Guarded value
        mtsgr   r0

        // Configure endianness, compression
        lis     r0,0x00000000@h   // Endianess value
        mtsler  r0
        lis     r0,0x00000000@h   // Compression value
        mtsu0r  r0

        // Invalidate the instruction cache and enable cachebility
        iccci   r0,r0             // Operand values are ignored
        lis     r0,0x80000000@h   // Cacheability value
        mticcr  r0
        isync
        State   FN,FS_4           // Update state

        // Select whether APU, Wait Enable, interrupts/exceptions and address
        // translation should be enabled when application starts
        mfmsr   r3
        oris    r3,r3,AP@h        // Enable APU
        ori     r3,r3,AP@l
        mtsrr1  r3                // SSR1 is loaded into MSR upon rfi
        mflr    r28
        mr      r23,r28           // Debuging help
        mtsrr0  r28               // Return address
        rfi                       // Context synchronize


        .eject

cpuInit:
        State   FN,FS_1           // Update state

       /*-------------------------------------------------------------------
        * Do initialization up to the point where a context sync is required
        *------------------------------------------------------------------*/
        mflr    r31               // Save return address
        bl      first             // Do first things first

        State   FN,FS_5           // Update state

        // Select where the exception vector table is going to be
        lis     r0,0x0000        // Exception vector prefix
        mtevpr  r0

        // Set up the debug register to freeze timers on debug events
        mfdbcr0 r2
        ori     r2, r2, 0x0001
        mtdbcr0 r2
        isync
        State   FN,FS_6           // Update state

        // Configure timer facilities
        mttbl   r0                // Clear Timebase to prevent Fixed Interval..
        mttbu   r0                // ..timer and Watchdog Timer exceptions
        mtpit   r0                // Programmable interval timer
        li      r2,-1             // -1 to clear TSR
        mttsr   r2                // Timer status register

        // Clear out stale values in certain registers to avoid confusion
        mtcrf   0xff,r0           // Need for simulation
        mtctr   r0                // Counter register
        mtxer   r0                // Fixed-point exception register
        mtesr   r0                // Exception syndrome register
        mtdear  r0                // Data exception address register
        mtmcsr  r0                // Machine check syndrome register

        State   FN,FS_7           // Update state
        mtlr    r31               // Restore return address
        blr                       // Return to main flow


        .eject

        .global dCacheCfg

        // Configure data cache write-through and cacheability
dCacheCfg:
        // Invalidate the data cache
        lis     r3,EXTADDR@h      // Start address
        li      r0,0x100          // Set data cache congruence class count
        mtctr   r0                // Transfer data cache congruence class count to CTR
1:      dccci   r3,r4             // Invalidate a data cache congruence class
        addi    r4,r4,0x20        // Update memory offset
        bdnz    1b                // Loop over data cache congruence class
        State   FN,FS_8           // Update state

        // Set Core Configuration Register 0 as follows:
        // sum: 0x02700E00
        // bit 1 off:   as given by ppc405 errata to avoid CPU_213 ppc bug
        // bit 3 off:   as given by ppc405 errata to avoid CPU_213 ppc bug
        //              (Note added later: PPC405F6 is not subject to CPU_213.)
        // bit 1 on:    Xilinx: CR 203746 Patch for PPC405 errata (RiC 12/8/11)
        // bit 2 on:    Xilinx: CR 203746 Patch for PPC405 errata (RiC 12/8/11)
        // bit 6 on:    load word as line
        // bit 7 off:   load  misses allocate cache line
        // bit 8 off:   store misses allocate cache line
        // bit 9-11 on: default settings to do with plb priority
        // bit 20 on:   prefetching for cacheable regions
        // bit 21 on:   prefetching for non-cacheable regions
        // bit 22 on:   request size of non-cacheable inst fetches is 8 words
        // bit 23 off:  fetch misses allocate cache line
        lis     r3,   0x52700E00@h
        ori     r3,r3,0x52700E00@l

        // To change CCR0 we make sure the code writing to it is running from
	// the I-cache.  This is needed because changing some CCR0 fields will
	// cause a hang if the processor is trying to access memory at the same
        // time.
        lis     r4,   2f@h
        ori     r4,r4,2f@l
        icbt    r0,r4
        b       2f                // Jump over alignment gap

        .align  5                 // New cache line (32 bytes each)

2:      icbt    r0,r4             // Put this line into the I-cache
        isync
        mtccr0  r3
        isync
        b       3f                // Jump over alignment gap

        .align  5                 // Different cache line (32 bytes each)

3:      // Set the Data Cache Write-Through Register for no write-through,
        // i.e., for write-back
        li      r0,0              // Write through value
        mtdcwr  r0

        // Set the Data Cache Cacheablility Register
        lis     r0,0x80000000@h   // Cacheablility value
        mtdccr  r0
        isync

        State   FN,FS_9           // Update state
        blr                       // Return to main flow
