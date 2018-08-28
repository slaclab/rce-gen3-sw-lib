// -*-Mode: gas;-*-
//!@file   display.h
//
// @brief   Macros to access the front panel display
//
// @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
//
// @date    October 28, 2011 -- Created
//
// $Revision: 1358 $
//
// @verbatim                    Copyright 2011
//                                     by
//                        The Board of Trustees of the
//                      Leland Stanford Junior University.
//                             All rights reserved.
// @endverbatim

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "datCode.hh"
#include DAT_GENERATION(bootstrap, common, display.h)

#if !tgt_os_eabi                  // Compiler doesn't support macros
        // Write an immediate value to the display
        .macro  SetDpyi value:req,reg=r30
        li      \reg,\value
        SetDpy  \reg
        .endm

        // Reset the display value using the supplied register
        .macro  RstDpy value=0
        SetDpyi \value
        .endm

        // Increment the display value using the supplied register
        .macro  IncDpy reg:req
        GetDpy  \reg
        addi    \reg,\reg,1
        SetDpy  \reg
        .endm
#endif

#endif
