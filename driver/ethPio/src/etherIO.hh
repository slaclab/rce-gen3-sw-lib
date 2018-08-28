// -*-Mode: C++;-*-
/*!@file     etherIO.hh
*
* @brief     Class EtherIO definitions
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      June 26, 2012 -- Created
*
* $Revision: 3121 $
*
* @verbatim                    Copyright 2012
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef DRIVER_ETHPIO_ETHERIO_HH
#define DRIVER_ETHPIO_ETHERIO_HH


#include "cpu/apu.hh"

namespace driver {

  namespace ethPio {

    class EtherIO : private tool::cpu::APU
    {
    public:
      EtherIO();
      ~EtherIO();

    public:
      //! Read ethernet response register
      //    cc = udi0fcm. rT,rA,rB
      //      rT receives register value,
      //      rA is ignored,
      //      rB is ignored
      //      cc are the condition codes (return value)
      inline unsigned response(uint32_t* opcode,
                               uint32_t* tid,
                               uint32_t* arg) const;

      //! Wait for and read ethernet response register
      //    1: udi0fcm. rT,rA,rB
      //       bt       3,1b
      //      rT receives register value,
      //      rA is ignored,
      //      rB is ignored
      inline void responseW(uint32_t* opcode,
                            uint32_t* tid,
                            uint32_t* arg) const;

      //! Write ethernet control register
      //    udi0fcm rT,rA,rB
      //      rT is unchanged
      //      rA is ignored,
      //      rB holds register value
      inline void command(const uint32_t opcode,
                          const uint32_t tid,
                          const uint32_t arg) const;

      //! Load to ethernet transmit FIFO
      //     ldfcmx 0,rA,rB
      //      rA holds source base address,
      //      rB is offset
      inline void transmit(const uint64_t* base, const unsigned& offset) const;

      //! Store from ethernet receive FIFO
      //    stdfcmx 0,rA,rB
      //      rA holds destination base address,
      //      rB holds offset
      inline void receive(const uint64_t* base, const unsigned& offset) const;

      //! Load-with-update to ethernet transmit FIFO
      //     ldfcmux 0,rA,rB
      //      rA holds source base address,
      //      rB is offset
      inline uint64_t* transmitU(uint64_t* base, const unsigned& offset) const;

      //! Store-with-update from ethernet receive FIFO
      //    stdfcmux 0,rA,rB
      //      rA holds destination base address,
      //      rB holds offset
      inline uint64_t* receiveU(uint64_t* base, const unsigned& offset) const;
    };

  }

}

#include "ethPio/cpu/etherIO-inl.hh"

#endif
