// -*-Mode: C++;-*-
/**
@file
@brief Run the unit tests for tool::dataStructure.
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
\$Date: 2012-02-15 15:02:47 -0800 (Wed, 15 Feb 2012) $ by \$Author: tether $.

@par Revision number:
\$Revision: 662 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/dataStructure/test/testDataStructure.cc $

@par Credits:
SLAC
*/
void testBitSet();
void testBitRota();

void testDataStructure() {
  testBitSet();
  testBitRota();
}
