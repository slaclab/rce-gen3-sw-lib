/* Main function for removing a namespace
 *
 *  Author:  <Jim Panetta: panetta@slac.stanford.edu>
 *  Created: <2009-03-24>
 *  Time-stamp: <2009-03-24 14:42:18 panetta>
 *
 *
 *
 *
 */



#include "shell/Command.h"
#include "debug/print.h"
#include "elf/linker.h"
#include "ldr/Ldr.h"

#include "ShellCommon.h"

#define CMD_NAME   "ns_remove"
#define CMD_TOPIC  "namespace"

namespace service {
  namespace shell {
    namespace nsRemove {
      const char* usage = "ns_remove namespace\n";

      int main(int argc, const char **argv) {

        if (argc <= 1) {
          dbg_printv("Error: image spec not provided\n");
          return -1;
        }

        const char* ns  = argv[1];

        // Colon is prohibited
        if (strchr(ns, ':') != 0) {
          dbg_printv("Error: namespace string '$s' contains a colon (':')\n",
                     ns);
          return -1;
        }

        Ldr_path rpth = Ldr_Remove(ns);
        if (!rpth) {
          dbg_printv("Error: Ldr_Remove(%s) returned 0\n", ns);
          return -1;
        }

        return 0;
      }
    }
  }
}

SHELL_COMMAND(NsRemove_Cmd, CMD_NAME, service::shell::nsRemove::usage, CMD_TOPIC, service::shell::nsRemove::main);
