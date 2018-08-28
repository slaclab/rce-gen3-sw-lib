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
#include DAT_GENERATION(bootstrap, common, bootOpts.h)
#include DAT_PUBLIC(    bootstrap, common, functionStates.h)
#include DAT_PUBLIC(    bootstrap, common, bugcheck.h)

#define FN FN_3


        // Global Entry Point Definitions
        .section .text,"ax"

        //! imagePrepare - Prepare the SD subsystem to read an image
        .global imagePrepare
imagePrepare:
        State   FN,FS_1           // Update state
        mflr    r24               // Save the return address

        // Ensure boot protocol version number matches what we're compiled with
        BootChk r0,BS_K_SCHEMA    // Bugcheck if mismatched

        // Process boot option element to determine which bootstrap to load
        State   FN,FS_2           // Update state
        BootOpt r3                // Get boot options and return it in r3
        State   FN,FS_3           // Update state

        mtlr    r24               // Restore return address
        blr

        .eject

        //! imageLoad - Load the image
        // @return r3 Entry point address
        .global imageLoad
imageLoad:
        State   FN,FS_4           // Update state
        mflr    r26               // Save the return address

        andi.   r0,r11,0x1f       // Is the load address valid?
        beq     1f                // Branch to move it to the right place

        State   FN,FS_5           // Update state

        //      Load the ELF file
        bl      sdInit            // Make SD firmware ready for operations

        State   FN,FS_6           // Update state

        bl      sdRdMbr           // Read the MBR to find the first partition

        State   FN,FS_7           // Update state

        bl      sdRdVolId         // Read the Volume ID of the first partition

        State   FN,FS_8           // Update state

        // Helpful values
        // - r3: Sector number of the Volume ID sector (LBA_Begin)
        // - r5: Number of reserved sectors
        // - r13: FAT Begin LBA
        // - r14: Sectors per Cluster
        // - r15: Data Begin LBA
        // - r16: Sectors per FAT
        // - r17: Root directory first cluster
        add     r13,r5,r3         // Calculate FAT_Begin_LBA = LBA_Begin + Rsvd
        mr      r14,r4            // Save Sectors_Per_Cluster value
        add     r15,r13,r6        // Calculate Data_Begin_LBA, where data is
        add     r15,r15,r6        // = FAT_Begin_LBA + (NumFats * Sectors_Per_FAT)
        mr      r16,r6            // Save Sectors_Per_FAT value
        mr      r17,r7            // save Root_Dir_1st_Cluster value

        BootOpt r3                // Get the bootstrap no.
        bl      sdI2File          // Convert bootstrap no. to a short filename

        State   FN,FS_9           // Update state

        bl      sdFindFile        // Find where the file is located

        State   FN,FS_10          // Next state <--- 4

        li      r5,0              // Load address is 0x00000000
        bl      sdRdFile          // Read the file into memory
        li      r11,0             // Load address is 0x00000000
        State   FN,FS_11          // Update state

        // Parse the ELF headers
1:      mr      r3,r11            // Provide the load address
        bl      ElfLoad           // Load the ELF image -> r3 = entry point
        State   FN,FS_12          // Update state

        mtlr    r26               // Restore the return address
        blr
