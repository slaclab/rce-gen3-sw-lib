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
INCLUDE linkcmds.xilinx_zynq
