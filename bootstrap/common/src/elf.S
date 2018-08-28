/*!@file     elf.S
*
* @brief     ELF loading routines
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      November 2, 2011 -- Created
*
* $Revision: 1358 $
*
* @verbatim                    Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

        .file   "elf.S"

#include "datCode.hh"
#include DAT_PUBLIC(bootstrap, common, ppc-asm.h)
#include DAT_PUBLIC(bootstrap, common, elf.h)
#include DAT_PUBLIC(bootstrap, common, functionStates.h)
#include DAT_PUBLIC(bootstrap, common, bugcheck.h)
#include DAT_PUBLIC(bootstrap, common, dcr.h)

#define FN FN_5


        // This code assumes a number of things:
        // - The offset to the first program segment isn't zero
        // - The number of program segments isn't zero
        // - The virtual and physical address spaces are identical
        // Files that don't have these characteristics shouldn't have been
        // stored into the flash memory in the first place.

	.section .text,"ax"
        .global ElfTest
ElfTest:
        lwz     r3,e_ident(r11)   // Load magic number
        subis   r3,r3,ELFMAG@ha   // Magic number value we expect
        subic.  r3,r3,ELFMAG@l    // Check that reality matches expectation
        blr

        // Parse the ELF file header and the program segment headers
        // @param r3 ELF file load address
        // @return r3 Entry point address
	.global ElfLoad
ElfLoad:
        mflr    r28               // Save the return address
        State   FN,FS_1           // Initial state <--- 1

        mr      r11,r3            // Address of beginning of file

        // Sanity check: Make sure we have the right magic number
        // We presume more stringent checks were done before flashing the file.
        // For example, only PT_LOAD Program Header sections should appear in the
        // file since other section types can't be used (and won't be loaded).
        bl      ElfTest
        BugChk  bnel              // Bugcheck when it doesn't
        State   FN,FS_2           // Next state

        // Handle program segments from back to front
        lwz     r12,e_phoff(r11)  // Offset to the first phdr
        add     r12,r12,r11       // Address of the first phdr
        lhz     r6,e_phentsize(r11) // Size of a phdr in lower 16 bits
        lhz     r7,e_phnum(r11)   // Number of phdrs in upper 16 bits
        subic.  r7,r7,1           // One less to go
        mullw   r0,r6,r7
        add     r12,r12,r0        // Address of the last phdr

        // Load only loadable segments
1:      State   FN,FS_3           // Next state
        lwz     r3,p_type(12)     // Get the segment type
        cmpwi   r3,PT_LOAD        // Is it a loadable segment?
        bne-    5f                // Don't try to load non PT_LOAD sections
        State   FN,FS_4           // Next state

        // Load the program segments, which appear in the file in ascending
        // vaddr order (Portable Formats Specification, Version 1.1), in
        // reverse order.  This allows the source and destination buffers to
        // overlap, since destination buffers will never appear lower in memory
        // than its corresponding source buffer.
        lwz     r4,p_offset(r12)  // Source offset from beginning of file
        add     r4,r4,r11         // Source address
        lwz     r3,p_vaddr(r12)   // Destination address
        lwz     r5,p_memsz(r12)   // Number of bytes of "allocated" to the segment
        lwz     r8,p_filesz(r12)  // Number of bytes of program data
        sub.    r0,r5,r8          // Number of "extra" bytes to zero

        // First, zero any "extra" space between the segments' filesz and memsz
        add     r3,r3,r5          // After the end of the destination buffer
        beq+    3f                // If no "extra" bytes, skip ahead
        State   FN,FS_5           // Next state
        srawi   r0,r0,2           // Convert to number of "extra" longwords
        mtctr   r0                // Number of words to zero
        li      r0,0              // Value to set memory to
2:      stwu    r0,-4(r3)         // Predecrement and zero the word
        bdnz+   2b                // Loop until CTR is zero

        // Second, load the program segments
3:      State   FN,FS_6           // Next state
        add     r4,r4,r8          // After the end of the source buffer
        cmpw    r3,r4             // If source and destination are the same...
        beq+    5f                // ... no need to copy, so skip ahead
        State   FN,FS_7           // Next state
        srawi.  r8,r8,2           // Convert to number of longwords
        beq+    5f                // If none, skip ahead
        State   FN,FS_8           // Next state
        mtctr   r8                // Number of words to copy
4:      lwzu    r0,-4(r4)         // Predecrement and load source       value
        stwu    r0,-4(r3)         // Predecrement and store destination value
        bdnz+   4b                // Loop until CTR is zero

5:      State   FN,FS_9           // Next state
        sub     r12,r12,r6        // Move up one phdr toward the front
        subic.  r7,r7,1           // One less to go
        bge     1b                // Loop if there are more phdrs to handle
        State   FN,FS_10           // Done

        lwz     r3,e_entry(r11)   // Return e_entry
        mtlr    r28               // Restore the return address
        blr
