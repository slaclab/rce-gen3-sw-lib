/*!@file      cpuInit.S
*
*  @brief     CPU initialization for the PowerPC 440 processor
*
*  @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
*  @date      October 31, 2011 -- Created
*
*  $Revision: 1389 $
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
#include DAT_PUBLIC(bootstrap, common, functionStates.h)

#define FN FN_2

#define   V_TS_SZ_I    0x0290      // V,TS=0(Inst),SIZE=9,TID=0
#define   V_TS_SZ_D    0x0390      // V,TS=1(Data),SIZE=9,TID=0
#define   WIMG_U_S_0   0x043F      // !(U0-3),!W, I,!M,!G,!E,UX,UW,UR,SX,SW,SR
#define   WIMG_U_S_1   0x003F      // !(U0-3),!W,!I,!M,!G,!E,UX,UW,UR,SX,SW,SR
#define   PAGE_SZ      0x10000000  // 256 MB


        // Global Entry Point Definitions
        .section .text,"ax"
        .global cpuInit

       /*------------------------------------------------------------------
        * This code follows section 3.4 of the PPC440x5 CPU Core User's
        * Manual.  The numbers in the comments refer to the step numbers
        * therein.  Some of the implementation comes from examples provided
        * by Xilinx in their ML510 Reference Designs, e.g.,
        * ml510_bsb1_design_ppc440.  See boot.S in standalone/.
        *------------------------------------------------------------------*/
       /*------------------------------------------------------------------
        * This code is designed to accomodate warm restarts, in which the
        * application software triggers the restart of the system by branching
        * to the following code (either boot or boot1) without causing
        * one of the hardware resets: core, chip, system or JTAG (section
        * 3.2,3 in the Power PC 440-S Embedded Processor Core User's Manual).
        *------------------------------------------------------------------*/

       /* -----------------------------------------------------------------
        * Setup CPU
        *-----------------------------------------------------------------*/
