// -*-Mode: C++;-*-
/**
@file LogMessage.cc
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
Implements the LogMessage class.

@author
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2010/03/25

@par Credits:
SLAC
*/

#include <stdio.h>

#include <cstdarg>
using std::va_list;

#include <time.h>


#include "logger/Logger.hh"
#include "logger/LogMessage.hh"
#include "string/StringBuffer.hh"
#include "concurrency/Thread.hh"

namespace {
  using service::logger::Logger;
  using tool::string::StringBuffer;
  using tool::concurrency::Thread;

  void addProlog(StringBuffer &msg, Logger::Severity sev) {
    fflush(stderr);
    if (Logger().needTimestamps()) {
      timespec now;
      clock_gettime(CLOCK_REALTIME, &now);
      struct tm result;
      localtime_r(&now.tv_sec, &result);
      char buf[100];
      strftime(buf, sizeof buf, "%Y/%m/%d %H:%M:%S", &result);
      msg.uadd(buf);
      msg.add(".%06ld: ", (now.tv_nsec + 500L) / 1000L);
    }
    // Print the severity level.
    msg.add("%-8s", tool::type::estr(sev));
  }
}


namespace service {

  namespace logger {

    size_t LogMessage::m_initialBufferSize = 200;

    LogMessage::LogMessage(Logger::Severity s)
      : m_willSend(s >= tool::concurrency::Thread::currentThread().logThreshold()),
	m_buffer(m_willSend ? initialBufferSize() : 0)
    {
      if (m_willSend) addProlog(m_buffer, s);
    }

    LogMessage::~LogMessage() {
      if (m_willSend) Logger().send(m_buffer.str(), m_buffer.strlen());
    }

    LogMessage &LogMessage::vadd(const char *format, va_list ap) {
      if (m_willSend) {
	m_buffer.vadd(format, ap);
      }
      return *this;
    }

    LogMessage &LogMessage::add(const char *format, ...) {
      // Make the argument list pointer point to the first optional
      // argument.
      va_list ap;
      va_start(ap, format);

      // I don't know if it's required but I think it best to use
      // va_end() in the same stack frame as the corresponding
      // va_start(); I'm told that some implementations of va_start()
      // make changes to the stack frame that must be undone by
      // va_end() before the function can return.
      m_buffer.vadd(format, ap);
      va_end(ap);
      return *this;
    }

  }; // logger
}; // service
