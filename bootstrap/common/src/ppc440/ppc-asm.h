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

#define xer     0x001   /* Integer Exception Register                */
#define lr      0x008   /* Link Register                             */
#define ctr     0x009   /* Count Register                            */
#define pid     0x030   /* Process ID                                */
#define decar   0x036   /* Decrementer Auto-Reload                   */
#define dear    0x03d   /* Data Exception Address Register           */
#define esr     0x03e   /* Exception Syndrome Register               */
#define ivpr    0x03f   /* Interrupt Vector Prefix Register          */
#define sprg4_w 0x104   /* Special Purpose Register General 4 (WO)   */
#define sprg5_w 0x105   /* Special Purpose Register General 5 (WO)   */
#define sprg6_w 0x107   /* Special Purpose Register General 6 (WO)   */
#define sprg7_w 0x108   /* Special Purpose Register General 7 (WO)   */
#define tbl     0x10c   /* Time Base Lower                           */
#define tbu     0x10d   /* Time Base Upper                           */
#define pir     0x11e   /* Processor ID Register                     */
#define pvr     0x11f   /* Processor Version Register                */
#define dbsr    0x130   /* Debug Status Register                     */
#define dbcr0   0x134   /* Debug Control Register 0                  */
#define dbcr1   0x135   /* Debug Control Register 1                  */
#define dbcr2   0x136   /* Debug Control Register 2                  */
#define iac1    0x138   /* Instruction Address Compare 1             */
#define iac2    0x139   /* Instruction Address Compare 2             */
#define iac3    0x13a   /* Instruction Address Compare 3             */
#define iac4    0x13b   /* Instruction Address Compare 4             */
#define dac1    0x13c   /* Data Address Compare 1                    */
#define dac2    0x13d   /* Data Address Compare 2                    */
#define dvc1    0x13e   /* Data Value Compare 1                      */
#define dvc2    0x13f   /* Data Value Compare 2                      */
#define tsr     0x150   /* Timer Status Register                     */
#define tcr     0x154   /* Timer Control Register                    */
#define ivor0   0x190   /* Interrupt Vector Offset Register 0        */
#define ivor1   0x191   /* Interrupt Vector Offset Register 1        */
#define ivor2   0x192   /* Interrupt Vector Offset Register 2        */
#define ivor3   0x193   /* Interrupt Vector Offset Register 3        */
#define ivor4   0x194   /* Interrupt Vector Offset Register 4        */
#define ivor5   0x195   /* Interrupt Vector Offset Register 5        */
#define ivor6   0x196   /* Interrupt Vector Offset Register 6        */
#define ivor7   0x197   /* Interrupt Vector Offset Register 7        */
#define ivor8   0x198   /* Interrupt Vector Offset Register 8        */
#define ivor9   0x199   /* Interrupt Vector Offset Register 9        */
#define ivor10  0x19a   /* Interrupt Vector Offset Register 10       */
#define ivor11  0x19b   /* Interrupt Vector Offset Register 11       */
#define ivor12  0x19c   /* Interrupt Vector Offset Register 12       */
#define ivor13  0x19d   /* Interrupt Vector Offset Register 13       */
#define ivor14  0x19e   /* Interrupt Vector Offset Register 14       */
#define ivor15  0x19f   /* Interrupt Vector Offset Register 15       */
#define mcsr    0x23c   /* Machine Check Status Register             */
#define inv0    0x370   /* Instruction Cache Normal Victim 0         */
#define inv1    0x371   /* Instruction Cache Normal Victim 1         */
#define inv2    0x372   /* Instruction Cache Normal Victim 2         */
#define inv3    0x373   /* Instruction Cache Normal Victim 3         */
#define itv0    0x374   /* Instruction Cache Transient Victim 0      */
#define itv1    0x375   /* Instruction Cache Transient Victim 1      */
#define itv2    0x376   /* Instruction Cache Transient Victim 2      */
#define itv3    0x377   /* Instruction Cache Transient Victim 3      */
#define ccr1    0x378   /* Core Configuration Register 1             */
#define dnv0    0x390   /* Data Cache Normal Victim 0                */
#define dnv1    0x391   /* Data Cache Normal Victim 1                */
#define dnv2    0x392   /* Data Cache Normal Victim 2                */
#define dnv3    0x393   /* Data Cache Normal Victim 3                */
#define dtv0    0x394   /* Data Cache Transient Victim 0             */
#define dtv1    0x395   /* Data Cache Transient Victim 1             */
#define dtv2    0x396   /* Data Cache Transient Victim 2             */
#define dtv3    0x397   /* Data Cache Transient Victim 3             */
#define dvlim   0x398   /* Data Cache Victim Limit                   */
#define ivlim   0x399   /* Instruction Cache Victim Limit            */
#define rstcfg  0x39b   /* Reset Configuration                       */
#define dcdbtrl 0x39c   /* Data Cache Debug Tag Register Low         */
#define dcdbtrh 0x39d   /* Data Cache Debug Tag Register High        */
#define icdbtrl 0x39e   /* Instruction Cache Debug Tag Register Low  */
#define icdbtrh 0x39f   /* Instruction Cache Debug Tag Register High */
#define mmucr   0x3b2   /* Memory Management Unit Control Register   */
#define ccr0    0x3b3   /* Core Configuration Register 0             */
#define icdbdr  0x3d3   /* Instruction Cache Debug Data Register     */
#define dbdr    0x3f3   /* Debug Data Register                       */

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

