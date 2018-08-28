/*!@file   cpu_init.S
*
*  @brief   __cpu_init function required for the main() calling convention
*
*  @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
*  @date    December 14, 2011 -- Created
*
*  $Revision: 1356 $
*
*  @verbatim                    Copyright 2011
*                                      by
*                         The Board of Trustees of the
*                       Leland Stanford Junior University.
*                              All rights reserved.
*  @endverbatim
*/

        .file   "cpu_init.S"


	.section .text,"ax"
	.global __cpu_init

__cpu_init:
        blr
