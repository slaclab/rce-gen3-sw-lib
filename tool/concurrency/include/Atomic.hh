// -*-Mode: C++;-*-
/*!
* @file
* @brief   Class for performing atomic operations
*
* @verbatim
*                               Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#if !defined(TOOL_CONCURRENCY_ATOMIC_HH)
#define      TOOL_CONCURRENCY_ATOMIC_HH

namespace tool {
  namespace concurrency {


    class Atomic {
    public:
      Atomic(): m_counter(0) {}

      int count ()           const {return m_counter;}
      int add   (int addend)       {return __sync_add_and_fetch(&m_counter, addend);}
      int inc   ()                 {return add(1);}
      int dec   ()                 {return add(-1);}

    private:
      int m_counter;
    };


  } // concurrency
} // tool
#endif // TOOL_CONCURRENCY_ATOMIC_HH
