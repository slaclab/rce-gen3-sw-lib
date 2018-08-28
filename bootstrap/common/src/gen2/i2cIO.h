// -*-Mode: gas;-*-
/*!@file   i2cIO.h
*
* @brief   Definition of I2C I/O macros for Gen 2 Virtex 5 devices
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    February 28, 2012 -- Created
*
* $Revision: 1358 $
*
* @verbatim                    Copyright 2012
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef _I2C_IO_H_
#define _I2C_IO_H_

#include "datCode.hh"
#include DAT_PUBLIC(configuration, apu, apu.hh)

#define i2cRd  UDIFCM(APU_I2C_BSI_READ)
#define i2cWrt UDIFCM(APU_I2C_BSI_WRITE)

#if !tgt_os_eabi                  // Compiler doesn't support macros
        //! Move from I2C response FIFO
        .macro  mfi2c. reg:req, idx:req
        li      \reg,\idx
        i2cRd.  \reg,\reg,\reg    // Third argument (rB) is ignored
#if tgt_board_rce405 || tgt_board_cm405
        eieio
#endif
        .endm

        //! Move to I2C command FIFO
        .macro  mti2c reg:req, idx:req
        li      \reg,\idx
        i2cWrt  \reg,\reg,\reg    // First argument (rT) is ignored
#if tgt_board_rce405 || tgt_board_cm405
        eieio
#endif
        .endm
#else
#error Compiler doesn't support macros
#endif

#undef i2cRd
#undef i2cWrt

#endif
