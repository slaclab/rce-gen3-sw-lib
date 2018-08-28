// -*-Mode: C++;-*-
/**
@file test_newCommands.cc
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
Program to test adding new commands to the RCE

@author
Jim Panetta <panetta@slac.stanford.edu>

@par Date created:
2010/05/14

@par Last revision:
\$Date: 2014-04-07 21:29:49 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $

@par Revision number:
\$Revision: 3123 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/service/shell/test/test_addCommand.cc $

@par Credits:
SLAC
*/


#include "shell/Command.hh"

#include <stdio.h>
#include <stdlib.h>

namespace NewCMD {
  using service::shell::Command;
  class NewCommand : public Command {
  public:
    NewCommand();
    virtual ~NewCommand() {}
    virtual std::string usage();
    virtual MainFunc main();
  };
}

namespace NewCMD {
  using service::shell::Command;

  int NewCommand_main(int argc, char** argv) {
    printf("This is a test of the new command\n");
    return 0;
  }

  NewCommand::NewCommand() : Command("newCommand", "misc") {}

  std::string NewCommand::usage() {
    static std::string u = "  newCommand     # Do nothing but print a message \n";
    return u;
  }

  Command::MainFunc NewCommand::main() {
    return NewCommand_main;
  }

}

extern "C" {
  void EXPORT addCommands(int unused) {
    // Example of dynamic adding a new command.
    // the user must first instantiate, then initialize
    NewCMD::NewCommand cmd; cmd.initialize();
  }
}
