// -*-Mode: C++;-*-
/**
@file
@brief Inline implementation of class Once for RTEMS.
@verbatim
                               Copyright 2011
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/


#include "interrupt/platform/InterruptGuard.hh"

namespace tool {

  namespace concurrency {

    inline Once::Once() {
      m_data.notdone = true;
    }

    inline void Once::run(void (*func)()) {
      tool::interrupt::InterruptGuard ig;
      if (m_data.notdone) {
	func();
	m_data.notdone = false;
      }
    }

  } // concurrency

} // tool
