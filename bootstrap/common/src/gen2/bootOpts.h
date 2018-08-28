// -*-Mode: gas;-*-
//!@file   bootOpts.h
//
// @brief   Macros to get the boot options from the front panel
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
#include DAT_GENERATION(bootstrap, common, i2c.h)

#define BS_V_SCHEMA            28
#define BS_N_SCHEMA             4
#define BS_M_SCHEMA    0xf0000000
#define BS_B_SCHEMA           (32 - BS_V_SCHEMA - BS_N_SCHEMA)
#define BS_V_BOOTSTRAP         16
#define BS_N_BOOTSTRAP         12
#define BS_M_BOOTSTRAP 0x0fff0000
#define BS_B_BOOTSTRAP        (32 - BS_V_BOOTSTRAP - BS_N_BOOTSTRAP)
#define BS_V_PARAMS             0
#define BS_N_PARAMS            16
#define BS_M_PARAMS    0x0000ffff
#define BS_B_PARAMS           (32 - BS_V_PARAMS - BS_N_PARAMS)

#if !tgt_os_eabi                  // Compiler doesn't support macros
        // Check the boot protocol version
        .macro  BootChk reg:req, ver:req
        mfi2c.  \reg,I2C_BOOTSTRAP     // Get the bootstrap word
        extrwi  \reg,\reg,BS_N_SCHEMA,BS_B_SCHEMA
        cmpwi   \reg,\ver              // Check whether they match
        BugChk  bnel                   // If not, bugcheck
        .endm

        // Get the boot options
        .macro  BootOpt reg:req
        mfi2c.  \reg,I2C_BOOTSTRAP     // Get the bootstrap word
        extrwi  \reg,\reg,BS_N_BOOTSTRAP,BS_B_BOOTSTRAP
        .endm

        // Get the boot parameters
        .macro BootPrm reg:req
        mfi2c.  \reg,I2C_BOOTSTRAP     // Get the bootstrap word
        extrwi  \reg,\reg,BS_N_PARAMS,BS_B_PARAMS
        .endm
#else
#error Compiler doesn't support macros
#endif

#endif
