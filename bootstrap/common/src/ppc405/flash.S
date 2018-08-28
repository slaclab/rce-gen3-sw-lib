// Version 2.0  06-October-2011
//
// Reworked for 2nd generation boards
//
// Version 1.3  09-May-2011
//
// Use eieio to prevent the PPC405 reordering of DCR accesses
//
// Version 1.2  11-January-2007
//
// Interrupt protect flash memory waits for busy to be asserted
//
// Version 1.0  27-December-2006

        .file  "flash.S"

        // Global Constant Definitions
#include "datCode.hh"
#include DAT_PUBLIC(    bootstrap, common, ppc-asm.h)
#include DAT_PUBLIC(    bootstrap, common, dcr.h)
#include DAT_GENERATION(bootstrap, common, bootOpts.h)
#include DAT_PUBLIC(    bootstrap, common, functionStates.h)

#define FN FN_4


        // Global Entry Point Definitions
        .section .text,"ax"

        .global readPage,readPageNoECC,writePage,eraseBlock
        .global readFile

        .eject

        // readPage - Read flash memory page into RAM
readPage:
        li      r6,0x82           // Set word count

        // FLRDPG - Read flash memory page into RAM with specified word count
FLRDPG: li      r0,0x3            // Set function code and busy bit

        // FLPGCM - Common processing for page-oriented functions
FLPGCM: insrwi  r0,r4,5,11        // Merge page number into function code

        // FLBLCM - Common processing for block-oriented functions
FLBLCM: insrwi  r0,r3,11,0        // Merge block number into function code
        mfmsr   r3                // Get machine state
        eieio                     // Any pending accesses to the flash buffer must finish
        wrteei  0                 // Disable interrupts
        isync                     // Make sure I-disable has taken effect
        mtdcr   FLSHCMD,r0        // Initiate flash memory operation
FLCKIN: mfdcr   r0,FLSHCMD        // Get transfer status
        andi.   r0,r0,0x1         // Check for operation initiated
        beq     FLCKIN            // Branch if operation not initiated yet
        State   FN,FS_1           // Update state
        mtmsr   r3                // Restore machine state
FLCKCP: mfdcr   r0,FLSHCMD        // Get transfer status
        andi.   r3,r0,0x1         // Check for operation completed
        bne     FLCKCP            // Branch if operation not completed yet
        State   FN,FS_2           // Update state
        andi.   r3,r0,0x4         // Check for read operation
        extrwi  r3,r0,3,16        // Get operation status
        inslwi  r3,r0,11,5        // Merge block number into status
        bnelr-                    // Return if not read operation
        State   FN,FS_3           // Update state

        //      Read data copying loop
        cmpi    cr0,r6,0          // Check to see if there is data to read (cpo)
        beqlr-                    // Return if there was no data to read (cpo)
        State   FN,FS_4           // Update state

        mfctr   r0                // Save CTR
        mtctr   r6                // Transfer word count to CTR
        la      r5,-4(r5)         // Offset data pointer
        li      r6,0              // Set data block offset
        mtdcr   FLSHOFF,r6        // Initialize flash memory data block offset
        eieio                     // Mustn't read FLSHDAT before the write to FLSHOFF finishes
FLRDLP: mfdcr   r6,FLSHDAT        // Get next user data word from flash memory data block
        stwu    r6,4(r5)          // Store next user data word
        bdnz    FLRDLP            // Loop over user data words
        State   FN,FS_5           // Update state
        mtctr   r0                // Restore CTR
        blr                       // Return

        .eject

        // readPageNoECC - Read flash memory page into RAM without error correction
readPageNoECC:
        li      r6,0x84           // Set word count
        li      r0,0x1            // Set function code and busy bit
        b       FLPGCM            // Join common code

        // writePage - Write flash memory page from RAM
