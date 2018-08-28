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


#include "memory/memory.hh"


namespace configuration {
  namespace system {

    static uint32_t const RO_Base = 0x00000000;
    static uint32_t const RO_Size = 0x01000000; // 16 MB

    static uint32_t const RW_Base = 0x01000000;
    static uint32_t const RW_Size = 0x0f000000; // 240 MB

    static uint32_t const IO_Base = 0x10000000;
    static uint32_t const IO_Size = 0x70000000; // 2G - 256 MB

    namespace mem = service::memory;

    static mem::MemMap const memoryMap[] = {
      { RO_Base, RO_Size, mem::Page::SIZE_16M,  mem::WRITE_BACK,  mem::RX   },
      { RW_Base, RW_Size, mem::Page::SIZE_16M,  mem::WRITE_BACK,  mem::RWX  },
      { IO_Base, IO_Size, mem::Page::SIZE_256M, mem::WRITE_BACK,  mem::RWX  },
      {       0,       0,                    0, mem::NEVER_WRITE, mem::NONE }
    };
  }
}

#endif // CONFIGURATION_SYSTEM_RCECA9_MEMORY_HH
