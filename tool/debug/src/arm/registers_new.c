// -*-Mode: C;-*-
/**
@file
@brief Implement register-related functions for ARM.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
DAT

@author
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2014/02/16

@par Last commit:
\$Date: 2014-04-07 21:30:01 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3125 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/debug/src/arm/registers_new.c $

@par Credits:
SLAC
*/

#include "debug/registers.h"

void dbg_printGeneralRegisters(dbg_Registers* gregs, dbg_PrintFunc print) {
  uint32_t* r = &gregs->r0;
  int i;
  for (i = 0; i < 12; ++i) {
    if (i < 10)  print("    r%d:", i);
    else         print("   r%d:", i);
    print(" 0x%08x  ", r[i]);
    if ((i+1) %4 == 0) print("\n");
  }
  print("   r12: 0x%08x      sp: 0x%08x      lr: 0x%08x      pc: 0x%08x\n",
        gregs->r11, gregs->r12, gregs->sp, gregs->lr, gregs->pc);
}

void dbg_printSpecialRegisters(dbg_PrintFunc print) {
  uint32_t v1, v2, v3, v4;
  asm ("mrs %0, cpsr": "=r"(v1));
  asm ("mrc p15, 0, %0, c1, c0, 0": "=r"(v2));
  asm ("mrc p15, 0, %0, c1, c0, 1": "=r"(v3));
  asm ("mrc p15, 0, %0, c3, c0, 0": "=r"(v4));
  print("  cpsr: 0x%08x   sctlr: 0x%08x   actlr: 0x%08x    dacr: 0x%08x\n", v1, v2, v3, v4);
  asm ("mrc p15, 0, %0, c2, c0, 2": "=r"(v1));
  asm ("mrc p15, 0, %0, c2, c0, 0": "=r"(v2));
  asm ("mrc p15, 0, %0, c2, c0, 1": "=r"(v3));
  asm ("mrc p15, 0, %0, c12, c0, 0": "=r"(v4));
  print(" ttbcr: 0x%08x   ttbr0: 0x%08x   ttbr1: 0x%08x    vbar: 0x%08x\n", v1, v2, v3, v4);
  asm ("mrc p15, 0, %0, c5, c0, 1": "=r"(v1));
  asm ("mrc p15, 0, %0, c6, c0, 2": "=r"(v2));
  asm ("mrc p15, 0, %0, c5, c0, 0": "=r"(v3));
  asm ("mrc p15, 0, %0, c6, c0, 0": "=r"(v4));
  print("  ifsr: 0x%08x    ifar: 0x%08x    dfsr: 0x%08x    dfar: 0x%08x\n", v1, v2, v3, v4);
}
