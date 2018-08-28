// -*-Mode: C++;-*-
/**
@file reboot.cc
@verbatim
                               Copyright 2010
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
Service %Shell

@par Abstract:
Command to reboot the RCE

@author
Jim Panetta <panetta@slac.stanford.edu>

@par Date created:
2010/10/04

@par Last commit:
\$Date: 2014-04-07 21:29:49 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3123 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/service/shell/src/rtems/lookup.cc $

@par Credits:
SLAC
*/



#include "shell/Command.h"
#include "ldr/Ldr.h"
#include "debug/print.h"


#define CMD_NAME  "lookup"
#define CMD_TOPIC "misc"

namespace service {
  namespace shell {
    namespace lookup {
     
      const char* usage =
        "Usage:  " CMD_NAME " <soname> \n"
        "  \n";
    
      int main(int argc, const char** argv) {
        if (argc < 1)
          dbg_prints(usage);
        const char* soname = argv[1];            
        
        Ldr_elf* elf = Ldr_Lookup(soname);
        dbg_printv("Lookup of %s: %p\n",soname, elf);
      }
    }
  }
}

SHELL_COMMAND(Lookup_Cmd, CMD_NAME, service::shell::lookup::usage, CMD_TOPIC, service::shell::lookup::main);

