/*!@file     Params.hh
*
* @brief     General system parameters
*
* @author    Richard Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      April 22, 2011 -- Imported
*            March  2, 2012 -- Expanded
*
* $Revision: 2961 $
*
* @verbatim                    Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

extern "C" char bsp_section_bss_end;
extern "C" char bsp_section_stack_begin;
extern "C" char bsp_section_stack_size;
extern "C" char bsp_section_stack_end;


namespace configuration {
  namespace system {
    static const uint32_t bspRamBase     = (uint32_t)&bsp_section_bss_end;
    static const uint32_t bspRamEnd      = sysRamEnd;
    static const uint32_t rtemsEnd       = (uint32_t)&bsp_section_bss_end;
    static const uint32_t stackEnd       = (uint32_t)&bsp_section_stack_begin;
    static const uint32_t stackSize      = (uint32_t)&bsp_section_stack_size;
    static const uint32_t stackBase      = (uint32_t)&bsp_section_stack_end;
  }
}
