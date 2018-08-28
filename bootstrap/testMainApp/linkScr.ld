_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x200;
_HEAP_SIZE  = DEFINED(_HEAP_SIZE)  ? _HEAP_SIZE  : 0x100;

/* Define Memories in the system */

MEMORY
{
  scratch :  ORIGIN = 0x00000000,  LENGTH = 0x00000100
  vectors :  ORIGIN = 0x00000100,  LENGTH = 0x00002f00
  ram     :  ORIGIN = 0x00003000,  LENGTH = 0x07ffd000
  stack   :  ORIGIN = 0x07fffc00,  LENGTH = 0x00000200
  heap    :  ORIGIN = 0x07fffe00,  LENGTH = 0x00000100
  rom     :  ORIGIN = 0xFFFFF000,  LENGTH = 0x00000FC0
  xfrvec  :  ORIGIN = 0xFFFFFFC0,  LENGTH = 0x00000038
  version :  ORIGIN = 0xFFFFFFF8,  LENGTH = 0x00000004
  boot    :  ORIGIN = 0xFFFFFFFC,  LENGTH = 0x00000004
}

/* Specify the default entry point to the program */

ENTRY(_start)

PHDRS
{
  vectors_phdr PT_LOAD;
  ram_phdr     PT_LOAD;
  rom_phdr     PT_LOAD;
  stack_phdr   PT_LOAD;
  heap_phdr    PT_LOAD;
  xfrvec_phdr  PT_LOAD;
  version_phdr PT_LOAD;
  boot_phdr    PT_LOAD;
}

/* Define the sections, and where they are mapped in memory */

SECTIONS
{
  .vectors : {           /* Load the vectors */
    *(.vectors)
  } > vectors :vectors_phdr

  .text : {
     *(.text)
     *(.text.*)
     *(.gnu.linkonce.t.*)
  } > ram : ram_phdr

  .init : {
     KEEP (*(.init))
  } > ram : ram_phdr

  .fini : {
     KEEP (*(.fini))
  } > ram : ram_phdr

  .rodata : {
     __rodata_start = .;
     *(.rodata)
     *(.rodata.*)
     *(.gnu.linkonce.r.*)
     __rodata_end = .;
  } > ram : ram_phdr

  .rodata1 : {
     __rodata1_start = .;
     *(.rodata1)
     *(.rodata1.*)
     __rodata1_end = .;
  } > ram : ram_phdr

  .sdata2 : {
     __sdata2_start = .;
     *(.sdata2)
     *(.sdata2.*)
     *(.gnu.linkonce.s2.*)
     __sdata2_end = .;
  } > ram : ram_phdr

  .sbss2 : {
     __sbss2_start = .;
     *(.sbss2)
     *(.sbss2.*)
     *(.gnu.linkonce.sb2.*)
     __sbss2_end = .;
  } > ram : ram_phdr

  .data : {
     __data_start = .;
     *(.data)
     *(.data.*)
     *(.gnu.linkonce.d.*)
     __data_end = .;
  } > ram : ram_phdr

  .data1 : {
     __data1_start = .;
     *(.data1)
     *(.data1.*)
     __data1_end = .;
  } > ram : ram_phdr

  .got : {
     *(.got)
  } > ram : ram_phdr

  .got1 : {
     *(.got1)
  } > ram : ram_phdr

  .got2 : {
     *(.got2)
  } > ram : ram_phdr

  .ctors : {
     __CTOR_LIST__ = .;
     ___CTORS_LIST___ = .;
     KEEP (*crtbegin.o(.ctors))
     KEEP (*(EXCLUDE_FILE(*crtend.o) .ctors))
     KEEP (*(SORT(.ctors.*)))
     KEEP (*(.ctors))
     __CTOR_END__ = .;
     ___CTORS_END___ = .;
  } > ram : ram_phdr

  .dtors : {
     __DTOR_LIST__ = .;
     ___DTORS_LIST___ = .;
     KEEP (*crtbegin.o(.dtors))
     KEEP (*(EXCLUDE_FILE(*crtend.o) .dtors))
     KEEP (*(SORT(.dtors.*)))
     KEEP (*(.dtors))
     __DTOR_END__ = .;
     ___DTORS_END___ = .;
  } > ram : ram_phdr

  .fixup : {
     __fixup_start = .;
     *(.fixup)
     __fixup_end = .;
  } > ram : ram_phdr

  .eh_frame : {
     *(.eh_frame)
  } > ram : ram_phdr

  .jcr : {
     *(.jcr)
  } > ram : ram_phdr

  .gcc_except_table : {
     *(.gcc_except_table)
  } > ram : ram_phdr

  .sdata : {
     __sdata_start = .;
     *(.sdata)
     *(.sdata.*)
     *(.gnu.linkonce.s.*)
     __sdata_end = .;
  } > ram : ram_phdr

  .sbss : {
     __sbss_start = .;
     *(.sbss)
     *(.sbss.*)
     *(.gnu.linkonce.sb.*)
     *(.scommon)
     __sbss_end = .;
  } > ram : ram_phdr

  .tdata : {
     __tdata_start = .;
     *(.tdata)
     *(.tdata.*)
     *(.gnu.linkonce.td.*)
     __tdata_end = .;
  } > ram : ram_phdr

  .tbss : {
     __tbss_start = .;
     *(.tbss)
     *(.tbss.*)
     *(.gnu.linkonce.tb.*)
     __tbss_end = .;
  } > ram : ram_phdr

  .bss : {
     __bss_start = .;
     *(.bss)
     *(.bss.*)
     *(.gnu.linkonce.b.*)
     *(COMMON)
     . = ALIGN(4);
     __bss_end = .;
  } > ram : ram_phdr

  /* Generate Stack and Heap Sections */

  .stack : {
     _stack_end = .;
     . += _STACK_SIZE;
     . = ALIGN(16);
     __stack = .;
  } > stack : stack_phdr

  .heap : {
     . = ALIGN(16);
     _heap_start = .;
     . += _HEAP_SIZE;
     . = ALIGN(16);
     _heap_end = .;
     _end = .;
  } > heap : heap_phdr

  .inproc :  /* load initialization code */
  {
    *(.inproc)
  } > rom : rom_phdr

  .indata :  /* load initialization data */
  {
    *(.indata)
  } > rom : rom_phdr

  .xfrvec :  /* load transfer vector just below version and boot vector */
  {
    *(.xfrvec)
  } > xfrvec : xfrvec_phdr

  .version : /* load version number just below boot vector */
  {
    *(.version)
  } > version : version_phdr

  .boot :    /* make sure the boot vector is loaded at 0xFFFFFFFC */
  {
    *(.boot)
  } > boot : boot_phdr
}
