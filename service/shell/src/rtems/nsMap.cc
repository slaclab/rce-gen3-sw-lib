/* Main function for mapping a namespace
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

#define CMD_NAME   "ns_map"
#define CMD_TOPIC  "namespace"

namespace service {
  namespace shell {
    namespace nsMap {
      const char* usage = "ns_map namespace:image\n";

      int main(int argc, const char **argv) {

        if (argc <= 1) {
          dbg_printv("Error: image spec not provided\n");
          return -1;
        }

        const char* spec  = argv[1];

        // Colon is required in spec
        if (strchr(spec, ':') == 0) {
          dbg_printv("Error: image spec string '$s' does not contain a colon (':')\n",
                     spec);
          return -1;
        }

        char pth[LDR_MAX_PATH];
        Ldr_path rpth = Ldr_Map(spec, pth);
        if (!rpth) {
          dbg_printv("Error: Ldr_Map(%s) returned 0\n", spec);
          return -1;
        }

        printf("Path=%s\n", pth);
        
        return 0;
      }
    }
  }
}

SHELL_COMMAND(NsMap_Cmd, CMD_NAME, service::shell::nsMap::usage, CMD_TOPIC, service::shell::nsMap::main);
