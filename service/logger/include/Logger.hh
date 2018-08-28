// -*-Mode: C++;-*-
/**
@file Logger.hh
@brief Declares the Logger class.
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
2009/12/19

@par Revision date:
\$Date: 2014-06-04 17:27:22 -0700 (Wed, 04 Jun 2014) $

@par Revision number:
\$Revision: 3344 $

@par Credits:
SLAC
*/
#if !defined(SERVICE_LOGGER_LOGGER_HH)
#define      SERVICE_LOGGER_LOGGER_HH

#include <tr1/memory>

#include <cstdarg>
#include <stdexcept>


#include "type/EnumInfo.hh"
#include "logger/LoggerImpl.hh"
#include "concurrency/Semaphore.hh"

namespace service {

  namespace logger {

    // Make use of the shared_ptr<> "smart pointer" described in C++ Technical Report 1, ISO/IEC TR 19768.
    typedef std::tr1::shared_ptr<LoggerImpl> LoggerImplPtr;

    class LoggerImpl;

    /// @brief Handle log messages from applications.
    ///
    /// All logging is delegated to a per-Thread pointer to LoggerImpl
    /// which is set when the Thread is created.  The severity
    /// threshold and LoggerImpl pointer are either inherited from the
    /// parent Thread or are set to defaults of Info and
    /// SyslogLogger*.  The threshold and logging implementation can
    /// be changed inside a scope by creating an instance of
    /// LoggingGuard.
    /// @see service::concurrency::Thread
    ///
    /// Instances of Logger have no state of their own so you can just
    /// create them as you need them, e.g.:
    /// @code
    /// Logger().info("My %s has no nose.", pet);
    ///
    /// LogMessage(Logger::Info).add("How does ").add("he smell?");
    ///
    /// { LogMessage msg(Logger::Info);
    ///   msg.add("Awful!");
    /// }
    /// @endcode
    /// Messages with severity less than the current threshold are
    /// never formatted or sent. Your formats need not end with a
    /// newline because one is always appended to the end of a
    /// message.
    ///
    /// The member functions log(), debug(), info(), etc., each create
    /// and send a single message.  The constructor of LogMessage
    /// creates a message to which you can add piecemeal.  Such a
    /// message is sent when the LogMessage object is destroyed.  In
    /// the second example above that happens at the semicolon. In the
    /// third example it happens at the right brace.
    ///
    /// This class' design is a variation on the Flyweight pattern in
    /// that there is only one state which all instances share. In
    /// Python programming this is called the Borg pattern.
    class Logger {
    public:

      /// @brief The severity levels for log messages.
      ///
      enum Severity {Debug, Info, Warning, Error, Fatal};

      /// @brief Do we need to add a timestamp to each message?
      ///
      /// A timestamp may be added by the system, depending on the
      /// destination of the messages, e.g., a Unix syslog via the network.
      bool needTimestamps() const;

      /** @brief Send a message that's a preformatted string. */
      void logString(Severity, const char *msg);

      /** @brief Log with an adjustable severity. */
      void log(Severity, const char *format, ...);

      /// @brief Use this form when you have a va_list already initialized.
      void vlog(Severity, const char *format, std::va_list ap);

      /** @brief Log with severity Debug. */
      void debug(const char *format, ...);

      /** @brief Log with severity Info. */
      void info(const char *format, ...);

      /** @brief Log with severity Warning. */
      void warning(const char *format, ...);

      /** @brief Log with severity Error. */
      void error(const char *format, ...);

      /** @brief Log with severity Fatal. */
      void fatal(const char *format, ...);

    private:

      friend class LogMessage;

      void send(const char *msg, size_t nbytes);

    };

  } // logger

} // Service

namespace tool {
  namespace type {
    using namespace service::logger;
    // Make sure that emin(), emax(), etc., will work for Severity.
    template<> struct EnumInfo<Logger::Severity> {
      static const Logger::Severity min = Logger::Debug;
      static const Logger::Severity max = Logger::Fatal;
      static const size_t count = 1 + max - min;
      static Logger::Severity next(Logger::Severity s) {return s == max ? min : Logger::Severity(s+1);}
      static Logger::Severity prev(Logger::Severity s) {return s == min ? max : Logger::Severity(s-1);}
      static const char* tostr(Logger::Severity s) {
        static const char* name[count] = {"DEBUG", "INFO", "WARNING", "ERROR", "FATAL"};
        return name[s];
      }
    };
  } // string
} // tool
#endif // SERVICE_LOGGER_LOGGER_HH
