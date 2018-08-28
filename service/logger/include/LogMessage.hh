// -*-Mode: C++;-*-
/**
   @file LogMessage.hh
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
   Declares class LogMessage.

   @author
   Stephen Tether <tether@slac.stanford.edu>

   @par Date created:
   2010/03/25

   @par Last commit:
   \$Date: 2014-06-04 17:27:22 -0700 (Wed, 04 Jun 2014) $ by \$Author: panetta $.

   @par Revision number:
   \$Revision: 3344 $

   @par Credits:
   SLAC
*/


#if !defined(SERVICE_LOGGER_LOGMESSAGE_HH)
#define      SERVICE_LOGGER_LOGMESSAGE_HH



#include "logger/Logger.hh"
#include "string/StringBuffer.hh"

namespace service {

  namespace logger {

    /// @brief Assembles a log message in a buffer.
    ///
    /// Non-copyable.
    /// @see Logger
    class LogMessage {
    public:
      /** @brief Return the initial number of bytes allocated for each message. */
      static size_t initialBufferSize();

      /** @brief Set the initial number of bytes allocated for each message. */
      static void initialBufferSize(size_t s);

      /** @brief Determine whether the message will be sent. */
      LogMessage(Logger::Severity);

      /** @brief Send the accumulated message string. */
      ~LogMessage();

      /// @brief Format and append new content to the existing message string.
      /// @return A reference to this object so that adds may be chained.
      LogMessage& add(const char *format, ...);

      /// @brief Add with formatting using an already initialized va_list.
      /// @return A reference to this object so that adds may be chained.
      LogMessage& vadd(const char *format, std::va_list);

      /// @brief Add a string directly, unformatted.
      /// @return A reference to this object so that adds may be chained.
      LogMessage& uadd(const char *more);

    private:

      static size_t m_initialBufferSize;

      const bool    m_willSend;

      tool::string::StringBuffer  m_buffer;
    };

  };  // logger
}; // service

#include "logger/impl/LogMessage-inl.hh"

#endif // SERVICE_LOGGER_LOGMESSAGE_HH
