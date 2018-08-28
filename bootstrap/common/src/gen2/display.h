// -*-Mode: gas;-*-
//!@file   display.h
//
// @brief   Macros to access the front panel display
//
// @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
//
// @date    October 28, 2011 -- Created
//
// $Revision: 1352 $
//
// @verbatim                    Copyright 2011
//                                     by
//                        The Board of Trustees of the
//                      Leland Stanford Junior University.
//                             All rights reserved.
// @endverbatim

#ifndef _GEN2_DISPLAY_H_
#define _GEN2_DISPLAY_H_

#if !tgt_os_eabi                  // Compiler doesn't support macros
        // Write a value in a register to the display
        .macro  SetDpy reg:req
        // Do nothing for now
        .endm

        // Read a value from the display into the supplied register
        .macro  GetDpy reg:req
        // Do nothing for now
        .endm
#endif

#endif
