// -*-Mode: C++;-*-
/**
@file
@brief Test the syslog-related classes in tool::debug.
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
Stephen Tether <@slac.stanford.edu>

@par Date created:
2013/08/08

@par Last commit:
\$Date: 2014-04-07 21:30:01 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3125 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/debug/test/armCA9-rtems/testSyslog_module.cc $

@par Credits:
SLAC
*/

#include <stdio.h>

#include <memory>



#include "debug/os/SyslogChunker.hh"
#include "debug/os/SyslogTracker.hh"
using tool::debug::SyslogChunker;
using tool::debug::SyslogTracker;

char buffer[100];
int  inext = 0;

void pc(char ch) {
  buffer[inext++] = ch;
  if (inext == sizeof buffer) inext = 0;
  buffer[inext] = '\0';
}

int pchunks(SyslogChunker &sc) {
  int n(0);
  for (; sc.chunkAddress(); sc.advance()) {
    printf("  Chunk %d: '", n++);
    for (int i =0; i < sc.chunkSize(); ++i) putchar(*(sc.chunkAddress()+i));
    printf("'\n");
  }
  return n;
}

extern "C" void rce_appmain(void*) {
  std::auto_ptr<SyslogTracker> trk(new SyslogTracker(buffer, sizeof buffer, 0, 0, pc));
  printf("\nChunker on empty buffer.\n");
  int n;
  { SyslogChunker sc(*trk);
    n = pchunks(sc);
    printf("  No. of chunks = %d. %s\n\n", n, (n==0) ? "OK" : "FAIL");
  }

  printf("Chunker on a non-full, non-empty buffer.\n");
  for (const char* ch = "Hello"; *ch; ++ch) trk->sendChar(*ch);
  { SyslogChunker sc(*trk);
    n = pchunks(sc);
    printf("  No. of chunks = %d. %s\n\n", n, (n==1) ? "OK" : "FAIL");
  }

  printf("Chunker on a full (wrapped-around) buffer.\n");
  for (size_t i = 0; i < sizeof(buffer)-20U; ++i) trk->sendChar('*');
  { SyslogChunker sc(*trk);
    n = pchunks(sc);
    printf("  No. of chunks = %d. %s\n", n, (n==2) ? "OK" : "FAIL");
  }
  { SyslogChunker sc(*trk);
    int sum(0);
    for (; sc.chunkAddress(); sc.advance()) sum += sc.chunkSize();
    printf("  Sum of chunk sizes = %d. %s\n\n", sum, ((sum+1)==sizeof buffer) ? "OK" : "FAIL");
  }
  
}
