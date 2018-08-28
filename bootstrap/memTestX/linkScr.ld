/*!@file   linkScr.ld
*
* @brief   This is the linker script for the Xilinx Memory Test
*
*          The MsgArea will contain output from print() calls.  This can be
*          dumped in XMD using the msglog TCL script in dumps.tcl.
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    December 8, 2011 -- Created
*
* $Revision: 1376 $
*
* @verbatim                    Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
MsgAreaSize = DEFINED(MsgAreaSize) ? MsgAreaSize : 8K-8;
HeapSize    = DEFINED(HeapSize)    ? HeapSize    : 8K;
StackSize   = DEFINED(StackSize)   ? StackSize   : 32K;

/* Specify the default entry point to the program */

ENTRY(_start)

/* Define Memories in the system */

MEMORY
{
   rldram  : ORIGIN = 0x00000000, LENGTH = 2048M
   bram    : ORIGIN = 0xFFFF0000, LENGTH = 0x00010000 - 4 - 4
   version : ORIGIN = 0xFFFFFFF8, LENGTH = 0x00000004
   boot    : ORIGIN = 0xFFFFFFFC, LENGTH = 0x00000004
}

PHDRS
{
  bram_phdr    PT_LOAD;
  version_phdr PT_LOAD;
  boot_phdr    PT_LOAD;
}

/* Define the sections, and where they are mapped in memory */

SECTIONS
{
  .vectors : {           /* Load the vectors */
    *(.vectors)
  } > bram :bram_phdr

  .text : {
     __text_start = .;
     *(.text)
     *(.text.*)
     *(.gnu.linkonce.t.*)
     __text_end = .;
  } > bram :bram_phdr

  .init : {
     KEEP (*(.init))
  } > bram :bram_phdr

  .fini : {
     KEEP (*(.fini))
  } > bram :bram_phdr

  .rodata : {
     __rodata_start = .;
     *(.rodata)
     *(.rodata.*)
     *(.gnu.linkonce.r.*)
     __rodata_end = .;
  } > bram :bram_phdr

  .rodata1 : {
     __rodata1_start = .;
     *(.rodata1)
     *(.rodata1.*)
     __rodata1_end = .;
  } > bram :bram_phdr

  .sdata2 : {
     __sdata2_start = .;
     *(.sdata2)
     *(.sdata2.*)
     *(.gnu.linkonce.s2.*)
     __sdata2_end = .;
  } > bram :bram_phdr

  .sbss2 : {
     __sbss2_start = .;
     *(.sbss2)
     *(.sbss2.*)
     *(.gnu.linkonce.sb2.*)
     __sbss2_end = .;
  } > bram :bram_phdr

  .data : {
     __data_start = .;
     *(.data)
     *(.data.*)
     *(.gnu.linkonce.d.*)
     __data_end = .;
  } > bram :bram_phdr

  .data1 : {
     __data1_start = .;
     *(.data1)
     *(.data1.*)
     __data1_end = .;
  } > bram :bram_phdr

  .got : {
     *(.got)
  } > bram :bram_phdr

  .got1 : {
     *(.got1)
  } > bram :bram_phdr

  .got2 : {
     *(.got2)
  } > bram :bram_phdr

  .ctors : {
     __CTOR_LIST__ = .;
     ___CTORS_LIST___ = .;
     KEEP (*crtbegin.o(.ctors))
     KEEP (*(EXCLUDE_FILE(*crtend.o) .ctors))
     KEEP (*(SORT(.ctors.*)))
     KEEP (*(.ctors))
     __CTOR_END__ = .;
     ___CTORS_END___ = .;
  } > bram :bram_phdr

  .dtors : {
     __DTOR_LIST__ = .;
     ___DTORS_LIST___ = .;
     KEEP (*crtbegin.o(.dtors))
     KEEP (*(EXCLUDE_FILE(*crtend.o) .dtors))
     KEEP (*(SORT(.dtors.*)))
     KEEP (*(.dtors))
     __DTOR_END__ = .;
     ___DTORS_END___ = .;
  } > bram :bram_phdr

  .fixup : {
     __fixup_start = .;
     *(.fixup)
     __fixup_end = .;
  } > bram :bram_phdr

  .eh_frame : {
     *(.eh_frame)
  } > bram :bram_phdr

  .jcr : {
     *(.jcr)
  } > bram :bram_phdr

  .gcc_except_table : {
     *(.gcc_except_table)
  } > bram :bram_phdr

  .sdata : {
     __sdata_start = .;
     *(.sdata)
     *(.sdata.*)
     *(.gnu.linkonce.s.*)
     __sdata_end = .;
  } > bram :bram_phdr

  .sbss : {
     __sbss_start = .;
     *(.sbss)
     *(.sbss.*)
     *(.gnu.linkonce.sb.*)
     *(.scommon)
     __sbss_end = .;
  } > bram :bram_phdr

  .tdata : {
     __tdata_start = .;
     *(.tdata)
     *(.tdata.*)
     *(.gnu.linkonce.td.*)
     __tdata_end = .;
  } > bram :bram_phdr

  .tbss : {
     __tbss_start = .;
     *(.tbss)
     *(.tbss.*)
     *(.gnu.linkonce.tb.*)
     __tbss_end = .;
  } > bram :bram_phdr

  .bss : {
     __bss_start = .;
     *(.bss)
     *(.bss.*)
     *(.gnu.linkonce.b.*)
     *(COMMON)
     . = ALIGN(4);
     __bss_end = .;
  } > bram :bram_phdr

  /* Generate Stack and Heap Sections */

  .stack : {
     _stack_end = .;
     . += StackSize;
     . = ALIGN(16);
     __stack = .;
  } > bram :bram_phdr

  .heap : {
     . = ALIGN(16);
     _heap_start = .;
     . += HeapSize;
     . = ALIGN(16);
     _end = .;
  } > bram :bram_phdr

  /* Message area for capturing print output */
  /* Placed here to be easily findable with a debugger */
  .msg 0xffffe000: {
    MsgAreaBase  = .;
    .           += MsgAreaSize;
  } > bram :bram_phdr

  /DISCARD/ :
  {
    *(.comment)
    *(.gnu.attributes)
    *(.debug*)
  }

}
