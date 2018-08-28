// -*-Mode: C;-*-
/**
@file rtemsrefs.c
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
RCE %core

@par Abstract:
This file contains references to needed objects and functions that
are required by RTEMS.

@author
S Maldonado <smaldona@slac.stanford.edu>

@par Date created:
2014/01/15

@par Last commit:
\$Date: $ by \$Author: $.

@par Revision number:
\$Revision: $

@par Location in repository:
\$HeadURL: $

@par Credits:
SLAC
*/

/* libc.a - lib_a-hash.o - fixed in 1.20.0 */  
  int _fstat;
  int _stat;  

/* linker-symbols.h, arm-cp15-start.h - used to reside in memory.ld */  
  int bsp_section_work_begin;
  int bsp_section_work_end;
  int bsp_section_work_size;
  int bsp_section_stack_begin;
  int bsp_section_stack_end;  
  int bsp_section_stack_size;
  int bsp_nocache_heap_begin;
  int bsp_nocache_heap_end;
  int bsp_nocache_heap_size;

/* these are weak symbols that do not cause undefined symbol errors during dynamic linking */
/* consequently, we don't need to define them, but rather keep a note of their existence */
/* crtbegin.o */
// w _ITM_deregisterTMCloneTable
// w _ITM_registerTMCloneTable
// w _Jv_RegisterClasses
// w __deregister_frame_info
// w __register_frame_info
/* libgcc.a unwind-arm-common.inc */
// w __gnu_Unwind_Find_exidx
