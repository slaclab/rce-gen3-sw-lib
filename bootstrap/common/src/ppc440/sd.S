//!@file    sd.S
//
// @brief   Code for interacting with a FAT formatted SD card
//
// @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
//
// @date    October 23, 2012 -- Created
//
// $Revision: 1476 $
//
// @verbatim                    Copyright 2012
//                                     by
//                        The Board of Trustees of the
//                      Leland Stanford Junior University.
//                             All rights reserved.
// @endverbatim

        .file  "sd.S"

        // Global Constant Definitions
#include "datCode.hh"
#include DAT_PUBLIC(    bootstrap, common, ppc-asm.h)
#include DAT_PRIVATE(   bootstrap, common, sdIO.h)
#include DAT_GENERATION(bootstrap, common, bootOpts.h)
#include DAT_PUBLIC(    bootstrap, common, functionStates.h)
#include DAT_PUBLIC(    bootstrap, common, bugcheck.h)

#define FN FN_4

#define SD_CARD_CMD_FIFO_DEPTH  512
#define SD_CARD_RSP_FIFO_DEPTH 1024
#define SD_CARD_TX_FIFO_DEPTH   512
#define SD_CARD_RX_FIFO_DEPTH   512

#define SD_CARD_BLOCK_SIZE_DEFAULT 512

#define OPCODE(cmd)  ((cmd & 0x3f) << 26)

/**
 * @name Supported Commands
 * @{
 */

#define SD_CARD_CMD_READ_SINGLE_BLOCK OPCODE(17)

/** @} */

/* *
 * @name Supported Application Commands
 * @{
 */

#define SD_CARD_ACMD_SD_SEND_OP_COND OPCODE(41)

/** @} */

/**
 * @name Command Flags
 * @{
 */

#define SD_CARD_OP_COND_SIZE 4

#define SD_CARD_FLAG_HCS  0x40000000
#define SD_CARD_FLAG_BUSY 0x80000000

/** @} */

#define SCRATCH_MEMORY 0x20000000@h  /* Start of scratch memory  */

#define MBR_O_P0 446                 /* Size of MBR boot code area */
#define MBR_O_P0_TYPE_CODE (MBR_O_P0 + 4)
#define MBR_O_P0_LBA_BEGIN (MBR_O_P0 + 8)
#define MBR_O_SIGNATURE 510

#define MBR_K_TC_FAT32  0xb          /* FAT32 type code  */
#define MBR_K_SIGNATURE 0xaa55

#define VID_O_BYTESPERSEC 0x0b
#define VID_O_SECPERCLUS  0x0d
#define VID_O_RSVDSECCNT  0x0e
#define VID_O_NUMFATS     0x10
#define VID_O_SECPERFAT   0x24
#define VID_O_ROOTCLUS    0x2c
#define VID_O_SIGNATURE   0x1fe

#define VID_K_BYTESPERSEC SD_CARD_BLOCK_SIZE_DEFAULT
#define VID_K_NUMFATS     2
#define VID_K_SIGNATURE   0xaa55

#define SPC 0x20202020               /* "    "  */
#define SYS 0x53595300               /* "SYS\0"  */


        // Global Entry Point Definitions
        .section .text,"ax"

        .global sdCmd
        .global sdInit
        .global sdRdMbr
        .global sdRdVolId
        .global sdI2File
        .global sdFindFile
        .global sdRdFile

        .eject

        //! sdCmd - Command the SD and retrieve the response
        // Since this code operates only at the boot loader level, TID is zero.
        // Bugchecks: When the response TID is not zero.
        //            When response opcode is not the commanded opcode.
        //            When the response result code is not 0x10.
        // Clobbered: None
        // @param r3 Command opcode (i.e., cmd no. placed in the opcode field)
        // @param r4 Command argument
        // @return r3 The size (B) of data pushed into the receive FIFO
        //         r4 Unchanged
sdCmd:  mflr    r28               // Save return address
        mtsd    r3,r4             // Send the command
1:      mfsd.   r1                // Retrieve the response
        bt      3,1b              // Retry if response not ready yet
        andi.   r0,r1,0xff        // Is TID zero?
        BugChk  bnel              // Bugcheck if not
        slwi    r0,r1,18          // Line opcode field up with input value
        cmplw   r0,r3             // Do they match?
        BugChk  bnel              // Bugcheck if not
        extrwi  r0,r1,5,12        // Extract the result code
        cmplwi  r0,0x10           // Is it 0x10?
        BugChk  bnel              // Bugcheck if not
        extrwi  r3,r1,10,2        // Extract the size value
        mtlr    r28               // Restore return address
        blr


        //! sdInit - Initialize the SD firmware to an operational state
        // Bugchecks: When the OCR isn't correctly read.
        //            When it is found that the SD card is not high capacity.
        // Clobbered: ctr,r3,r10
        // This function takes no arguments and produces no return value.
