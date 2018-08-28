/*!@file      boot.S
*
*  @brief     Boot into an EABI function
*
*  @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
*  @date      October 31, 2011 -- Created
*
*  $Revision: 761 $
*
*  @verbatim                    Copyright 2011
*                                      by
*                         The Board of Trustees of the
*                       Leland Stanford Junior University.
*                              All rights reserved.
*  @endverbatim
*/

	.file "boot.S"

#include "datCode.hh"
#include DAT_PUBLIC(bootstrap, common, ppc-asm.h)

	.section .text,"ax"
	.global Boot0

Boot0:
        lis     r0,     _start@h
        ori     r0, r0, _start@l
        mtlr    r0
	blr
