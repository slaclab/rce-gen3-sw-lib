// -*-Mode: C;-*-
/*!@file     cpu-inl.h
*
* @brief     CPU implementation
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      March 2, 2012 -- Created
*
* @verbatim                    Copyright 2012
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

#include "cpu/impl/slcr.h"
#include <rtems.h>

inline void TOOL_CPU_initialize()
{
  // Ensure caches are enabled
  rtems_cache_enable_instruction();
  rtems_cache_enable_data();
}

inline void TOOL_CPU_reset(uint32_t rst)
{
  uint32_t vec;

  if (rst == 2) {
    uint32_t reboot;

    // Unlock the SLCR then reset the system.
    TOOL_CPU_SLCR_unlock();

    // Clear 0x0F000000 bits of reboot status register to workaround
    // the FSBL not loading the bitstream after soft-reboot
    // This is a temporary solution until we know more.
    reboot = TOOL_CPU_SLCR_readReg(SLCR_REBOOT_STATUS);
    TOOL_CPU_SLCR_writeReg(SLCR_REBOOT_STATUS, reboot & 0xF0FFFFFF);

    // This is a hard reset.  RAM is cleared and PL is reset.
    TOOL_CPU_SLCR_writeReg(SLCR_PSS_RST_CTRL_OFFSET, 1);
  } else {
    // Soft reset: jump to the reset vector.
    asm volatile ("mrc p15, 0, %0, c12, c0, 0": "=r"(vec));
    asm volatile ("mov pc,%0" : : "r"(&vec));
  }
}
