// -*-Mode: gas;-*-
/*!@file   sdIO.h
*
* @brief   Definition of SD-card I/O macros
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    October 23, 2012 -- Created
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
#ifndef _SD_IO_H_
#define _SD_IO_H_

#include "datCode.hh"
#include DAT_PUBLIC(configuration, apu, apu.hh)

#define sdRd  UDIFCM(APU_SD_READ)
#define sdWrt UDIFCM(APU_SD_WRITE)
#define sdRst UDIFCM(APU_SD_RESET)

#if !tgt_os_eabi                  // Compiler doesn't support macros
        //! Move from SD response FIFO and set condition register
        // @param val Value read from the response register
        .macro  mfsd. val:req
        sdRd.   \val,\val,\val    // Second and third argument (rB) are ignored
#if tgt_board_rce405 || tgt_board_cm405
        eieio
#endif
        .endm

        //! Move to SD command FIFO
        // @param opc Opcode and TID
        // @param arg Command argument
        .macro  mtsd opc:req,arg:req
        sdWrt   \opc,\opc,\arg    // First argument (rT) is ignored
#if tgt_board_rce405 || tgt_board_cm405
        eieio
#endif
        .endm

        //! Reset SD firmware
        .macro  sdReset
        sdRst   r0,r0,r0          // Register is ignored
#if tgt_board_rce405 || tgt_board_cm405
        eieio
#endif
        .endm

        //! Load from memory to SD transmit FIFO
        // @param base Base address of block to load from
        // @param offset Offset from base address
        // Note that \base and \offset are added prior to instruction execution!
        .macro  ldsd base:req,offset:req
        ldfcmx  APU_SD_LOAD,\base,\offset
        .endm

        //! Store from SD receive FIFO to memory
        // @param base Base address of block to load from
        // @param offset Offset from base address
        // Note that \base and \offset are added prior to instruction execution!
        .macro  stsd base:req,offset:req
        stdfcmx APU_SD_STORE,\base,\offset
        .endm

        //! Load memory with update to SD transmit FIFO
        // @param base Base address of block to load from
        // @param offset Offset from base address
        // Note that \base and \offset are added prior to instruction execution!
        .macro  ldsdu base:req,offset:req
        ldfcmux APU_SD_LOAD,\base,\offset
        .endm

        //! Store from SD receive FIFO to memory with update
        // @param base Base address of block to load from
        // @param offset Offset from base address
        // Note that \base and \offset are added prior to instruction execution!
        .macro  stsdu base:req,offset:req
        stdfcmux APU_SD_STORE,\base,\offset
        .endm
#else
#error Compiler doesn't support macros
#endif

#undef sdRd
#undef sdWrt
#undef sdRst

#endif
