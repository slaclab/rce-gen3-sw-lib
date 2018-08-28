// -*-Mode: C++;-*-
/*!@file     sdIO.hh
*
* @brief     Class SdIO definitions
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      August 21, 2012 -- Created
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
#ifndef DRIVER_SD_SDIO_HH
#define DRIVER_SD_SDIO_HH


#include "cpu/apu.hh"

namespace driver {

  namespace sd {

    class SdIO : private tool::cpu::APU
    {
    public:
      SdIO();
      ~SdIO();

    public:
#if 0                     // @todo - Need to find the bit number to hit
      //! Reset the SD firmware and wait for its completion
      inline void reset() const;
#endif

      //! Read SD response register
      //    cc = udi0fcm. rT,rA,rB
      //      rT receives register value,
      //      rA is ignored,
      //      rB is ignored
      //      cc are the condition codes (return value)
      inline uint32_t response(uint32_t* opcode,
                               uint32_t* tid,
                               uint32_t* tmo,
                               uint32_t* arg) const;

      //! Wait for and read SD response register
      //    1: udi0fcm. rT,rA,rB
      //       bt       3,1b
      //      rT receives register value,
      //      rA is ignored,
      //      rB is ignored
      inline void responseW(uint32_t* opcode,
                            uint32_t* tid,
                            uint32_t* tmo,
                            uint32_t* arg) const;

      //! Write SD control register
      //    udi0fcm rT,rA,rB
      //      rT is unchanged
      //      rA is ignored,
      //      rB holds register value
      inline void command(const uint32_t opcode,
                          const uint32_t tid,
                          const uint32_t arg) const;

      //! Load to SD transmit FIFO
      //     ldfcmx 0,rA,rB
      //      rA holds source base address,
      //      rB is offset
      inline void transmit(const uint64_t* base, const unsigned& offset) const;

      //! Store from SD receive FIFO
      //    stdfcmx 0,rA,rB
      //      rA holds destination base address,
      //      rB holds offset
      inline void receive(const uint64_t* base, const unsigned& offset) const;

      //! Load-with-update to SD transmit FIFO
      //     ldfcmux 0,rA,rB
      //      rA holds source base address,
      //      rB is offset
      inline uint64_t* transmitU(uint64_t* base, const unsigned& offset) const;

      //! Store-with-update from SD receive FIFO
      //    stdfcmux 0,rA,rB
      //      rA holds destination base address,
      //      rB holds offset
      inline uint64_t* receiveU(uint64_t* base, const unsigned& offset) const;
    };

  }

}

#include "sd/cpu/sdIO-inl.hh"

#endif
