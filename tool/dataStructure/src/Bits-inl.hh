// -*-Mode: C++;-*-
/*!
* @file
* @brief   Various bit counting functions.
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    March 30, 2011 -- Created
*
* $Revision: 1475 $
*
* Copyright:                    Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
*/

namespace tool
{
  namespace dataStructure
  {

    // __builtin_foo() denotes a GCC-specific built-in function.

    namespace Bits {
      //! Returns one plus the index of the least significant 1-bit of
      //x, or if x is zero, returns zero.
      inline int ffs(unsigned x)                 { return __builtin_ffs(x);   }
      inline int ffs(unsigned long x)            { return __builtin_ffsl(x);  }
      inline int ffs(unsigned long long x)       { return __builtin_ffsll(x); }

      //! Returns the number of leading 0-bits in x, starting at the
      //most significant bit position. If x is 0, the result is
      //undefined.
      inline int clz(unsigned x)                 { return __builtin_clz(x);   }
      inline int clz(unsigned long x)            { return __builtin_clzl(x);  }
      inline int clz(unsigned long long x)       { return __builtin_clzll(x); }

      //! Returns the number of trailing 0-bits in x, starting at the
      //least significant bit position. If x is 0, the result is
      //undefined.
      inline int ctz(unsigned x)                 { return __builtin_ctz(x);   }
      inline int ctz(unsigned long x)            { return __builtin_ctzl(x);  }
      inline int ctz(unsigned long long x)       { return __builtin_ctzll(x); }

      //! Returns the number of 1-bits in x.
      inline int popcount(unsigned x)            { return __builtin_popcount(x);   }
      inline int popcount(unsigned long x)       { return __builtin_popcountl(x);  }
      inline int popcount(unsigned long long x)  { return __builtin_popcountll(x); }

      //! Returns the parity of x, i.e. the number of 1-bits in x modulo 2.
      inline int parity(unsigned x)              { return __builtin_parity(x);   }
      inline int parity(unsigned long x)         { return __builtin_parityl(x);  }
      inline int parity(unsigned long long x)    { return __builtin_parityll(x); }

    } // Bits
  } // dataStructure
} // tool
