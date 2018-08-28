/* Defines the prototypes for all core shell commands on the RCE
 *
 *  Author:  <Jim Panetta: panetta@slac.stanford.edu>
 *  Created: <2009-03-24>
 *  Time-stamp: <2011-01-12 18:22:36 panetta>
 *
 *  Command prototypes:

 *
*/

#ifndef SERVICE_SHELL_BASECOMMANDS_HH
#define SERVICE_SHELL_BASECOMMANDS_HH


#include "shell/Command.h"

namespace service {
  namespace shell {
    extern Shell_Command Reboot_Cmd;
    extern Shell_Command RunTask_Cmd;
    extern Shell_Command StopTask_Cmd;
    extern Shell_Command ResumeTask_Cmd;
    extern Shell_Command SuspendTask_Cmd;
    extern Shell_Command Syslog_Cmd;
    extern Shell_Command ResUse_Cmd;

    extern Shell_Command Lookup_Cmd;

  }
}

#include "shell/gen/BaseCommands.hh"

#endif