writePage:
        mfctr   r6                // Save CTR

        // Write data copying loop
        li      r0,0x82           // Set word count
        mtctr   r0                // Transfer word count to CTR
        la      r5,-4(r5)         // Offset data pointer
        li      r0,0              // Set data block offset
        mtdcr   FLSHOFF,r0        // Initialize flash memory data block offset
        eieio                     // Mustn't write FLSHDAT before the write to FLSHOFF finishes
FLWRLP: lwzu    r0,4(r5)          // Get next user data word
        mtdcr   FLSHDAT,r0        // Transfer next user data word to flash memory data block
        bdnz    FLWRLP            // Loop over user data words
        State   FN,FS_6           // Update state
        mtctr   r6                // Restore CTR
        li      r0,0x5            // Set function code and busy bit
        b       FLPGCM            // Join common code

        // eraseBlock - Erase flash memory block
eraseBlock:
        li      r0,0x7            // Set function code and busy bit
        b       FLBLCM            // Join common code

        .eject

        // readFile - Read flash memory file into RAM
readFile:
        clrlwi  r3,r3,26          // Extract file number
        addi    r3,r3,0x40        // Add file handle vector offset
        mtdcr   FLSHOFF,r3        // Transfer to flash memory data block offset

        // Read page 0 of block 0 into data block
        li      r0,0x3            // Set function code and busy bit
        mfmsr   r3                // Get machine state
        eieio                     // Any pending accesses to the flash buffer must finish
        wrteei  0                 // Disable interrupts
        isync                     // Make sure I-disable has taken effect
        mtdcr   FLSHCMD,r0        // Initiate flash memory operation
FLRFSC: mfdcr   r0,FLSHCMD        // Get transfer status
        andi.   r0,r0,0x1         // Check for operation initiated
        beq     FLRFSC            // Branch if operation not initiated yet
        State   FN,FS_7           // Update state
        mtmsr   r3                // Restore machine state
FLRFEC: mfdcr   r0,FLSHCMD        // Get transfer status
        andi.   r3,r0,0x1         // Check for operation completed
        bne     FLRFEC            // Branch if operation not completed yet
        State   FN,FS_8           // Update state
        extrwi. r3,r0,3,16        // Get operation status
        bnelr-                    // Return if status error
        State   FN,FS_9           // Update state

        // Recover file handle and initialize for reading first block in file
        mr      r12,r5            // Save the filesize (in longwords) (cpo)
        mr      r5,r4             // Transfer user buffer address
        eieio                     // Don't let the PPC move DCR accesses across this boundary
        mfdcr   r0,FLSHDAT        // Get file handle
        clrlwi. r7,r0,16          // Extract file link
        extrwi  r8,r0,16,0        // Extract file length
        addi    r8,r8,1           // Remove length bias
        clrrwi  r9,r0,16          // Initialize auxiliary status
        mfctr   r10               // Save CTR
        mflr    r11               // Save LR

        // FLRFNB - Read next block in file
FLRFNB: beq     FLRFNL            // Branch if null file link
        State   FN,FS_10          // Update state
        li      r4,0              // Initialize page number
        extrwi  r3,r7,5,16        // Get page range
        addi    r3,r3,1           // Remove page range bias
        cmpw    cr0,r3,r8         // Check for premature length exhaustion
        ble+    FLRFLL            // Branch if no premature length exhaustion
        State   FN,FS_11          // Update state
        ori     r9,r9,0x20        // Set premature length exhaustion auxiliary status flag
        mr      r3,r8             // Use remaining file length
FLRFLL: mtctr   r3                // Transfer block page count to CTR
        sub     r8,r8,r3          // Reduce remaining file length

// cpo: Some pseudo-code to describe the wordcount logic.
// this shortens readpages (or skips them) if the user memory
// area is smaller than an integral number of pages (likely)
// RDFLPG has been changed to accept zero words
// words = fullpage;
// cr0 = (wrdcntreq<=fullpage);
// if (cr0) words = wrdcntreq;
// wrdcntreq-=fullpage;
// if (cr0) wrdcntreq=0;
// readpage(words);

        // FLRFLP - Page read loop
