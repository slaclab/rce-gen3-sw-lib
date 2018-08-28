/*!@file    bsp.ld
*
*  @brief   Linker script for defining the BSP in the system
*
*           Use ld with -L to pick up the appropriate instance
*
*  @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
*  @date    Sat Mar 16 11:22:36 2013 -- Created
*
*  $Revision: 1907 $
*
*  @verbatim                    Copyright 2013
*                                      by
*                         The Board of Trustees of the
*                       Leland Stanford Junior University.
*                              All rights reserved.
*  @endverbatim
*/

/* Pick up the linker script for the BSP in use */
/*INCLUDE linkcmds.xilinx_zynq*/

/* Until the dynamic sections are sorted out, we need to invoke the old
   linkcmds.base.  The following is meant to be Temporary. */

/* Here is linkcmds.xilinx_zynq without the include of linkcmds.armv4: */
REGION_ALIAS ("REGION_START",          RAM);
REGION_ALIAS ("REGION_VECTOR",         RAM);
REGION_ALIAS ("REGION_TEXT",           RAM);
REGION_ALIAS ("REGION_TEXT_LOAD",      RAM);
REGION_ALIAS ("REGION_RODATA",         RAM);
REGION_ALIAS ("REGION_RODATA_LOAD",    RAM);
REGION_ALIAS ("REGION_DATA",           RAM);
REGION_ALIAS ("REGION_DATA_LOAD",      RAM);
REGION_ALIAS ("REGION_FAST_TEXT",      RAM);
REGION_ALIAS ("REGION_FAST_TEXT_LOAD", RAM);
REGION_ALIAS ("REGION_FAST_DATA",      RAM);
REGION_ALIAS ("REGION_FAST_DATA_LOAD", RAM);
REGION_ALIAS ("REGION_BSS",            RAM);
REGION_ALIAS ("REGION_WORK",           RAM);
REGION_ALIAS ("REGION_STACK",          RAM);

bsp_stack_irq_size = DEFINED (bsp_stack_irq_size) ? bsp_stack_irq_size : 4096;
bsp_stack_abt_size = DEFINED (bsp_stack_abt_size) ? bsp_stack_abt_size : 1024;

bsp_section_rwbarrier_align = DEFINED (bsp_section_rwbarrier_align) ? bsp_section_rwbarrier_align : 1M;

bsp_vector_table_in_start_section = 1;

bsp_translation_table_base = ORIGIN (RAM_MMU);

/* Here is linkcmds.armv4 without the include of linkcmds.base */
bsp_stack_abt_size = DEFINED (bsp_stack_abt_size) ? bsp_stack_abt_size : 128;
bsp_stack_abt_size = ALIGN (bsp_stack_abt_size, bsp_stack_align);

bsp_stack_fiq_size = DEFINED (bsp_stack_fiq_size) ? bsp_stack_fiq_size : 128;
bsp_stack_fiq_size = ALIGN (bsp_stack_fiq_size, bsp_stack_align);

bsp_stack_irq_size = DEFINED (bsp_stack_irq_size) ? bsp_stack_irq_size : 512;
bsp_stack_irq_size = ALIGN (bsp_stack_irq_size, bsp_stack_align);

bsp_stack_svc_size = DEFINED (bsp_stack_svc_size) ? bsp_stack_svc_size : 512;
bsp_stack_svc_size = ALIGN (bsp_stack_svc_size, bsp_stack_align);

bsp_stack_und_size = DEFINED (bsp_stack_und_size) ? bsp_stack_und_size : 128;
bsp_stack_und_size = ALIGN (bsp_stack_und_size, bsp_stack_align);

/* Here is the include of the old linkcmds.base */
INCLUDE linkcmds_old.base

/* End Temporary */
