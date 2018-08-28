/* Main function assigning namespaces
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

#define CMD_NAME   "ns_assign"
#define CMD_TOPIC  "namespace"

namespace service {
  namespace shell {
    namespace nsAssign {
      const char* usage = "ns_assign namespace path\n";

      int main(int argc, const char **argv) {

        if (argc <= 2) {
          dbg_printv("Error: namespace not provided\n");
          return -1;
        }

        // pth is passed to Ldr_Assign which takes ownership
        int l = strlen(argv[2]);
        char* pth = new char[l+2]; // +2 -- we might need to add a trailing /
        memset(pth, 0, l+2);
        strncpy(pth, argv[2], l);
        if (pth[l-1] != '/') pth[l] = '/';
        const char* ns  = argv[1];

        // No colons are allowed in ns
        if (strchr(ns, ':') != 0) {
          dbg_printv("Error: namespace string '$s' contains a colon (':')\n",
                     ns);
          return -1;
        }

        Ldr_path lPth = Ldr_Assign(ns, pth);
        if (!lPth) {
          dbg_printv("Error: Ldr_Assign(%s,%s) returned 0\n",
                     ns, pth);
        }
        
        return 0;
      }
    }
  }
}

SHELL_COMMAND(NsAssign_Cmd, CMD_NAME, service::shell::nsAssign::usage, CMD_TOPIC, service::shell::nsAssign::main);
