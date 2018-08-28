// -*-Mode: C;-*-
/**
@file
@brief Source code for a shared object to be used by the testElf program.
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
2013/08/26

@par Last commit:
\$Date: 2014-04-07 21:30:01 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3125 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/elf/test/linux/testSubject.c $

@par Credits:
SLAC
*/


EXPORT char spam[10];

extern void sausage(void);

void egg(void) EXPORT;
void egg(void) {sausage();}

double tomato;

/* Const string, should show up in the text or RO data segment. */
EXPORT const char string1[] = {"This is const string #1."};

/* Mutable string, should show up in the RW data segment. */
EXPORT char string2[] = {"This is mutable string #2."};
