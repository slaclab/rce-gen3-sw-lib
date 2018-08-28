/*!@file    bootstrap.ld
*
*  @brief   Linker script for building bootable images
*
*  @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
*  @date    October 27, 2011 -- Created
*
*  $Revision: 490 $
*
*  @verbatim                    Copyright 2011
*                                      by
*                         The Board of Trustees of the
*                       Leland Stanford Junior University.
*                              All rights reserved.
*  @endverbatim
*/

/* Define Memories in the system */
INCLUDE memory.ld

/* Specify the default entry point to the program */
ENTRY(Boot)

/* Prepare the block ram data as one loadable program section */
PHDRS
{
  text PT_LOAD;
  data PT_LOAD;
  bram PT_LOAD;
}

/* Define the sections, and where they are mapped in memory */
SECTIONS
{
  .vectors :             /* Load the vectors */
  {
    *(.vectors)
  } > bram :bram

  .text :                /* Load initialization code */
  {
    *(.text)
  } > bram :bram

  .data :                /* Load initialization data */
  {
    *(.data)
  } > bram :bram

  .bldInfo 0xffffffc0 :  /* Load build information just below version number */
  {
    *(.build)
  } > bram :bram

  .version 0xfffffff8 :  /* Load version number just below boot vector */
  {
    *(.version)
  } > bram :bram

  .boot 0xfffffffc :     /* Ensure the boot vector is loaded at 0xFFFFFFFC */
  {
    *(.boot)
  } > bram :bram

  /DISCARD/ :            /* Discard unused sections from the output file */
  {
    *(.comment)
    *(.debug*)
  }
}
