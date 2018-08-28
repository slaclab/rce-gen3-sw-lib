// -*-Mode: C;-*-
/**
@file
@brief Run test code for the "mem" subpackage under Linux.
@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
DAT

@author
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2013/06/20

@par Last commit:
\$Date: 2013-12-19 14:48:10 -0800 (Thu, 19 Dec 2013) $ by \$Author: tether $.

@par Revision number:
\$Revision: 2435 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/memory/test/armCA9-linux/testMem_main.c $

@par Credits:
SLAC
*/

//void mem_testProbe(void);
//void mem_testFlush(void);
void mem_testFlags(void);

int main(void) {
  //mem_testProbe();
  //mem_testFlush();
  mem_testFlags();
  return 0;
}
