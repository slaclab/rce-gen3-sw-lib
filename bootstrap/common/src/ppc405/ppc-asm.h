//-----------------------------------------------------------------------------
// pkt_proc
//-----------------------------------------------------------------------------
//
//     XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
//     SOLELY FOR USE IN DEVELOPING PROGRAMS AND SOLUTIONS FOR
//     XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION
//     AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE, APPLICATION
//     OR STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS
//     IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
//     AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
//     FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
//     WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
//     IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
//     REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
//     INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//     FOR A PARTICULAR PURPOSE.
//
//     (c) Copyright 2002 Xilinx, Inc.
//     All rights reserved.
//
//----------------------------------------------------------------------------

#if !defined(__WINNT__)
#define R0      0
#define r0      0
#define SP      1
#define sp      1
#define R1      1
#define r1      1
#define TOC     2
#define toc     2
#define R2      2
#define r2      2
#define R3      3
#define r3      3
#define R4      4
#define r4      4
#define R5      5
#define r5      5
#define R6      6
#define r6      6
#define R7      7
#define r7      7
#define R8      8
#define r8      8
#define R9      9
#define r9      9
#define R10     10
#define r10     10
#define R11     11
#define r11     11
#define R12     12
#define r12     12
#define R13     13
#define r13     13
#define R14     14
#define r14     14
#define R15     15
#define r15     15
#define R16     16
#define r16     16
#define R17     17
#define r17     17
#define R18     18
#define r18     18
#define R19     19
#define r19     19
#define R20     20
#define r20     20
#define R21     21
#define r21     21
#define R22     22
#define r22     22
#define R23     23
#define r23     23
#define R24     24
#define r24     24
#define R25     25
#define r25     25
#define R26     26
#define r26     26
#define R27     27
#define r27     27
#define R28     28
#define r28     28
#define R29     29
#define r29     29
#define R30     30
#define r30     30
#define R31     31
#define r31     31

#define CR0     0
#define cr0     0
#define CR1     1
#define cr1     1
#define CR2     2
#define cr2     2
#define CR3     3
#define cr3     3
#define CR4     4
#define cr4     4
#define CR5     5
#define cr5     5
#define CR6     6
#define cr6     6
#define CR7     7
#define cr7     7

#define F0      0
#define F1      1
#define F2      2
#define F3      3
#define F4      4
#define F5      5
#define F6      6
#define F7      7
#define F8      8
#define F9      9
#define F10     10
#define F11     11
#define F12     12
#define F13     13
#define F14     14
#define F15     15
#define F16     16
#define F17     17
#define F18     18
#define F19     19
#define F20     20
#define F21     21
#define F22     22
#define F23     23
#define F24     24
#define F25     25
#define F26     26
#define F27     27
#define F28     28
#define F29     29
#define F30     30
#define F31     31


/*-----------------------------------------------------------------------------+
| Special Purpose Registers  (mtspr/mfspr)
+-----------------------------------------------------------------------------*/

#define    CCR0         0x3b3      /*  core configuration register 0  */
#define    CTR          0x009      /*  count register                 */
#define    DAC1         0x3f6      /*  data address compare register 1*/
#define    DAC2         0x3F7      /*  data address compare register 2*/
#define    DBCR0        0x3f2      /*  debug control register 0       */
#define    DBCR1        0x3bd      /*  debug control register 1       */
#define    DBSR         0x3f0      /*  debug status register          */
#define    DCCR         0x3fa      /*  data cache cacheability reg    */
#define    DCWR         0x3ba      /*  data cache write-thru register */
#define    DEAR         0x3d5      /*  data error address register    */
#define    DVC1         0x3b6      /*  data cache compare 1           */
#define    DVC2         0x3b7      /*  data cache compare 2           */
#define    ESR          0x3d4      /*  exception syndrome register    */
#define    EVPR         0x3d6      /*  exception vector prefix reg    */
#define    IAC1         0x3f4      /*  inst. address compare reg 1    */
#define    IAC2         0x3f5      /*  inst. address compare reg 2    */
#define    IAC3         0x3B4      /*  inst. address compare reg 3    */
#define    IAC4         0x3B5      /*  inst. address compare reg 4    */
#define    ICCR         0x3fb      /*  inst. cache cacheability reg   */
#define    ICDBDR       0x3d3      /*  inst. cache debug data reg     */
#define    LR           0x008      /*  link register                  */
#define    PID          0x3b1      /*  process id                     */
#define    PIT          0x3db      /*  programmable interval timer    */
#define    PVR          0x11f      /*  processor version              */
#define    SGR          0x3b9      /*  storage guarded register       */
#define    SLER         0x3bb      /*  storage little endian register */
#define    SPRG0        0x110      /*  special general reg 0          */
#define    SPRG1        0x111      /*  special general reg 1          */
#define    SPRG2        0x112      /*  special general reg 2          */
#define    SPRG3        0x113      /*  special general reg 3          */
#define    SPRG4_R      0x104      /*  special general reg 4 read-only*/
#define    SPRG4_W      0x114      /*  special general reg 4 write-only*/
#define    SPRG5_R      0x105      /*  special general reg 5 read-only*/
#define    SPRG5_W      0x115      /*  special general reg 5 write-only*/
#define    SPRG6_R      0x106      /*  special general reg 6 read-only*/
#define    SPRG6_W      0x116      /*  special general reg 6 write-only*/
#define    SPRG7_R      0x107      /*  special general reg 7 read-only*/
#define    SPRG7_W      0x117      /*  special general reg 7 write-only*/
#define    SRR0         0x01a      /*  save/restore register 0        */
#define    SRR1         0x01b      /*  save/restore register 1        */
#define    SRR2         0x3de      /*  save/restore register 2        */
#define    SRR3         0x3df      /*  save/restore register 3        */
#define    SU0R         0x3bc      /*  storage user-defined 0 register*/
#define    TBL_R        0x10c      /*  time base lower read-only      */
                                   /*  user mode access via mftb inst.*/
