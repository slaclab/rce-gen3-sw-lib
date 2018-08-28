/*!
*
* @brief   Constants describing the memory layout
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    April 5, 2011 -- Created
*
* $Revision: 1233 $
*
* @verbatim:
*                               Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef CONFIGURATION_SYSTEM_RCECA9_MEMORY_HH
#define CONFIGURATION_SYSTEM_RCACA9_MEMORY_HH

#include <rtems.h>
#include <libcpu/arm-cp15.h>

#ifdef RTEMS_SMP
  #define MMU_DATA_READ_WRITE ARMV7_MMU_DATA_READ_WRITE_SHAREABLE
#else
  #define MMU_DATA_READ_WRITE ARMV7_MMU_DATA_READ_WRITE_CACHED
#endif


#include "system/Params.hh"
#include "memory/memory.hh"

#define NA 0                            // Not Applicable, yet
#define MSGAREABASE ((uint32_t)&MsgAreaBase)
#define MSGAREASIZE ((uint32_t)&MsgAreaSize)


namespace configuration {
  namespace system {

    namespace mem = service::memory;

    static mem::MemMap const memoryMap[] = {
      /* Base      , Size       , Page size,            Cache Policy,    Attributes */

#if 0                   // Enable to allow ability to set breakpoints with XMD
      // Message area
      { 0x00100000U, 0x40000000U, NA,                   NA,              MMU_DATA_READ_WRITE },
#endif
      // Message area
      { MSGAREABASE, MSGAREASIZE, NA,                   NA,              MMU_DATA_READ_WRITE },

      // FPGA slave 0
      { 0x40000000U, 0x40000000U, NA,                   NA,              ARMV7_MMU_DEVICE    },

      // FPGA slave 1
      { 0x80000000U, 0x40000000U, NA,                   NA,              ARMV7_MMU_DEVICE    },

      // Devices
      { 0xe0000000U, 0x02000000U, NA,                   NA,              ARMV7_MMU_DEVICE    },

      // AMBA APB Peripherals
      { 0xf8000000U, 0x01000000U, NA,                   NA,              ARMV7_MMU_DEVICE    },

      // OCM
      { 0xfff00000U, 0x00100000U, NA,                   NA,              MMU_DATA_READ_WRITE },

      //{ 0x00000000U, 0x01000000U, mem::Page::SIZE_16M,  mem::WRITE_BACK, mem::RX  }, // 16 MB
      //
      //{ 0x01000000U, 0x0f000000U, mem::Page::SIZE_16M,  mem::WRITE_BACK, mem::RWX }, // 240 MB
      //
      //{ 0x10000000U, 0x70000000U, mem::Page::SIZE_256M, mem::WRITE_BACK, mem::RWX }, // 2G - 256 MB

      { 0,           0,           0,                    NA,              NA }
    };
  }
}

#endif // CONFIGURATION_SYSTEM_RCECA9_MEMORY_HH
