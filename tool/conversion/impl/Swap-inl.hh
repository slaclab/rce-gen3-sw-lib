// -*-Mode: C++;-*-
/*!
*
* @brief   Implement some byte swapping functions
*
* @author  Jim Panetta -- REG/DRD - (panetta@slac.stanford.edu)
*
* @date    March 29, 2011 -- Created
*
* $Revision: 3125 $
*
* @verbatim
*                               Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/


// GCC as of version 4.3 has builtin, optimized functions for byte swapping.
// Prior to 4.3, we have to use our own code.
#define GCC_HAS_BUILTIN_SWAP (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))


#include "conversion/BSWP.h"
#include "conversion/impl/BSWP.ih"

namespace tool {
  namespace conversion {

    //! Unconditional word swapping routines
    inline uint16_t swap16(uint16_t x)
    {
      return BSWP__swap16(x);
    }

    inline uint32_t swap32(uint32_t x)
    {
#if GCC_HAS_BUILTIN_SWAP
      return __builtin_bswap32(x);
#else
      return BSWP__swap32(x);
#endif
    }

    inline uint64_t swap64(uint64_t x)
    {
#if GCC_HAS_BUILTIN_SWAP
      return __builtin_bswap64(x);
#else
      return BSWP__swap64(x);
#endif
    }

    //! Unconditional array swapping routines
    inline void swap16N(uint16_t* dst, const uint16_t* src, int cnt)
    {
      BSWP_swap16N(dst, src, cnt);
    }

    inline void swap32N(uint32_t* dst, const uint32_t* src, int cnt)
    {
      BSWP_swap32N(dst, src, cnt);
    }

    inline void swap64N(uint64_t* dst, const uint64_t* src, int cnt)
    {
      BSWP_swap64N(dst, src, cnt);
    }

    //! Swap word to/from little endian from/to host endian routines
    inline uint16_t swap16l(uint16_t x)
    {
      return BSWP__swap16l(x);
    }

    inline uint32_t swap32l(uint32_t x)
    {
      return BSWP__swap32l(x);
    }

    inline uint64_t swap64l(uint64_t x)
    {
      return BSWP__swap64l(x);
    }

    //! Swap array to/from little endian from/to host endian routines
    inline void swap16lN(uint16_t* dst, const uint16_t* src, int cnt)
    {
      BSWP_swap16lN(dst, src, cnt);
    }

    inline void swap32lN(uint32_t* dst, const uint32_t* src, int cnt)
    {
      BSWP_swap32lN(dst, src, cnt);
    }

    inline void swap64lN(uint64_t* dst, const uint64_t* src, int cnt)
    {
      BSWP_swap64lN(dst, src, cnt);
    }

    //! Swap word to/from big endian from/to host endian routines
    inline uint16_t swap16b(uint16_t x)
    {
      return BSWP__swap16b(x);
    }

    inline uint32_t swap32b(uint32_t x)
    {
      return BSWP__swap32b(x);
    }

    inline uint64_t swap64b(uint64_t x)
    {
      return BSWP__swap64b(x);
    }

    //! Swap array to/from big endian from/to host endian routines
    inline void swap16bN(uint16_t* dst, const uint16_t* src, int cnt)
    {
      BSWP_swap16bN(dst, src, cnt);
    }

    inline void swap32bN(uint32_t* dst, const uint32_t* src, int cnt)
    {
      BSWP_swap32bN(dst, src, cnt);
    }

    inline void swap64bN(uint64_t* dst, const uint64_t* src, int cnt)
    {
      BSWP_swap64bN(dst, src, cnt);
    }

  }
}
