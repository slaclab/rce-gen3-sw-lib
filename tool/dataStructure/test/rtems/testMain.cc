// -*-Mode: C++;-*-
/**
@file
@brief An rce_appmain() function to run the package's unit tests as a module under RTEMS.
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
\$Date: 2012-02-15 15:02:47 -0800 (Wed, 15 Feb 2012) $ by \$Author: tether $.

@par Revision number:
\$Revision: 662 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/dataStructure/test/rtems/testMain.cc $

@par Credits:
SLAC
*/

void testDataStructure();

extern "C" void rce_appmain() {
  testDataStructure();
}

