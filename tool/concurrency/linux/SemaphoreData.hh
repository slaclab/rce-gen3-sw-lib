// -*-Mode: C++;-*-
/**
@file
@brief Declares the SemaphoreData class for Linux.
@verbatim
                               Copyright 2010
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
DAT

@author
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2010/12/02

@par Credits:
SLAC
*/
#if !defined(TOOL_CONCURRENCY_SEMAPHOREDATA_HH)
#define      TOOL_CONCURRENCY_SEMAPHOREDATA_HH

#include "pth.h"

namespace tool {

  namespace concurrency {

    /** @cond development */
    struct SemaphoreData {
      pth_mutex_t m_mutex;
    };
    /** @endcond */

  } // concurrency

} // tool
#endif //TOOL_CONCURRENCY_SEMAPHOREDATA_HH
