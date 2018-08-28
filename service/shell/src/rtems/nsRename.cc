/* Main function for renaming a namespace
 *
 *  Author:  <Jim Panetta: panetta@slac.stanford.edu>
 *  Created: <2009-03-24>
 *  Time-stamp: <2009-03-24 14:42:18 panetta>
 *
 *
 *
 *
 */

//#define __need_getopt_newlib
//#include <getopt.h>
//#include <stdlib.h>



#include "shell/Command.h"
#include "debug/print.h"
#include "elf/linker.h"
#include "ldr/Ldr.h"

#include "ShellCommon.h"

#define CMD_NAME   "ns_rename"
#define CMD_TOPIC  "namespace"

namespace service {
  namespace shell {
    namespace nsRename {
      const char* usage = "ns_rename namespace path\n";

      int main(int argc, const char **argv) {

        if (argc <= 2) {
          dbg_printv("Error: namespace not provided\n");
          return -1;
        }

        // pth is passed to Ldr_Rename which takes ownership
        char* pth = new char[strlen(argv[2])+1];
        memset(pth, 0, strlen(argv[2])+1);
        strncpy(pth, argv[2], strlen(argv[2]));
        const char* ns  = argv[1];

        // No colons are allowed in ns
        if (strchr(ns, ':') != 0) {
          dbg_printv("Error: namespace string '$s' contains a colon (':')\n",
                     ns);
          return -1;
        }

        Ldr_path lPth = Ldr_Rename(ns, pth);
        if (!lPth) {
          dbg_printv("Error: Ldr_Rename(%s,%s) returned 0\n",
                     ns, pth);
        }
        
        return 0;
      }
    }
  }
}

SHELL_COMMAND(NsRename_Cmd, CMD_NAME, service::shell::nsRename::usage, CMD_TOPIC, service::shell::nsRename::main);
