// -*-Mode: C++;-*-
/**
@file StderrLogger.cc
@brief Implements class StderrLogger.
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

@par Credits:
SLAC
*/

#include <stdio.h>



#include "logger/Logger.hh"
#include "logger/LoggerImpl.hh"
#include "logger/StderrLogger.hh"
#include "io/rwall.hh"

namespace service {

  namespace logger {

    StderrLogger::StderrLogger() {}

    void StderrLogger::send(const char *msg, size_t nbytes) {
      (void)tool::io::writeAll(fileno(stderr), msg, nbytes);
      (void)tool::io::writeAll(fileno(stderr), "\n", 1);
    }

    StderrLogger::~StderrLogger() {}

  }; // logger

}; // service
