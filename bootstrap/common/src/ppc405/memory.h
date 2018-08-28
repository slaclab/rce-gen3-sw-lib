// -*-Mode: gas;-*-
//!@file   memory.h
//
// @brief   Macros to verify memory operation status
//
// @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
//
// @date    February 2, 2012 -- Created
//
// $Revision: 1358 $
//
// @verbatim                    Copyright 2012
//                                     by
//                        The Board of Trustees of the
//                      Leland Stanford Junior University.
//                             All rights reserved.
// @endverbatim

#ifndef _PPC405_MEMORY_H_
#define _PPC405_MEMORY_H_

#include "datCode.hh"
#include DAT_PUBLIC(bootstrap, common, ppc-asm.h)
#include DAT_PUBLIC(bootstrap, common, dcr.h)
#include DAT_PUBLIC(bootstrap, common, bugcheck.h)


        // Initialize memory operation status
        .macro  MemStRst rg1:req,rg2:req
        lis     \rg2,0x00FF       // A mask to test for double bit errors: 0x00ff0000
        mtdcr   MEMERST,\rg1      // Clear the memory status register
        .endm

        // Test memory operation status and bugcheck if bad
        .macro  MemStTst rg1:req,rg2:req
        mfdcr   \rg1,MEMERST      // Get accumulated RLDRAM read error status
        and.    \rg1,\rg1,\rg2    // Set up condition codes
        BugChk  bnel              // Bugcheck if result is non-zero
        .endm

#endif
