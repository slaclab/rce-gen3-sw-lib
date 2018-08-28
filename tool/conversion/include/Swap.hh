// -*-Mode: C++;-*-
/*!
* @file
* @brief   Define some byte swapping functions
*
* @author  Anonymous -- REG/DRD - (someone@slac.stanford.edu)
*
* @date    March 29, 2011 -- Created
*
* $Revision: 3096 $
*
* @verbatim
*                               Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef TOOL_CONVERSION_SWAP_HH
#define TOOL_CONVERSION_SWAP_HH
#include <stdint.h>

namespace tool {
  namespace conversion {
    inline uint16_t swap16(uint16_t x);
    inline uint32_t swap32(uint32_t x);
    inline uint64_t swap64(uint64_t x);

    inline uint16_t le2h16(uint16_t x);
    inline uint32_t le2h32(uint32_t x);
    inline uint64_t le2h64(uint64_t x);

    inline uint16_t be2h16(uint16_t x);
    inline uint32_t be2h32(uint32_t x);
    inline uint64_t be2h64(uint64_t x);

    inline uint16_t h2le16(uint16_t x);
    inline uint32_t h2le32(uint32_t x);
    inline uint64_t h2le64(uint64_t x);

    inline uint16_t h2be16(uint16_t x);
    inline uint32_t h2be32(uint32_t x);
    inline uint64_t h2be64(uint64_t x);
  } // conversion
} // tool


#include "conversion/impl/Swap-inl.hh"

#endif // TOOL_CONVERSION_SWAP_HH