FLRFLP: clrlwi  r3,r7,21          // Extract block number
        li      r6,0x80           // Set default FLRDPG words to read out a full-page
        cmpw    cr0,r12,r6        // See if requested wordcount is >= a full page (cpo)
        bgt     FULLPG            // Don't touch FLRDPG words (cpo)
        State   FN,FS_12          // Update state
        mr      r6,r12            // Set FLRDPG words to be remaining req wordcount (cpo)
FULLPG: subi    r12,r12,0x80      // Reduce the requested word count by a page (cpo)
        bgt     NOTEND            // Requested word count not low enough yet (cpo)
        State   FN,FS_13          // Update state
        li      r12,0             // Make the word count for next time zero (cpo)
NOTEND: bl      FLRDPG            // Read the next page
        State   FN,FS_14          // Update state
        clrlwi. r0,r3,29          // Check operation status
        bne     FLRFEX            // Branch if status error
        State   FN,FS_15          // Update state
        addi    r4,r4,1           // Update page number
        la      r5,4(r5)          // Update data pointer
        bdnz    FLRFLP            // Loop over pages in block
        State   FN,FS_16          // Update state

        // End-of-block processing
        li      r0,0x81           // Set file link offset
        mtdcr   FLSHOFF,r0        // Transfer file link offset to memory data block offset
        eieio                     // Let the write to FLSHOFF finish
        mfdcr   r0,FLSHDAT        // Get file link
        li      r4,0              // Clear register
        cmpw    cr1,r8,r4         // Check remaining file length
        clrlwi. r7,r0,16          // Extract file link
        bne     cr1,FLRFNB        // Branch if file length not exhausted
        State   FN,FS_17          // Update state
        beq+    FLRFFC            // Branch if null file link
        State   FN,FS_18          // Update state
        ori     r3,r3,0x10        // Set missing null file link error

        // FLRFFC - Data cache flush initialization

        // We assume here that the file we just read
        // is a system image and hence executable.
        // The I-cache can't tap the D-cache so we must
        // flush to main memory. We also assume that no
        // code has been run from main RAM since sysreset
        // so we can skip the I-cache invalidation.
        // Note that cache flushing would be undesirable
        // in a general purpose routine since it would
        // slow down access to non-executable data.

FLRFFC: rlwinm  r8,r9,25,7,22     // Recover file length in bytes
        addi    r8,r8,0x200       // Remove length bias
        sub     r5,r5,r8          // Recover starting user buffer address
        srwi    r8,r8,5           // Convert length to cache line count
        clrlwi. r0,r5,27          // Check starting offset in cache line
        beq+    FLRFFI            // Branch if starting offset cache line aligned
        State   FN,FS_19          // Update state
        addi    r8,r8,1           // Add extra cache line for non-alignment
FLRFFI: mtctr   r8                // Transfer cache line count to CTR
        li      r6,0              // Initialize user buffer offset

        // FLRFFL - Data cache flush loop
FLRFFL: dcbf    r5,r6             // Flush a cache line
        addi    r6,r6,0x20        // Update user buffer offset
        bdnz    FLRFFL            // Loop over cache lines in user buffer
        State   FN,FS_20          // Update state

        // FLRFEX - Normal fileRead exit processing
FLRFEX: mtlr    r11               // Restore LR
        mtctr   r10               // Restore CTR
        clrlwi  r9,r9,16          // Extract auxiliary status
        or      r3,r3,r9          // Merge auxiliary status
        State   FN,FS_21          // Update state
        blr                       // Return

        // Error processing
FLRFNL: ori     r3,r3,0x8         // Set unexpected null file link error
        b       FLRFEX            // Join common code


.Lreadpage:
        .size   readPage,.Lreadpage-readPage
