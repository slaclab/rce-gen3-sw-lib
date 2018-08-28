// -*-Mode: C;-*-
/**
@file
@brief Run test code for too/memory/mem under RTEMS.
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
\$Date: 2013-10-31 15:26:32 -0700 (Thu, 31 Oct 2013) $ by \$Author: tether $.

@par Revision number:
\$Revision: 2242 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/memory/test/armCA9-rtems/testMem_module.c $

@par Credits:
SLAC
*/

/* void mem_testProbe(void); */
/* void mem_testFlush(void); */
void mem_testFlags(void);

void rce_appmain(void) {
  /* mem_testProbe(); */
  /* mem_testFlush(); */
  mem_testFlags();
}