#define    TBL_W        0x11c      /*  time base lower                */
#define    TBU_R        0x10d      /*  time base upper read-only      */
                                   /*  user mode access via mftb inst.*/
#define    TBU_W        0x11d      /*  time base upper                */
#define    TCR          0x3da      /*  timer control register         */
#define    TSR          0x3d8      /*  timer status register          */
#define    XER          0x001      /*  fixed point exception register */
#define    ZPR          0x3b0      /*  zone protection register       */

/*-----------------------------------------------------------------------------+
| Machine State Register     msr     Bit Masks
+-----------------------------------------------------------------------------*/
#define    AP           0x2000000  /* Auxillary processor available   */
#define    APE          0x80000    /* Auxillary processor exeption en */
#define    WE           0x40000    /* Activates power management      */
#define    CE           0x20000    /* Critical interrupt inable       */
#define    EE           0x8000     /* external interrupt enable       */
#define    PR           0x4000     /* privelege level                 */
#define    FP           0x2000     /* floating point available        */
#define    ME           0x1000     /* machine check enable            */
#define    FE0          0x0800     /* floating  point exception mode0 */
#define    DWE          0x0400     /* debug wait enable               */
#define    DE           0x0200     /* debug exception enable          */
#define    FE1          0x0100     /* floating  point exception mode1 */
#define    IR           0x0020     /* instruction relocate            */
#define    DR           0x0010     /* data relocate                   */
#define    MSR_EE       0x8000     /* external interrupt              */
#define    MSR_PR       0x4000     /* privelege level                 */
#define    MSR_FP       0x2000     /* floating point available        */
#define    MSR_ME       0x1000     /* machine check                   */
#define    MSR_FE0      0x0800     /* floating point exception enable */
#define    MSR_DWE      0x0400     /* debug wait enable               */
#define    MSR_DE       0x0200     /* debug exception enable          */
#define    MSR_FE1      0x0100     /* floating point exception enable */
#define    MSR_IR       0x0020     /* instruction relocate            */
#define    MSR_DR       0x0010     /* data relocate                   */

#endif

/*
 * Macros to glue together two tokens.
 */

#ifdef __STDC__
#define XGLUE(a,b) a##b
#else
#define XGLUE(a,b) a/**/b
#endif

#define GLUE(a,b) XGLUE(a,b)

/*
 * Macros to begin and end a function written in assembler.  If -mcall-aixdesc
 * or -mcall-nt, create a function descriptor with the given name, and create
 * the real function with one or two leading periods respectively.
 */

#ifdef _RELOCATABLE
#define DESC_SECTION ".got2"
#else
#define DESC_SECTION ".got1"
#endif

#if defined(_CALL_AIXDESC)
#define FUNC_NAME(name) GLUE(.,name)
#define FUNC_START(name) \
     .section DESC_SECTION,"aw"; \
name: \
     .long GLUE(.,name); \
     .long _GLOBAL_OFFSET_TABLE_; \
     .long 0; \
     .previous; \
     .type GLUE(.,name),@function; \
     .globl name; \
     .globl GLUE(.,name); \
GLUE(.,name):

#define FUNC_END(name) \
GLUE(.L,name): \
     .size GLUE(.,name),GLUE(.L,name)-GLUE(.,name)

#elif defined(__WINNT__)
#define FUNC_NAME(name) GLUE(..,name)
#define FUNC_START(name) \
     .pdata; \
     .align 2; \
     .ualong GLUE(..,name),GLUE(name,.e),0,0,GLUE(..,name); \
     .reldata; \
name: \
     .ualong GLUE(..,name),.toc; \
     .section .text; \
     .globl name; \
     .globl GLUE(..,name); \
GLUE(..,name):

#define FUNC_END(name) \
GLUE(name,.e): ; \
GLUE(FE_MOT_RESVD..,name):

#elif defined(_CALL_NT)
#define FUNC_NAME(name) GLUE(..,name)
#define FUNC_START(name) \
     .section DESC_SECTION,"aw"; \
name: \
     .long GLUE(..,name); \
     .long _GLOBAL_OFFSET_TABLE_; \
     .previous; \
     .type GLUE(..,name),@function; \
     .globl name; \
     .globl GLUE(..,name); \
GLUE(..,name):

#define FUNC_END(name) \
GLUE(.L,name): \
     .size GLUE(..,name),GLUE(.L,name)-GLUE(..,name)

#else
#define FUNC_NAME(name) name
#define FUNC_START(name) \
     .type name,@function; \
     .globl name; \
name:

#define FUNC_END(name) \
GLUE(.L,name): \
     .size name,GLUE(.L,name)-name
#endif

