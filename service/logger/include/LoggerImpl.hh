// -*-Mode: C++;-*-
/**
@file LoggerImpl.hh
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
Declares class LoggerImpl.

@author
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2009/12/19

@par Revision date:
\$Date: 2014-06-04 17:27:22 -0700 (Wed, 04 Jun 2014) $

@par Revision number:
\$Revision: 3344 $

@par Credits:
SLAC
*/
#if !defined(SERVICE_LOGGER_LOGGERIMPL_HH)
#define      SERVICE_LOGGER_LOGGERIMPL_HH

#include <cstdarg>

#include <tr1/memory>

namespace service {

  namespace logger {

    /// @brief Abstract base class for logging implementations.
    /// @see Logger
    class LoggerImpl {
    public:

      /// @brief Deliver a log message, already formatted.
      /// @param[in] msg The message to send. Need not be null terminated.
      /// @param[in] nbytes The number of bytes to send (not counting any terminating null).
      virtual void send(const char *msg, size_t nbytes) = 0;

      /** @brief Should messages already have timestamps? */
      virtual bool needTimestamps() const = 0;

      virtual ~LoggerImpl() {};

    };

  } // logger

} // service

#endif // SERVICE_LOGGER_LOGGERIMPL_HH
