// -*-Mode: C++;-*-
/**
@file
@brief Unit test for class BitRota.
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
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/dataStructure/test/testBitRota.cc $

@par Credits:
SLAC
*/


#include "logger/Logger.hh"
#include "logger/LogMessage.hh"
using service::logger::Logger;
using service::logger::LogMessage;

#include "dataStructure/BitRota.hh"
using tool::dataStructure::BitRota;

void testBitRota() {
  BitRota rota;
  Logger().info("");
  Logger().info("===== Test of tool::dataStructure::BitRota =====");
  Logger().info("removeNext() returns %d for an empty rota.", rota.removeNext());
  unsigned zero (0x80000000);
  unsigned one  (0x40000000);
  unsigned two  (0x20000000);
  unsigned three(0x10000000);
  unsigned four (0x08000000);

  Logger().info("Place masks (hex): %08x %08x %08x %08x %08x", zero, one, two, three, four);
  { LogMessage msg(Logger::Info);
    rota.add(zero | one | two | three | four);
    msg.uadd("Rota of five with no replacement:");
    for (int i = 0; i < 20; ++i) msg.add(" %d", rota.removeNext());
  }
  { LogMessage msg(Logger::Info);
    msg.uadd("Rota of five replacing everybody all the time:");
    for (int i = 0; i < 20; ++i) {
      rota.add(zero | one | two | three | four);
      msg.add(" %d", rota.removeNext());
    }
  }
  { LogMessage msg(Logger::Info);
    msg.uadd("Rota of five replacing only 1 and 4:");
    rota.add(zero | one | two | three | four);
    for (int i = 0; i < 20; ++i) {
      rota.add(one | four);
      msg.add(" %d", rota.removeNext());
    }
  }
}
