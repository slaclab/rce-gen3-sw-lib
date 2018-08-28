// -*-Mode: C;-*-
/**
@file
@brief Test the protection against stack overflow.

@verbatim
                               Copyright 2014
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
2014/04/25

@par Last commit:
\$Date: 2014-06-16 17:27:17 -0700 (Mon, 16 Jun 2014) $ by \$Author: tether $.

@par Revision number:
\$Revision: 3396 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/memory/test/armCA9-rtems/oroborous.c $

@par Credits:
SLAC

*/



#include "task/Task.h"         

void oroborous(void) {
  oroborous();
}

void Task_Start(int argc, const char** argv) {
  oroborous();
}

void Task_Rundown(void) {
}
