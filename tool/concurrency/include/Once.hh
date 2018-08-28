// -*-Mode: C++;-*-
/**
   @file
   @verbatim
                               Copyright 2011
                                     by
                        The Board of Trustees of the
                      Leland Stanford Junior University.
                             All rights reserved.
   @endverbatim
*/

#if !defined(TOOL_CONCURRENCY_ONCE_HH)
#define      TOOL_CONCURRENCY_ONCE_HH


#include "concurrency/os/OnceData.hh"

namespace tool {

  namespace concurrency {

    class Once {
    public:

      Once();

      void run(void (*func)());

    private:

      OnceData m_data;

    };

  }; // concurrency

}; // tool

#include "concurrency/os/Once-inl.hh"

#endif // TOOL_CONCURRENCY_ONCE_HH
