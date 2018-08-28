// -*-Mode: C++;-*-
/**
@file
@brief Linux main program for logging tests.
@verbatim
                               Copyright 2012
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
DAT

@author
Steve Tether <tether@slac.stanford.edu>

@par Date created:
2012/12/04

@par Last commit:
\$Date: 2014-04-07 21:29:49 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3123 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/service/logger/test/linux/testMain.cc $

@par Credits:
SLAC
*/


#include "concurrency/Thread.hh"

namespace service {

  namespace logger {

    void testLogging();
  }

}

namespace {
  class Main: public tool::concurrency::Thread {
  public:
    Main(): Thread(false) {}
    virtual void body() {service::logger::testLogging();}
  };
}

int main() {
  // Logging should work both inside and outside of Threads.
  service::logger::testLogging();
  Main main;
  main.graduate();
  return 0;
}
