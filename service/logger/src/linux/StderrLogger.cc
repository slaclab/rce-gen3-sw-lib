// -*-Mode: C++;-*-
/**
@file StderrLogger.cc
@brief Implements class StderrLogger on Linux
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

#include "logger/StderrLogger.hh"

namespace service {

  namespace logger {
    
    bool StderrLogger::needTimestamps() const {
      return true;
    }

  }
}
