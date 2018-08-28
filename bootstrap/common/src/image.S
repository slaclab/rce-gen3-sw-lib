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
#include DAT_PUBLIC(bootstrap, common, ppc-asm.h)
#include DAT_PUBLIC(bootstrap, common, params.h)
#include DAT_PUBLIC(bootstrap, common, functionStates.h)
#include DAT_PUBLIC(bootstrap, common, bugcheck.h)

#define FN FN_7


        // Global Entry Point Definitions
        .section .text,"ax"

        //! imageTest - Test whether there is an image already in memory
        .global imageTest
imageTest:
        State   FN,FS_1           // Update state
        andi.   r0,r11,0x1f       // Is the load address 32 byte aligned?
        bnelr                     // Return if not
        State   FN,FS_2           // Update state
        mflr    r26               // Save the link Register
        bl      ElfTest           // Check the magic number
        State   FN,FS_3           // Update state
        mtlr    r26               // Restore the link register
        blr
