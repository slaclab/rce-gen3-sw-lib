// -*-Mode: C++;-*-
/**
@file
@brief Declares the SemaphoreData class for RTEMS.
@verbatim
                               Copyright 2010
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#if !defined(TOOL_CONCURRENCY_SEMAPHOREDATA_HH)
#define      TOOL_CONCURRENCY_SEMAPHOREDATA_HH

#include <rtems.h>

namespace tool {

  namespace concurrency {

    struct SemaphoreData {
      rtems_id m_semId;
    };

  } // concurrency

} // tool

#endif // TOOL_CONCURRENCY_SEMAPHOREDATA_HH
