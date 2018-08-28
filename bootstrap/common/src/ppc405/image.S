/*!@file     image.S
*
* @brief     Functions to handle a boot image
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      November 9, 2011 -- Created
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

        .file  "image.S"

        // Global Constant Definitions
#include "datCode.hh"
#include DAT_PUBLIC(    bootstrap, common, ppc-asm.h)
#include DAT_PUBLIC(    bootstrap, common, params.h)
#include DAT_PUBLIC(    bootstrap, common, dcr.h)
#include DAT_GENERATION(bootstrap, common, bootOpts.h)
#include DAT_PUBLIC(    bootstrap, common, functionStates.h)
#include DAT_PUBLIC(    bootstrap, common, bugcheck.h)

#define FN FN_3


        // Global Entry Point Definitions
        .section .text,"ax"

        //! imagePrepare - Prepare the flash subsystem to read an image
        // @return r3 The boot parameters
        .global imagePrepare
imagePrepare:
        mflr    r28               // Save the return address
        State   FN,FS_1           // Update state
        // Read page 0 of block 0 into data block
        li      r0,0x3            // Set function code and busy bit
        mtdcr   FLSHCMD,r0        // Initiate flash memory operation
        eieio                     // Let the write to the FLSHCMD finish
1:      mfdcr   r0,FLSHCMD        // Get transfer status
        andi.   r0,r0,0x1         // Check for operation initiated
        beq     1b                // Branch if operation not initiated yet
        State   FN,FS_2           // Update state
2:      mfdcr   r0,FLSHCMD        // Get transfer status
        andi.   r3,r0,0x1         // Check for operation completed
        bne     2b                // Branch if operation not completed yet
        State   FN,FS_3           // Update state
        extrwi. r3,r0,3,16        // Get operation status
        BugChk  bnel              // Bugcheck if status error

        // Validate flash memory contents
        State   FN,FS_4           // Update state
        li      r0,1              // Set validation word data block offset
        mtdcr   FLSHOFF,r0        // Transfer to flash memory data block offset
        eieio                     // Let the write to FLSHOFF finish
        mfdcr   r0,FLSHDAT        // Get validation word
        not.    r0,r0             // Check validation word
        BugChk  beql              // Bugcheck if validation word all ones

        // Ensure boot protocol version number matches what we're compiled with
        State   FN,FS_5           // Update state
        BootChk r0,BS_K_SCHEMA    // Bugcheck if mismatched

        // Process boot option vector element to determine which bootstrap to load
        State   FN,FS_6           // Update state
        BootOpt r0                // Get boot options (12 bits; we use bit 0)
        rlwinm  r0,r0,2,26,29     // Convert boot options to data block offset
        mtdcr   FLSHOFF,r0        // Transfer to flash memory data block offset
        eieio                     // Let the write to FLSHOFF finish
        mfdcr   r3,FLSHDAT        // Get boot flags
        State   FN,FS_7           // Update state
        mtlr    r28               // Restore the return address
        blr

        .eject

        //! imageLoad - Load the image
        // r11 - Address where ELF is loaded, or -1 if it still needs to be
        // @return r3 Entry point address
        .global imageLoad
imageLoad:
        State   FN,FS_8           // Update state
        mflr    r26               // Save the return address

        andi.   r0,r11,0x1f       // Is the load address valid?
        beq     1f                // Branch to move it to the right place

        // Load the ELF file
        mfdcr   r3,FLSHDAT        // Get file number
        mfdcr   r4,FLSHDAT        // Get file load address
        mr      r11,r4            // Save the load address
        lis     r5,(1<<28)@h      // Load a "largest possible" filesize in longwords (cpo)
        bl      readFile          // Read the boot file -> r3 = status
        State   FN,FS_9           // Update state
        clrlwi. r0,r3,16          // Check file read status
        BugChk  bnel              // Bugcheck if file status error
        State   FN,FS_10          // Update state

        // Parse the ELF headers
1:      mr      r3,r11            // Provide the load address
        bl      ElfLoad           // Load the ELF image -> r3 = entry point
        State   FN,FS_11          // Update state

        mtlr    r26               // Restore the return address
        blr
