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

#ifndef _GEN1_DISPLAY_H_
#define _GEN1_DISPLAY_H_

#include "datCode.hh"
#include DAT_PUBLIC(bootstrap, common, dcr.h)

#if !tgt_os_eabi                  // Compiler doesn't support macros
        //! Write a value in a register to the display
        .macro  SetDpy reg:req
        mtdcr   LEDREG,\reg
        .endm

        //! Read a value from the display into the supplied register
        .macro  GetDpy reg:req
        mfdcr   \reg,LEDREG
        .endm
#else   // Some syntactically correct nonsense that allows things to compile
#define SetDpy mtdcr LEDREG,
#define GetDpy mfpid
#endif

#endif