sdInit: mflr    r10               // Save return address
        //sdReset
1:      mfsd.   r0                // Read a response FIFO entry
        bf      3,1b              // Go again if FIFO is not empty
        li      r0,SD_CARD_RX_FIFO_DEPTH
        mtctr   r0                // Number of receive FIFO entries to unload
        li      r1,0              // Offset of zero
        lis     r0,SCRATCH_MEMORY // Scratch memory location
2:      stsd    r0,r1             // Fetch a value
        bdnz    2b                // Loop until CTR is exhausted

        lis     r3,SD_CARD_ACMD_SD_SEND_OP_COND@h
        bl      sdCmd             // Get sd-card operating condition
        cmplwi  r3,SD_CARD_OP_COND_SIZE
        BugChk  bnel              // Bugcheck if the size isn't right
        lis     r3,SCRATCH_MEMORY // Address of scratch memory
        li      r1,0              // Offset of zero
        stsd    r3,r1             // Store the register from FIFO to memory
        lwz     r1,0(r3)          // Get the value into a GP register
        andis.  r1,r1,SD_CARD_FLAG_HCS@h
        BugChk  beql              // Bugcheck if card is not high capacity
        mtlr    r10               // Restore return address
        blr


        //! sdStBlk1 - Read and store a single block to memory
        // Bugchecks: When the amount of received data is not 512 bytes.
        // Clobbered: r3,r10
        // @param r4 Sector number to read
        // @param r5 Location in memory where to store the data
        // @return r3 Memory location after the stored data
        // @return r4 Unchanged
        // @return r5 Unchanged
sdStBlk1:
        mflr    r10               // Save return address
        lis     r3,SD_CARD_CMD_READ_SINGLE_BLOCK@h
        bl      sdCmd             // Issue the command
        cmplwi  r3,SD_CARD_BLOCK_SIZE_DEFAULT // Length should be 512
        BugChk  bnel              // Bugcheck if it isn't
        li      r0,8              // Increment by 8 bytes per cycle
        li      r1,64             // 64 = 512 / 8
        la      r3,-8(r5)         // Predecrement since stsdu preincrements
1:      stsdu   r3,r0             // Store data from FIFO to memory
        subic.  r1,r1,1           // Reduce the number to go by one
        bne     1b                // Loop if not zero
        la      r3,8(r3)          // Advance to after the last written quadword
        mtlr    r10               // Restore return address
        blr


        //! sdStBlk - Store multiple blocks to memory
        // Bugchecks: None
        // Clobbered: ctr,r11
        // @param r3 Number of sectors to read
        // @param r4 First sector number to read
        // @param r5 Location in memory where to store the data
        // @return r3 The location after the stored data
        // @return r4 Last sector number read plus one
        // @return r5 The location after the stored data
sdStBlk:
        mflr    r11               // Save return address
        mtctr   r3                // Load the loop counter
1:      bl      sdStBlk1          // Load a sector into the receive FIFO
        addi    r4,r4,1           // Increment to next sector
        mr      r5,r3             // Where to load next sector
        bdnz    1b                // Loop until counter exhausted
        mtlr    r11               // Restore return address
        blr


        //! Read the MBR sector
        // Bugchecks: When the expected MBR signature isn't found.
        // Clobbered: r4,r5,r12
        // @return r3 contains the LBA_Begin value
sdRdMbr:
        mflr    r12               // Save return address
        li      r4,0              // Index number of the first sector
        lis     r5,SCRATCH_MEMORY // Load it to the scratch area
        bl      sdStBlk1          // Read and store the sector
        lbz     r1,MBR_O_P0_TYPE_CODE(r5) // Check the partition Type Code
        cmplwi  r1,MBR_K_TC_FAT32         // Is it a FAT32 partition?
        BugChk  bnel                      // BugCheck if not: Unsupported SD Type
        li      r1,MBR_O_P0_LBA_BEGIN     // Offset where LBA Begin is stored
        lwbrx   r3,r5,r1                  // Load and byteswap LBA Begin
        li      r1,MBR_O_SIGNATURE        // Offset of the MBR sector signature
        lhbrx   r1,r5,r1                  // Load the signature
        cmplwi  r1,MBR_K_SIGNATURE        // Is it correct?
        BugChk  bnel              // Bugcheck if not
        mtlr    r12               // Restore return address
        blr


        //! Read the Volume ID sector
        // Bugchecks: When the expected Volume ID signature isn't found.
        //            When the number of bytes per sector is not 512.
        // Clobbered: r12,r13
        // @param r3 Sector number of the Volume ID sector (LBA_Begin)
        // @return r3 Unchanged
        // @return r4 Sectors per cluster
        // @return r5 Number of reserved sectors
        // @return r6 Sectors per FAT
        // @return r7 Root directory first cluster
