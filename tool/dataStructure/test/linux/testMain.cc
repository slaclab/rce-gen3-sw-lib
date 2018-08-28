// -*-Mode: C++;-*-
/**
@file
@brief A Linux main program to run the package's unit tests.
@verbatim
                               Copyright 2011
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
2011/07/21

@par Last commit:
\$Date: 2014-04-07 21:30:01 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3125 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/dataStructure/test/linux/testMain.cc $

@par Credits:
SLAC
*/
#include <unistd.h>


#include "concurrency/Thread.hh"
using tool::concurrency::Thread;

void testDataStructure();

// We need a Thread to provide the proper environment for Logger() and
// other services.
class TestThread: public Thread {
public:
  TestThread();
  virtual void body();
};

TestThread::TestThread(): Thread(false)  // Don't try to inherit from the parent thread - there is none!
{}

void TestThread::body() {testDataStructure();}

int main() {
  TestThread thr;
  thr.start();
  while (!thr.isFinished()) sleep(1);
}

