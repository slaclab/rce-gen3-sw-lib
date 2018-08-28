// -*-Mode: C++;-*-
/**
@file
@brief Unit tests for classes BitSet and BitPq.
@verbatim
                               Copyright 2010
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
Tool

@author
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2010/03/24

@par Last commit:
\$Date: 2014-04-07 21:30:01 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3125 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/dataStructure/test/testBitSet.cc $

@par Credits:
SLAC
*/


#include "dataStructure/BitSet.hh"
using tool::dataStructure::BitPq;
using tool::dataStructure::BitSet;

#include "logger/Logger.hh"
using service::logger::Logger;

void testBitSet() {
  unsigned contents = 0xdeadbeef;
  bool errors = false;
  Logger().info("");
  Logger().info("");
  Logger().info("===== Test of tool::dataStructure::BitSet and BitPq =====");
  Logger().info("Initial bitset contents = 0x%08x.", contents);
  for (BitPq q = BitPq(BitSet(contents)); !q.empty(); q.pop()) {
    const unsigned qnext = 1U << q.top();
    const unsigned bnext = contents & (-contents);
    contents ^= bnext;
    if (qnext != bnext) {
      Logger().error("Next from BitPq is 0x%08x, should be 0x%08x.",
		     qnext, bnext);
      errors = true;;
    }
  }
  if (contents) {
    Logger().error("BitPq lost these bits: 0x%08x.", contents);
    errors = true;;
  }
  if (errors) {
    Logger().error("FAILURE.");
  }
  else {
    Logger().info("SUCCESS.");
  }
}