sdRdVolId:
        mflr    r12               // Save return address
        mr      r13,r3            // Save r3
        mr      r4,r3             // Sector number of 1st partition Volume ID
        lis     r5,SCRATCH_MEMORY // Load it to the scratch area
        bl      sdStBlk1          // Read and store the sector
        mr      r2,r5             // Free up r5
        li      r1,VID_O_BYTESPERSEC
        lhbrx   r1,r1,r2                // Bytes per Sector
        cmplwi  r1,VID_K_BYTESPERSEC    // Compare with expected value
        BugChk  bnel                    // Bugcheck if not the same
        mr      r3,r13                  // Restore r3
        lbz     r4,VID_O_SECPERCLUS(r2) // Sectors per Cluster
        li      r1,VID_O_RSVDSECCNT
        lhbrx   r5,r1,r2                // Number of Reserved Sectors
        lbz     r1,VID_O_NUMFATS(r2)    // Number of FATs
        cmplwi  r1,VID_K_NUMFATS        // Compare with expected value
        BugChk  bnel                    // Bugcheck if not the same
        li      r1,VID_O_SECPERFAT
        lwbrx   r6,r1,r2                // Sectors per FAT
        li      r1,VID_O_ROOTCLUS
        lwbrx   r7,r1,r2                // Root Directory First Cluster
        li      r1,VID_O_SIGNATURE      // Volume ID sector signature
        lhbrx   r1,r1,r2                // Load the signature
        cmplwi  r1,VID_K_SIGNATURE      // Is it correct?
        BugChk  bnel              // Bugcheck if not
        mtlr    r12               // Restore return address
        blr


        //! Construct a short filename
        //  The resulting 4 character string is in the form of a hex number
        //  followed by spaces, with no superfluous leading zeros.
        // Bugchecks: None
        // Clobbered: None
        // @param r3 Twelve bit value to be converted to ASCII
        // @return r4 Four character string with trailing spaces
_hexDigits:
        .ascii  "0123456789ABCDEF"

sdI2File:
        lis     r1,   _hexDigits@h // Load the address of the digits string
        ori     r1,r1,_hexDigits@l
        rlwinm  r4,r3,24,28,31    // r4 <- most significant nibble
        lbzx    r4,r1,r4          // Convert to ASCII
        rlwinm  r0,r3,28,28,31    // r0 <- middle nibble
        lbzx    r0,r1,r0          // Convert to ASCII
        rlwimi  r0,r4,8,16,24     // Merge r4 and r0 into 2 bytes in r0
        andi.   r4,r3,0xf         // r4 <- least significant nibble
        lbzx    r4,r1,r4          // Convert to ASCII
        rlwimi  r4,r0,8,8,24      // Merge r4 and r0 into 3 bytes in r4
        slwi    r4,r4,8           // Shift up by one byte
        ori     r4,r4,0x20        // Merge in a space character
        cmplwi  r3,0x100          // Is value in range [0x100 to 0xfff]?
        bgelr                     // Return if so
        rlwinm  r4,r4,8,28,26     // Rotate  up by one byte and cvt '0' to ' '
        cmplwi  r3,0x10           // Is value in range [0x10 to 0xff]?
        bgelr                     // Return if so
        rlwinm  r4,r4,8,28,26     // Rotate  up by one byte and cvt '0' to ' '
        blr                       // Return


        //! Find the requested file in the partition's Root directory
        // Bugchecks: When file isn't found.
        // Clobbered: ctr,r5,r6,r7,r11
        // @param r4 Four character ASCII name of file to find
        // @param r15 Sector where data begins (Data_Begin_LBA)
        // @param r17 Root directory first cluster number
        // @return r3 1st cluster number for file
        // @return r4 File size in bytes
sdFindFile:
        mflr    r12               // Save return address
        mr      r6,r4             // Save the input filename
        subi    r4,r17,2          // Clusters start at 2
        add     r4,r4,r15         // Sector number where root directory starts
2:      lis     r5,SCRATCH_MEMORY // Load it to the scratch area
        bl      sdStBlk1          // Read and store the sector

        li      r0,512/32         // Number of directory entries per sector
        mtctr   r0                // Load loop counter