first:
        State   FN,FS_2           // Update state

        li      r0,0              // Clear r0

       /* -------------------------------------------------------------------
        * Initialize the memory system.
        *------------------------------------------------------------------*/
        iccci   r0,r0             // 2. Invalidate instruction cache
        dccci   r0,r0             // 3. Invalidate data cache
        msync                     // 4. Force in-progress data PLB ops to complete

        mfdbcr0 r2                // 5. Disable almost all debug events
        lis     r3,0x8100         // Leave EDM,TRAP up to XMD
        and     r2,r2,r3
        mtdbcr0 r2
        li      r2,-1
        mtdbsr  r2                // 6. Initialize all debug event status

       /* -------------------------------------------------------------------
        * Set Core Configuration Register 0 as follows:
        * sum: 0x00200000
        * bit  1    off  Parity Recovery Enable
        * bit  4    off  Cache Read Parity Enable
        * bit 10    on   Disable Store Gathering
        * bit 11    off  Disable APU Instruction Broadcast
        * bit 16    off  Disable Trace Broadcast
        * bit 17:18 off  Specifies behaviour of icbt,dcbt/dcbtst insts
        * bit 23    off  Force Load/Store Alignment
        * bit 28:29 off  Instruction Cache Speculative Line Count
        * bit 30:31 off  Instruction Cache Speculative Line Threshold
        *            NB: UG200/pg 21: Spec. prefetching must be disabled
        *------------------------------------------------------------------*/
        lis     r2,0x00200000@h   // 7. Set CCR0: DSTG
        mtccr0  r2                // Configure CCR0

        mtspr   ccr1,r0           // 8. Clear CCR1

       /*------------------------------------------------------------------
	* 9. Configure cache regions
        *------------------------------------------------------------------*/
	mtspr   inv0,r0
	mtspr   inv1,r0
	mtspr   inv2,r0
	mtspr   inv3,r0
	mtspr   dnv0,r0
	mtspr   dnv1,r0
	mtspr   dnv2,r0
	mtspr   dnv3,r0
	mtspr   itv0,r0
	mtspr   itv1,r0
	mtspr   itv2,r0
	mtspr   itv3,r0
	mtspr   dtv0,r0
	mtspr   dtv1,r0
	mtspr   dtv2,r0
	mtspr   dtv3,r0

       /*------------------------------------------------------------------
	* Cache victim limits
	* floors 0, ceiling max to use the entire cache -- nothing locked
        *------------------------------------------------------------------*/
	lis	r2,   0x0001f800@h
	ori	r2,r2,0x0001f800@l
	mtspr   ivlim,r2
	mtspr   dvlim,r2

       /*------------------------------------------------------------------
        * Configure instruction and data cache regions:
        * Set up register constants (r6,r7), page index (r5), address
        * variable (r4), EPN_V_TS bits (r3)
        *
        * Word 0 bits:  0xX0000290, 0xX0000390
        * Bits  Field  Inst  Data  Description
        *  0:21  EPN   0-15  0-15  Effective Page Number
        *    22  V     1     1     Valid
        *    23  TS    0     1     Translation Address Space
        * 24:27  SIZE  9     9     Page Size (9 = 256 MB)
        * 38:31  TPAR  0     0     Tag Parity
        * 32:39  TID   0     0     Translation ID (in the MMUCR)
        *
        * Word 1 bits:  0x00000000, 0x00000000
        * Bits  Field  Inst  Data  Description
        *  0:21  RPN   0     0     Real Page Number
        * 22:23  PAR1  0     0     Parity for TLB word 1
        * 28:31  ERPN  0     0     Extended Real Page Number
        *
        * Word 2 bits:  0x0000043f, 0x00000c3f
        * Bits  Field  Inst  Data  Description
        *  0: 1  PAR2  0     0     Parity for TLB word 2
        *    16  U0    0     0     User-Defineable Storage Attribute 0
        *    17  U1    0     0     User-Defineable Storage Attribute 1
        *    18  U2    0     0     User-Defineable Storage Attribute 2
        *    19  U3    0     0     User-Defineable Storage Attribute 3
        *    20  W     0     0     Write-Through
        *    21  I     1     1     Caching Inhibited
        *    22  M     0     0     Memory Coherence Required
        *    23  G     0     0     Guarded
        *    24  E     0     0     Endian
        *    26  UX    1     1     User State Execute Enable
        *    27  UW    1     1     User State Write Enable
        *    28  UR    1     1     User State Read Enable
        *    29  SX    1     1     Supervisor State Execute Enable
        *    30  SW    1     1     Supervisor State Write Enable
        *    31  SR    1     1     Supervisor State Read Enable
        *------------------------------------------------------------------*/

        mtspr   mmucr,r0          // 10a. Clear MMUCR
        li      r7,WIMG_U_S_0     // Word 2: Pages are cache inhibited
        lis     r6,   PAGE_SZ@h   // Page size constant
        ori     r6,r6,PAGE_SZ@l
        li      r5,0              // TLB entry index
        li      r4,0              // Initialize RPN to zero
        mflr    r28               // Save return address
        bl      tlbSetup          // 10b. Set up the TLBs
        mtlr    r28               // Restore return address
        State   FN,FS_3           // Update state

       /*------------------------------------------------------------------
        * Select whether Wait Enable, interrupts/exceptions and which address
        * spaces should be enabled when application starts
        *------------------------------------------------------------------*/
        lis     r3,   0x00000000@h // 10d. MSR[IS]=0 MSR[DS]=0
        ori     r3,r3,0x00000000@l
        mtsrr1  r3
        mtsrr0  r28               // Return address
        mr      r23,r28           // Debuging help
        rfi                       // Context synchronize to invalidate shadow TLB contents


        .eject

