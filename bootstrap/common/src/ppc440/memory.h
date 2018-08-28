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

#ifndef _PPC440_MEMORY_H_
#define _PPC440_MEMORY_H_

        // Initialize memory operation status
        .macro  MemStRst rg1:req,rg2:req
        // Do nothing for now
        .endm

        // Test memory operation status and bugcheck if bad
        .macro  MemStTst rg1:req,rg2:req
        // Do nothing for now
        .endm

#endif
