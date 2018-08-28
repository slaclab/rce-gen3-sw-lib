// -*-Mode: C++;-*-
/*!
*
* @file
* @brief Declare and implement functions in sub-namespace Bits.
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    March 30, 2011 -- Created
*
* $Revision: 2961 $
*
* Copyright:                    Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
*/
#ifndef TOOL_DATASTRUCTURE_BITS_HH
#define TOOL_DATASTRUCTURE_BITS_HH




namespace tool
{
  namespace dataStructure
  {

    // __builtin_foo() denotes a GCC-specific built-in function.

    /** @brief Contains various bit manipulation functions implemented using GCC builtins. */
    namespace Bits {
      /** @brief Returns one plus the index of the least significant 1-bit of
          x, or if x is zero, returns zero.


      */
      inline int ffs(unsigned x);
      /** @overload */
      inline int ffs(unsigned long x);
      /** @overload */
      inline int ffs(unsigned long long x);

      /** @brief Returns the number of leading 0-bits in x, starting at the
          most significant bit position. If x is 0, the result is
          undefined.


      */
      inline int clz(unsigned x);
      /** @overload */
      inline int clz(unsigned long x);
      /** @overload */
      inline int clz(unsigned long long x);

      /** @brief Returns the number of trailing 0-bits in x, starting at the
          least significant bit position. If x is 0, the result is
          undefined.


      */
      inline int ctz(unsigned x);
      /** @overload */
      inline int ctz(unsigned long x);
      /** @overload */
      inline int ctz(unsigned long long x);

      /** @brief Returns the number of 1-bits in x.


      */
      inline int popcount(unsigned x);
      /** @overload */
      inline int popcount(unsigned long x);
      /** @overload */
      inline int popcount(unsigned long long x);

      /** @brief Returns the parity of x, i.e. the number of 1-bits in x modulo 2.


      */
      inline int parity(unsigned x);
      /** @overload */
      inline int parity(unsigned long x);
      /** @overload */
      inline int parity(unsigned long long x);

    } // Bits
  } // dataStructure
} // tool


#include "dataStructure/impl/Bits-inl.hh"

#endif // TOOL_DATASTRUCTURE_BITS_HH
