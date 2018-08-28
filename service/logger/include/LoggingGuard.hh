// -*-Mode: C++;-*-
/**
@file LoggingGuard.hh
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
Declares the LoggingGuard class.

@author
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2011/01/27

@par Revision date:
\$Date: 2014-06-04 17:27:22 -0700 (Wed, 04 Jun 2014) $

@par Revision number:
\$Revision: 3344 $

@par Credits:
SLAC
*/
#if !defined(SERVICE_LOGGER_LOGGINGGUARD_HH)
#define      SERVICE_LOGGER_LOGGINGGUARD_HH


#include "logger/Logger.hh"

namespace service {

  namespace logger {

    class LoggerImpl;

    /** @brief Push a new logging severity threshold, keeping the current LoggerImpl.
        @param[in] sev The severity threshold.
    */
    class  LoggingGuard {

      /** @brief The saved logging theshold of the creating thread. */
      const Logger::Severity m_threshold;

      /** @brief The saved logger implementation of the creating thread. */
      const LoggerImplPtr m_loggerImpl;

    public:
      explicit LoggingGuard(Logger::Severity sev);

      /** @brief Push a new logging state.
	  @param[in] sev The severity threshold.
	  @param[in] impl The pointer to the new LoggerImpl.
      */
      LoggingGuard(Logger::Severity sev, const LoggerImplPtr& impl);

      /** @brief Pop the current logging state, restoring the previous state. */
      ~LoggingGuard();
    };

  } // logger

} // service

#include "logger/impl/LoggingGuard-inl.hh"
#endif // SERVICE_LOGGER_LOGGINGGUARD_HH