3:      lwz     r0,0x00(r5)       // Load 1st word of the short name
        cmplwi  r0,0              // Test whether r0 is zero
        BugChk  beql              // This was last entry; File was not found
        cmplw   r0,r6             // Do left 4 characters match?
        bne     1f                // Branch if not
        lwz     r1,0x04(r5)       // Load 2nd word of the short name
        subis   r1,r1,SPC@h       // Right 4 filename characters must be spaces
        subic.  r1,r1,SPC@l       // Right 4 filename characters must be spaces
        bne     1f                // Branch if no match
        lwz     r0,0x08(r5)       // Load 3rd word of the short name
        li      r1,0xe7           // Load complement of label and dir attr bits
        andc    r1,r0,r1          // Mask off the don't care attr bits
        subis   r1,r1,SYS@h       // Extension must by .SYS, with attr flag = 0
        subic.  r1,r1,SYS@l       // Extension must by .SYS, with attr flag = 0
        beq     4f                // Branch if we have a match
1:      addi    r5,r5,32          // Advance to the next directory entry
        bdnz    3b                // Go process next entry
        addi    r4,r4,1           // Next sector number
        b       2b                // Process next sector

        // Filename matches
4:      li      r1,0x14           // Offset to high bytes of 1st cluster no.
        lhbrx   r0,r1,r5          // Load high two bytes of 1st cluster no.
        li      r1,0x1a           // Offset to low bytes of 1st cluster no.
        lhbrx   r3,r1,r5          // Load low  two bytes of 1st cluster no.
        insrwi  r3,r0,16,0        // Merge the two halves
        li      r1,0x1c           // Offset to file size
        lwbrx   r4,r1,r5          // Load file size
        mtlr    r12               // Restore return address
        blr


        .eject

        //! readFile - Load a file into memory according to the FAT
        // Bugchecks: None
        // Clobbered: r6,r7,r8,r9,r12,r18,r19
        // @param r3 First cluster number of the file to load
        // @param r4 Size of the file to load
        // @param r5 File load address
        // @param r13 Sector number of the first FAT sector (FAT_Begin_LBA)
        // @param r14 Sectors per cluster
        // @param r15 Sector number of the first Data sector (Data_Begin_LBA)
        // This function produces no return values
sdRdFile:
        mflr    r12               // Save return address
        li      r18,-1            // fatOs = 0
        mr      r19,r4            // Save the file's size
        mr      r9,r5             // Save the file load address
        mr      r8,r3             // Save the file's first cluster number
1:      subis   r1,r8,0x1000
        addic.  r1,r1,8           // if (clNext >= 0x0ffffff8) => EOF
        BugChk  bgel              // Bugcheck if EOF seen prematurely
        lis     r5,SCRATCH_MEMORY // Location to load the FAT sector
        li      r6,0              // clCnt = 0
        mr      r7,r8             // clOs = cl
3:      srwi    r0,r8,9-2         // cl >> (9-2)
        cmplw   r0,r18            // if ((cl >> (9-2)) == fatOs)
        beq     2f                // Jump ahead
        mr      r18,r0            // fatOs = cl >> (9-2)
        add     r4,r13,r18        // FAT_Begin_LBA + fatOs
        bl      sdStBlk1          // Store a FAT sector to memory
2:      andi.   r0,r8,0x7f        // cl & 0x7f
        slwi    r0,r0,2           // Convert to longword offset
        lwbrx   r1,r5,r0          // clNext = swap32(fat[cl & 0x7f])
        addi    r6,r6,1           // ++clCnt
        addi    r8,r8,1           // ++cl
        cmplw   r8,r1             // if (cl == clNext) ...
        beq     3b                // ... go process next entry
        mr      r8,r1             // cl = clNext
        mullw   r3,r6,r14         // clCnt * Sectors_Per_Cluster
        subi    r4,r7,2           // clOs - 2
        mullw   r4,r4,r14         // (clOs - 2) * Sectors_Per_Cluster
        add     r4,r4,r15         // Data_Begin_LBA + (clOs -2) * Sectors_Per_Cluster
        mr      r5,r9             // Where to load the file segment
        bl      sdStBlk           // Store file data
        sub     r0,r3,r9          // Size of chunk that was stored
        mr      r9,r3             // mem += clSz * clCnt
        sub.    r19,r19,r0        // Calculate the amount remaining
        bgt     1b                // Loop if there's more to do
        subis   r1,r8,0x1000
        addic.  r1,r1,8           // if (clNext >= 0x0ffffff8) => EOF
        BugChk  bltl              // Bugcheck if EOF not seen
        mtlr    r12               // Restore return address
        blr                       // Return


.LsdCmd:
        .size   sdCmd,.LsdCmd-sdCmd
