// -*-Mode: C++;-*-
/**
@file
@brief Declare and implement class InterruptGuard.
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
Stephen Tether <ether@slac.stanford.edu>

@par Date created:
2011/01/26

@par Last commit:
\$Date: 2012-05-23 19:57:06 -0700 (Wed, 23 May 2012) $ by \$Author: claus $.

@par Revision number:
\$Revision: 1069 $

@par Credits:
SLAC
*/
#if !defined(TOOL_INTERRUPT_INTERRUPTGUARD_HH)
#define      TOOL_INTERRUPT_INTERRUPTGUARD_HH

#include <rtems.h>




namespace tool {

  namespace interrupt {

    /// @brief The constructor saves the state of the interrupt
    /// enables then disables interrupts. The destructor restores the
    /// saved state of the interrupt enables.
    class InterruptGuard {
      rtems_interrupt_level m_level;
    public:
      InterruptGuard() __attribute__((always_inline));
      ~InterruptGuard() __attribute__((always_inline));
    };

    inline InterruptGuard::InterruptGuard() {rtems_interrupt_disable(m_level);}

    inline InterruptGuard::~InterruptGuard() {rtems_interrupt_enable(m_level);}

  }; // interrupt
}; // tool
#endif // TOOL_INTERRUPT_INTERRUPTGUARD_HH
