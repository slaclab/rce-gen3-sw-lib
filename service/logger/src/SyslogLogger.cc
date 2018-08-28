// -*-Mode: C++;-*-
/**
@file SyslogLogger.cc
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
Implements class SyslogLogger.

@author
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2011/01/12

@par Credits:
SLAC
*/

#include <stdio.h>



#include "logger/Logger.hh"
#include "logger/LoggerImpl.hh"
#include "logger/SyslogLogger.hh"
#include "io/rwall.hh"

namespace service {

  namespace logger {

    enum {SYSLOG_FD = 2};

    SyslogLogger::SyslogLogger() {}

    void SyslogLogger::send(const char *msg, size_t nbytes) {
      (void)tool::io::writeAll(SYSLOG_FD, msg, nbytes);
      (void)tool::io::writeAll(SYSLOG_FD, "\n", 1);
    }

    bool SyslogLogger::needTimestamps() const {return false;} // Stamps are provided by the syslog.

    SyslogLogger::~SyslogLogger() {}

  }; // logger

}; // service