cpuInit:
        State   FN,FS_1           // Update state

       /*-------------------------------------------------------------------
        * Do initialization up to the point where a context sync is required
        *------------------------------------------------------------------*/
        mflr    r31               // Save return address
        bl      first             // Do first things first

        State   FN,FS_4           // Update state

       /*------------------------------------------------------------------
        * 11. Select where the interrupt vector table is going to be
        *------------------------------------------------------------------*/
        lis     r1,vect0@h        // Interrupt vector prefix
        mtivpr  r1

       /*------------------------------------------------------------------
        * Set up default exception and interrupt vectors
        *------------------------------------------------------------------*/
        lis      r1,   vect0@h    // Critical Input
        ori      r1,r1,vect0@l
        mtivor0  r1
        lis      r1,   vect1@h    // Machine Check
        ori      r1,r1,vect1@l
        mtivor1  r1
        lis      r1,   vect2@h    // Data Storage
        ori      r1,r1,vect2@l
        mtivor2  r1
        lis      r1,   vect3@h    // Instruction Storage
        ori      r1,r1,vect3@l
        mtivor3  r1
        lis      r1,   vect4@h    // External Input
        ori      r1,r1,vect4@l
        mtivor4  r1
        lis      r1,   vect5@h    // Alignment
        ori      r1,r1,vect5@l
        mtivor5  r1
        lis      r1,   vect6@h    // Program
        ori      r1,r1,vect6@l
        mtivor6  r1
        lis      r1,   vect7@h    // FPU Unavailable
        ori      r1,r1,vect7@l
        mtivor7  r1
        lis      r1,   vect8@h    // System Call
        ori      r1,r1,vect8@l
        mtivor8  r1
        lis      r1,   vect9@h    // APU Unavailable
        ori      r1,r1,vect9@l
        mtivor9  r1
        lis      r1,   vect10@h   // Decrementer (DEC)
        ori      r1,r1,vect10@l
        mtivor10 r1
        lis      r1,   vect11@h   // Fixed Interval Timer (FIT)
        ori      r1,r1,vect11@l
        mtivor11 r1
        lis      r1,   vect12@h   // Watchdog Timer
        ori      r1,r1,vect12@l
        mtivor12 r1
        lis      r1,   vect13@h   // Data TLB Error
        ori      r1,r1,vect13@l
        mtivor13 r1
        lis      r1,   vect14@h   // Instruction TLB Error
        ori      r1,r1,vect14@l
        mtivor14 r1
        lis      r1,   vect15@h   // Debug
        ori      r1,r1,vect15@l
        mtivor15 r1

       /*------------------------------------------------------------------
        * 12. Configure debug facilities
        *------------------------------------------------------------------*/
        mtdbcr1 r0
        mtdbcr2 r0
        mtiac1  r0
        mtiac2  r0
        mtiac3  r0
        mtiac4  r0
        mtdac1  r0
        mtdac2  r0
        mtdvc1  r0
        mtdvc2  r0
        mfdbcr0 r2                // Freeze timers on debug events
        ori     r2,r2,0x0001
        mtdbcr0 r2
        isync

       /*------------------------------------------------------------------
        * 13. Configure timer facilities
        *------------------------------------------------------------------*/
        mtdec   r0                // Clear Decrementer to prevent exception
        mttbl   r0                // Clear Timebase to prevent Fixed Interval..
        mttbu   r0                // ..Timer and Watchdog Timer exceptions
        mtpit   r0                // Programmable interval timer
        li      r2,-1             // -1 to clear TSR
        mttsr   r2                // Timer status register

       /*------------------------------------------------------------------
        * Clear out stale values in certain registers
        *------------------------------------------------------------------*/
        mtcrf   0xff,r0           // Need for simulation
        mtctr   r0                // Counter register
        mtxer   r0                // Fixed-point exception register
        mtesr   r0                // Exception syndrome register
        mtdear  r0                // Data exception address register
        mtmcsr  r0                // Machine check syndrome register

        State   FN,FS_5           // Update state
        mtlr    r31               // Restore return address
        blr


        .eject

        .global dCacheCfg

       /*------------------------------------------------------------------
        * Configure data cache write-through and cacheability
        *------------------------------------------------------------------*/
