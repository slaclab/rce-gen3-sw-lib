// -*-Mode: C++;-*-
/**
@file LoggingGuard-inl.hh
@verbatim
                               Copyright 2011
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
Service

@par Abstract:
Defines the inlines for class LoggingGuard.

@author
Steve Tether <tether@slac.stanford.edu>

@par Date created:
2011/01/27

@par Last commit:
\$Date: 2014-06-04 17:27:22 -0700 (Wed, 04 Jun 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3344 $

@par Credits:
SLAC
*/


#include "concurrency/Thread.hh"

namespace service {

  namespace logger {

    inline LoggingGuard::LoggingGuard(Logger::Severity sev)
      : m_threshold(tool::concurrency::Thread::currentThread().logThreshold()),
	m_loggerImpl()
    {
      tool::concurrency::Thread::currentThread().logThreshold(sev);
    }

    inline LoggingGuard::LoggingGuard(Logger::Severity sev, const LoggerImplPtr& impl)
      : m_threshold(tool::concurrency::Thread::currentThread().logThreshold()),
	m_loggerImpl(tool::concurrency::Thread::currentThread().loggerImpl())
    {
      tool::concurrency::Thread::currentThread().logThreshold(sev);
      tool::concurrency::Thread::currentThread().loggerImpl(impl);
    }

    inline LoggingGuard::~LoggingGuard() {
      tool::concurrency::Thread::currentThread().logThreshold(m_threshold);
      if (m_loggerImpl.get()) tool::concurrency::Thread::currentThread().loggerImpl(m_loggerImpl);
    }
  }; // logger

}; // tool
