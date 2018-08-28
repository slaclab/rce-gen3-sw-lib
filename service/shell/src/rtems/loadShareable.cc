// -*-Mode: C;-*-
/*!
*
* @brief   Shell command for loading an RTEMS shareable image
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

#define __need_getopt_newlib
#include <getopt.h>
#include <stdlib.h>

#include "shell/Command.h"
#include "debug/print.h"
#include "elf/linker.h"
#include "elf/lnkStatus.h"
#include "ldr/Ldr.h"

#include "ShellCommon.h"

#define CMD_NAME  "load"
#define CMD_TOPIC "shell"

namespace service {
  namespace shell {
    namespace loadShareable {

      const char* usage =
        "Usage:  " CMD_NAME " "
        "<namespace>:<image>\n"
        "  Load and link the code in file pointed to by namespace:image.\n"
        "\n";


      /** @internal @brief What was parsed from the command line. */
      struct CommandLine {

        /** @internal @brief Enumerates what can be specified on the command line. */
        enum Items {
          PRIORITY_OPTION  = (1<<1),
          STACKSIZE_OPTION = (1<<2),
          TASKNAME_OPTION  = (1<<3),
          ATTRIBUTE_OPTION = (1<<4),
          NS_ARGUMENT      = (1<<5)
        };

        CommandLine()
          : specified(0),
            ns(0),
            userArgc(0),
            userArgv(0)
          {
            setName(shell_default_attrs()->name);
            priority   = shell_default_attrs()->priority;
            stacksize  = shell_default_attrs()->stack_size;
            attributes = shell_default_attrs()->attributes;
          }

        void setName(const char* nm) {
          name = name_to_name(nm);
        }

        void setName(const rtems_name& nm) {
          name = nm;
        }
        
        unsigned     specified;  ///< @internal @brief Bitmask of Items.
        unsigned     priority;   ///< @internal @brief The priority from -P.
        unsigned     stacksize;  ///< @internal @brief The stack size from -S.
        unsigned     attributes; ///< @internal @brief The attributes from -A.
        rtems_name   name;       ///< @internal @brief The task name from -N.
        const char*  ns;         ///< @internal @brief The namespace name argument.
        int          userArgc;
        const char** userArgv;
      };

      int loadShareableParseOptions(int argc, const char **argv, CommandLine &cmd) {
        int theOpt;
        /* Use reentrant form of getopt as per RTEMS Shell Users Guide (4.9.99.0) */
        struct getopt_data getopt_reent;
        memset(&getopt_reent, 0, sizeof(getopt_data));
        while ( (theOpt = getopt_r(argc,
                                   const_cast<char* const*>(argv),
                                   "", &getopt_reent)) != EOF ) {
        }
        
        // Done parsing args, optind should be the namespace
        int nsPosition = getopt_reent.optind;
        if (nsPosition >= argc) {
          dbg_printv("loadShareable ERROR:  Image not specified.\n");
          return 2;
        }
        cmd.ns      =  argv[nsPosition];
        // Make sure that the namespace exists
        char exists[LDR_MAX_PATH];
        Ldr_path pth = Ldr_Map(cmd.ns, exists);
        if (pth)
          cmd.specified |= CommandLine::NS_ARGUMENT;
        if ((cmd.specified & CommandLine::NS_ARGUMENT) == 0) {
          dbg_printv("You have to specify a valid file spec (namespace:executable).\n");
          dbg_printv(usage);
          return 2;
        }
        
        cmd.userArgc = argc - (nsPosition);
        cmd.userArgv = new const char*[cmd.userArgc];
        cmd.userArgv[0] = strdup(cmd.ns);
        for (int i=1; i<cmd.userArgc; ++i)
          cmd.userArgv[i] = strdup(argv[nsPosition+i]);
        
        return 0;
      }

      int main(int argc, const char **argv) {
        CommandLine cmd;

        // Scan the command line.
        int const parseRet = loadShareableParseOptions(argc, argv, cmd);
        if (parseRet) return parseRet;

        // Load and link
        lnk_Status s;
        uint32_t error;
        Ldr_elf *elf = lnk_load(cmd.ns, NULL, &error, &s);
        if (!elf) {
          dbg_printv("lnk_load failed with error 0x%x\n", error);
          return 2;
        }
        return 0;
      }
    }
  }
}
    
SHELL_COMMAND(LoadShareable_Cmd, CMD_NAME, service::shell::loadShareable::usage, CMD_TOPIC, service::shell::loadShareable::main);
