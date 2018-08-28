// -*-Mode: C++;-*-
/**
@file Logger.cc
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
Implements Logger::log().

@author
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2010/02/11

@par Credits:
SLAC
*/

#include <cstdarg>
using std::va_list;


#include "logger/Logger.hh"
#include "logger/LoggerImpl.hh"
#include "logger/LogMessage.hh"
#include "concurrency/Thread.hh"


namespace service {

  namespace logger {

    bool Logger::needTimestamps() const {return tool::concurrency::Thread::currentThread().loggerImpl()->needTimestamps();}

    void Logger::send(const char *msg, size_t nbytes) {
      tool::concurrency::Thread::currentThread().loggerImpl()->send(msg, nbytes);
    }

    void Logger::logString(Severity sev, const char *msg) {
      LogMessage(sev).uadd(msg);
    }

    void Logger::log(Severity sev, const char *format, ...) {
      va_list ap;
      va_start(ap, format);
      LogMessage(sev).vadd(format, ap);
      va_end(ap);
    }

    void Logger::vlog(Severity sev, const char *format, va_list ap) {
      LogMessage(sev).vadd(format, ap);
    }

    void Logger::debug(const char *format, ...) {
      va_list ap;
      va_start(ap, format);
      LogMessage(Logger::Debug).vadd(format, ap);
      va_end(ap);
    }

    void Logger::info(const char *format, ...) {
      va_list ap;
      va_start(ap, format);
      LogMessage(Logger::Info).vadd(format, ap);
      va_end(ap);
    }

    void Logger::warning(const char *format, ...) {
      va_list ap;
      va_start(ap, format);
      LogMessage(Logger::Warning).vadd(format, ap);
      va_end(ap);
    }

    void Logger::error(const char *format, ...) {
      va_list ap;
      va_start(ap, format);
      LogMessage(Logger::Error).vadd(format, ap);
      va_end(ap);
    }

    void Logger::fatal(const char *format, ...) {
      va_list ap;
      va_start(ap, format);
      LogMessage(Logger::Fatal).vadd(format, ap);
      va_end(ap);
    }

  }; // logger

}; // service
