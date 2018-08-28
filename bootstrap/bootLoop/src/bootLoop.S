/*!@file    bootLoop.S
*
*  @brief   Simple boot loop for keeping the processor busy
*
*     This code results in an innoccuous executable that can be burned into the
*     FPGA so that the processor has something to chew on without disturbing
*     the state of the system.  XMD can then be used to connect to the system
*     through JTAG interface so that other code can be loaded and the system
*     can be inspected.
*
*  @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
*  @date    October 27, 2011 -- Created
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

	.file "bootLoop.S"

	.section .text,"ax"
	.global Boot0

Boot0:  b       Boot0
