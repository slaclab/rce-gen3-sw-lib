// -*-Mode: gas;-*-
//!@file   bootOpts.h
//
// @brief   Macro to get the boot options from the front panel
//
// @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
//
// @date    November 10, 2011 -- Created
//
// $Revision: 1358 $
//
// @verbatim                    Copyright 2011
//                                     by
//                        The Board of Trustees of the
//                      Leland Stanford Junior University.
//                             All rights reserved.
// @endverbatim

#ifndef _BOOTOPTS_H_
#define _BOOTOPTS_H_

#include "datCode.hh"
#include DAT_PUBLIC(bootstrap, common, dcr.h)

#if !tgt_os_eabi                  // Compiler doesn't support macros
        // Check the boot protocol version
        .macro  BootChk reg:req, ver:req
        // This does nothing
        .endm

        // Get the boot options
        .macro  BootOpt reg:req
        mfdcr   \reg,FLSHOPT
        srwi    \reg,\reg,16
        .endm

        // Get the boot parameters
        .macro BootPrm reg:req
        // This does nothing
        .endm
#else
#error Compiler doesn't support macros
#endif

#endif
