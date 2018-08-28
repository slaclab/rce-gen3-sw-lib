/*!@file    memory.ld
*
*  @brief   Linker script for defining memories in the system
*
*           Use ld with -L to pick up the appropriate instance
*
*  @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
*  @date    January 10, 2012 -- Created
*
*  $Revision: 1738 $
*
*  @verbatim                    Copyright 2012
*                                      by
*                         The Board of Trustees of the
*                       Leland Stanford Junior University.
*                              All rights reserved.
*  @endverbatim
*/

/* Define Memories in the system */
MEMORY
{
  vectors (r x) : ORIGIN = 0x00104000, LENGTH = 16M
  text    (r x) : ORIGIN = 0x01104000, LENGTH = 32M   - 16M
  data    (rwx) : ORIGIN = 0x02104000, LENGTH = 128M  - 32M - 1M
  msg     (rwx) : ORIGIN = 0x08004000, LENGTH = 1M
  io      (rw ) : ORIGIN = 0x08104000, LENGTH = 256M  - 144M
  bram    (rwx) : ORIGIN = 0x0F104000, LENGTH = 4K
}

