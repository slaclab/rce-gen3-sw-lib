// -*-Mode: C;-*-
/*!
*
* @brief   Shell command for dumping boot version info
*
* @author  S. Maldonado - (smaldona@slac.stanford.edu)
*
* @date    April 28, 2014 -- Created
*
* $Revision: $
*
* @verbatim
*                               Copyright 2014
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

#include "shell/Command.h"
#include "ShellCommon.h"
#include "bootdump.h"

#define CMD_NAME   "sysinfo"
#define CMD_TOPIC  "shell"

namespace service {
  namespace shell {
    namespace sysInfo {
      const char* usage = "Print system information.\n";
    
      int main(int argc, const char **argv) {
        return dumpBootVersion();
      }
    }
  }
}

SHELL_COMMAND(SysInfo_Cmd, CMD_NAME, service::shell::sysInfo::usage, CMD_TOPIC, service::shell::sysInfo::main);
