// -*-Mode: C++;-*-
/**
@file
@verbatim
                               Copyright 2010
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
Service

@par Abstract:
Declares the SyslogLogger class.

@author
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2011/01/12

@par Revision date:
\$Date: 2011-04-22 11:53:40 -0700 (Fri, 22 Apr 2011) $

@par Revision number:
\$Revision: 25 $

@par Credits:
SLAC
*/
#if !defined(SERVICE_LOGGER_SYSLOGLOGGER_HH)
#define      SERVICE_LOGGER_SYSLOGLOGGER_HH


#include "logger/Logger.hh"
#include "logger/LoggerImpl.hh"

namespace service {

  namespace logger {

    /** @brief Write log messages to the in-memory log. */
    class SyslogLogger: public LoggerImpl {

    public:

      SyslogLogger();

      /** @brief Deliver a preformatted log message. */
      virtual void send(const char *msg, size_t nbytes);

      /** @brief Should the preformatted messages already have timestamps? */
      virtual bool needTimestamps() const;

      virtual ~SyslogLogger();
    };

  }; // logger

}; // service
#endif // SERVICE_LOGGER_SYSLOGLOGGER_HH