dCacheCfg:
        li      r7,WIMG_U_S_1     // Word 2: Pages are not cache inhibited
        lis     r6,   PAGE_SZ@h   // Page size constant
        ori     r6,r6,PAGE_SZ@l
        li      r5,0              // TLB entry index
        li      r4,0              // Initialize RPN to zero
        mflr    r28               // Save return address
        bl      tlbSetup          // Set up the TLBs
        mtlr    r28               // Restore return address
        State   FN,FS_6           // Update state
        mtsrr0  r28
        rfi                       // Context synchronize to invalidate shadow TLB contents


        .eject

       /*------------------------------------------------------------------
        * Set up TLB entries: 2 entries are needed for the same 256MB page
        * one for instruction memory and the other for data memory.
        * (TS bit=0 for instructions)
        *------------------------------------------------------------------*/
tlbSetup:
1:      ori     r3,r4,V_TS_SZ_I   // Fold V_TS_SZ in with EPN=RPN
        tlbwe   r3,r5,0           // Word 0: EPN_V_TS_SZ (Instructions)
        tlbwe   r4,r5,1           // Word 1: RPN_ERPN
        tlbwe   r7,r5,2           // Word 2: WIMG_U_S
        ori     r3,r4,V_TS_SZ_D   // Fold V_TS_SZ in with EPN=RPN
        addi    r5,r5,1           // Next TLB entry
        tlbwe   r3,r5,0           // Word 0: EPN_V_TS_SZ (Data)
        tlbwe   r4,r5,1           // Word 1: RPN_ERPN
        tlbwe   r7,r5,2           // Word 2: WIMG_U_S
        add     r4,r4,r6          // Increment RPN to next 256MB block
        addi    r5,r5,1           // Next TLB entry
        cmpwi   r5,32             // Done yet?
        bne     1b
        State   FN,FS_7           // Update state
        li      r0,0
2:                                // Zero out index 32-63 TLB entries
        tlbwe   r0,r5,0
        tlbwe   r0,r5,1
        tlbwe   r0,r5,2
        addi    r5,r5,1
        cmpwi   r5,64
        bne     2b
        State   FN,FS_8           // Update state
        blr


        .eject

        /* Default exception/interrupt vectors */
        .section .vectors,"ax"

vect0:  b       vect0             // Critical Input
        b       vect0
        b       vect0
        b       vect0
vect1:  b       vect1             // Machine Check
        b       vect1
        b       vect1
        b       vect1
vect2:  b       vect2             // Data Storage
        b       vect2
        b       vect2
        b       vect2
vect3:  b       vect3             // Instruction Storage
        b       vect3
        b       vect3
        b       vect3
vect4:  b       vect4             // External Input
        b       vect4
        b       vect4
        b       vect4
vect5:  b       vect5             // Alignment
        b       vect5
        b       vect5
        b       vect5
vect6:  b       vect6             // Program
        b       vect6
        b       vect6
        b       vect6
vect7:  b       vect7             // FPU Unavailable
        b       vect7
        b       vect7
        b       vect7
vect8:  b       vect8             // System Call
        b       vect8
        b       vect8
        b       vect8
vect9:  b       vect9             // APU Unavailable
        b       vect9
        b       vect9
        b       vect9
vect10: b       vect10            // Decrementer (DEC)
        b       vect10
        b       vect10
        b       vect10
vect11: b       vect11            // Fixed Interval Timer (FIT)
        b       vect11
        b       vect11
        b       vect11
vect12: b       vect12            // Watchdog Timer
        b       vect12
        b       vect12
        b       vect12
vect13: b       vect13            // Data TLB Error
        b       vect13
        b       vect13
        b       vect13
vect14: b       vect14            // Instruction TLB Error
        b       vect14
        b       vect14
        b       vect14
vect15: b       vect15            // Debug
        b       vect15
        b       vect15
        b       vect15
