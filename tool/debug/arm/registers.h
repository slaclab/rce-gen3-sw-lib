// -*-Mode: C;-*-
/**
@file
@brief Declare register-handling functions for ARM.
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
\$Date: 2014-06-04 17:27:22 -0700 (Wed, 04 Jun 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3344 $

@par Credits:
SLAC
*/

#if !defined(TOOL_DEBUG_REGISTERS_H)
#error Do not include this header directly but use the generic registers.h in this package.
#endif
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

  /** @brief A fnction that is used to do the actual printing. */
  typedef void (*dbg_PrintFunc)(const char*, ...);


  /** @brief General registers as stored by popGeneralRegisters(). */
  typedef struct {
    uint32_t sp;
    uint32_t lr;
    uint32_t pc;
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t r12;
  } dbg_Registers;

  /** @brief Save all general registers on the stack. One should normally
      call popGeneralRegisters() immediately afterward.

  */
  static void dbg_pushGeneralRegisters(void)                 __attribute__((always_inline));

  /** @brief Pop into a dbg_Registers the register values just saved
      by pushGeneralRegisters().

      The value of gregs->sp and gregs->pc are adjusted to reflect the register
      values just before dbg_pushGeneralRegisters() was called.
  */
  static void dbg_popGeneralRegisters(dbg_Registers* gregs)  __attribute__((always_inline));

  /** @brief Print saved general registers in a standard format.

      Example:
      @verbatim
      r0  = 0x00000004  r1  = 0x00000001  r2  = 0x0000007b  r3  = 0x00000001
      r4  = 0x1fd32fdc  r5  = 0x00000001  r6  = 0x00000000  r7  = 0x00000000
      r8  = 0x00000000  r9  = 0x00000000  r10 = 0x00000000
      r11 = 0x0044affc  r12 = 0x1fd31fb0  sp  = 0x0044af30  lr  = 0x1fd31ecc  pc = 0x1fd31fe4
      @endverbatim

  */
  void dbg_printGeneralRegisters(dbg_Registers* gregs, dbg_PrintFunc print);

  /** @brief Print the values of a selection of special registers. */
  void dbg_printSpecialRegisters(dbg_PrintFunc print);




  static inline void dbg_pushGeneralRegisters(void) {
    // We need two stm instructions because when the save-list
    // contains rn when rn is also the register being modified by
    // stm, then rn must be the lowest numbered register in the
    // save-list.
    asm volatile
      ("stmdb sp!, {r0-r12}  \n\t"
       "stmdb sp!, {sp, lr, pc}"
       ::: "sp", "memory"
       );
  }

  static inline void dbg_popGeneralRegisters(dbg_Registers* gregs) {
    asm volatile
      ("mov r1, %[gregs]  \n\t"
       "ldr r0, [sp], #4  \n\t"
       "str r0, [r1], #4  \n\t"
       "ldr r0, [sp], #4  \n\t"
       "str r0, [r1], #4  \n\t"
       "ldr r0, [sp], #4  \n\t"
       "str r0, [r1], #4  \n\t"
       "ldr r0, [sp], #4  \n\t"
       "str r0, [r1], #4  \n\t"
       "ldr r0, [sp], #4  \n\t"
       "str r0, [r1], #4  \n\t"
       "ldr r0, [sp], #4  \n\t"
       "str r0, [r1], #4  \n\t"
       "ldr r0, [sp], #4  \n\t"
       "str r0, [r1], #4  \n\t"
       "ldr r0, [sp], #4  \n\t"
       "str r0, [r1], #4  \n\t"
       "ldr r0, [sp], #4  \n\t"
       "str r0, [r1], #4  \n\t"
       "ldr r0, [sp], #4  \n\t"
       "str r0, [r1], #4  \n\t"
       "ldr r0, [sp], #4  \n\t"
       "str r0, [r1], #4  \n\t"
       "ldr r0, [sp], #4  \n\t"
       "str r0, [r1], #4  \n\t"
       "ldr r0, [sp], #4  \n\t"
       "str r0, [r1], #4  \n\t"
       "ldr r0, [sp], #4  \n\t"
       "str r0, [r1], #4  \n\t"
       "ldr r0, [sp], #4  \n\t"
       "str r0, [r1], #4  \n\t"
       "ldr r0, [sp], #4  \n\t"
       "str r0, [r1], #4"
       :
       : [gregs]"r"(gregs)
       : "r0", "r1", "sp", "memory"
       );
    // Correct for the modification of sp by dbg_pushGeneralRegisters()
    // before sp was saved.
    gregs->sp -= 52U;
    // Correct for the advancement of the pc over the two stm
    // instructions in dbg_pushGeneralRegisters().
    gregs->pc -= 8U;
      
  }

#if defined(__cplusplus)
} /* extern "C" */
#endif
