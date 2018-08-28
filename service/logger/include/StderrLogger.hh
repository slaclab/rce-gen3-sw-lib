// -*-Mode: C++;-*-
/**
@file StderrLogger.hh
@brief Declares the StderrLogger class.
@verbatim
                               Copyright 2010
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
Service

@author
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2011/01/12

@par Revision date:
\$Date: 2014-06-04 17:27:22 -0700 (Wed, 04 Jun 2014) $

@par Revision number:
\$Revision: 3344 $

@par Credits:
SLAC
*/
#if !defined(SERVICE_LOGGER_STDERRLOGGER_HH)
#define      SERVICE_LOGGER_STDERRLOGGER_HH


#include "logger/Logger.hh"
#include "logger/LoggerImpl.hh"

namespace service {

  namespace logger {

    /** @brief Write log messages to stderr (from cstdio). */
    class StderrLogger: public LoggerImpl {

    public:

      StderrLogger();

      /** @brief Deliver a preformatted log message. */
      virtual void send(const char *msg, size_t nbytes);

      /** @brief Should the preformatted messages already have timestamps? */
      virtual bool needTimestamps() const;

      virtual ~StderrLogger();
    };

  }; // logger

}; // service
#endif // SERVICE_LOGGER_STDERRLOGGER_HH
