// -*-Mode: C++;-*-
/**
@file testLogging.cc
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
Unit tests for the logging service.

@author
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2010/02/11

@par Revision date:
\$Date: 2014-04-07 21:29:49 -0700 (Mon, 07 Apr 2014) $

@par Revision number:
\$Revision: 3123 $

@par Checkout/export tag:
\$Name$

@par Credits:
SLAC
*/


#include "logger/Logger.hh"
using service::logger::Logger;

#include "logger/LogMessage.hh"
using service::logger::LogMessage;

namespace service {

  namespace logger {

    void testLogging() {
      LogMessage::initialBufferSize(10); // Make sure we have to retry vsnprintf() every message.
      // Debug < Info so the next message should not be logged.
      Logger().debug("Test debug logging %d %f", 5, 5.0);
      Logger().info("Test info logging %d %f", 1, 1.0);
      Logger().warning("Test warning logging %d %f", 2, 2.0);
      Logger().error("Test error logging %d %f", 3, 3.0);
      Logger().fatal("Test fatal logging %d %f", 4, 4.0);
      LogMessage(Logger::Info).add("This message ").add("was %s ", "assembled").add("piecemeal.");
      { LogMessage msg(Logger::Warning);
        msg.add("And so ");
        msg.add("was %s ", "this");
        msg.add("one.");
      }
    }

  }
}

