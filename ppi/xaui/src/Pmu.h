// -*-Mode: C;-*-
/**
@file
@brief This is the PUBLIC interface to the PMU facility.

@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
PMU

@author
Sergio Maldonado, SLAC (smaldona@slac.stanford.edu)

@par Date created:
2014/09/16

@par Last commit:
\$Date: $ by \$Author: $.

@par Revision number:
\$Revision: $

@par Credits:
SLAC
*/

#define PMC_ICACHE_FILLS  0x1
#define PMC_ITLB_FILLS    0x2
#define PMC_DCACHE_FILLS  0x3
#define PMC_DCACHE_ACCESS 0x4
#define PMC_DTLB_FILLS    0x5
#define PMC_CPU_CYCLES    0x11
#define PMC_CACHE_MISS    0x50
#define PMC_CACHE_HITS    0x51
#define PMC_ICACHE_STALLS 0x60
#define PMC_DCACHE_STALLS 0x61
#define PMC_TLB_STALLS    0x62
#define PMC_INSTR_RENAME  0x68
#define PMC_INSTR_MAIN    0x70
#define PMC_INSTR_FP      0x73

/*
 Performance Monitoring Unit init
 disable overflow interrupts
 enable all counters
 select counter 0
 select event
 reset counters to zero, enable all counters
*/

#define INIT_PMU() \
      {\
      __asm__ volatile ("MCR p15, 0, %0, c9, c14, 2\t\n" :: "r"(0x8000001f)); \
      __asm__ volatile ("MCR p15, 0, %0, c9, c12, 1\t\n" :: "r"(0x8000001f)); \
      __asm__ volatile ("MCR p15, 0, %0, c9, c12, 5\t\n" :: "r"(0x0));        \
      __asm__ volatile ("MCR p15, 0, %0, c9, c12, 0\t\n" :: "r"(0x7)); \
      }

#define DISABLE_PMU() \
      {\
      __asm__ volatile ("MCR p15, 0, %0, c9, c12, 1\t\n" :: "r"(0x00000000)); \
      __asm__ volatile ("MCR p15, 0, %0, c9, c12, 0\t\n" :: "r"(0x6)); \
      }

#define SET_PMC(_val) \
      __asm__ volatile ("MCR p15, 0, %0, c9, c13, 1\t\n" :: "r"(_val));

#define READ_PMC(_val) \
      __asm__ volatile ("MRC p15, 0, %0, c9, c13, 2\t\n": "=r"(_val));

#define READ_CYCLES(_val) \
      __asm__ volatile ("MRC p15, 0, %0, c9, c13, 0\t\n": "=r"(_val));
      
#define MARK_START(_val) \
      {\
      __asm__ volatile ("isb sy"); \
      __asm__ volatile ("dsb sy"); \
      READ_CYCLES(_val); \
      }

#define MARK_END READ_CYCLES

#define GTC_REGISTER  0xF8F00200

#define READ_GTC(_val) _val = *gtc;

static inline void delay (unsigned long loops)
  {
  __asm__ volatile ("1:\n"
  "subs %0, %0, #1\n"
  "bne 1b":"=r" (loops):"0" (loops));
  }
