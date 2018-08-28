/* Main function for burning a Task into the RCE
 *
 *  Author:  <Jim Panetta: panetta@slac.stanford.edu>
 *  Created: <2009-03-24>
 *  Time-stamp: <2009-03-24 14:42:18 panetta>
 *
 *
 *
 *
 */

#define __need_getopt_newlib
#include <getopt.h>
#include <stdlib.h>



#include "shell/Command.h"
#include "debug/print.h"
#include "elf/linker.h"
#include "elf/lnkStatus.h"
#include "ldr/Ldr.h"
#include "task/Task.h"

#include "ShellCommon.h"

#define CMD_NAME  "run"
#define CMD_TOPIC "shell"

namespace service {
  namespace shell {
    namespace runTask {

      const char* usage =
        "Usage:  " CMD_NAME " [[-N taskName] [-P priority] [-S stacksize]] "
        "<namespace>:<exe> [-- task arguments] \n"
        "  Execute the code in file pointed to by namespace:exe\n"
        "  with RTEMS task name as <taskName> (4 characters)\n"
        "  Optional arguments: \n"
        "  -N taskName:     RTEMS task name        \n" 
        "  -P <priority>:   RTEMS priority.        \n"
        "  -S <stacksize>:  RTEMS stack size.      \n"
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

      int runTaskParseOptions(int argc, const char **argv, CommandLine &cmd) {
        int theOpt;
        /* Use reentrant form of getopt as per RTEMS Shell Users Guide (4.9.99.0) */
        struct getopt_data getopt_reent;
        memset(&getopt_reent, 0, sizeof(getopt_data));
        while ( (theOpt = getopt_r(argc,
                                   const_cast<char* const*>(argv),
                                   "P:S:N:A:", &getopt_reent)) != EOF ) {
          switch(theOpt) {
          case 'P':
            cmd.priority   = strtol(getopt_reent.optarg, NULL, 0);
            cmd.specified |= CommandLine::PRIORITY_OPTION;
            break;
          case 'S':
            cmd.stacksize =  strtol(getopt_reent.optarg, NULL, 0);
            cmd.specified |= CommandLine::STACKSIZE_OPTION;
            break;
          case 'N':
            cmd.setName(getopt_reent.optarg);
            cmd.specified |= CommandLine::TASKNAME_OPTION;
            break;
          default:
            dbg_printv("Option: -%c is not parsable.\n", theOpt);
            dbg_printv(usage);
            return 2;
          }
        }
        // Done parsing args, optind should be the namespace
        int nsPosition = getopt_reent.optind;
        if (nsPosition >= argc) {
          dbg_printv("runTask ERROR:  Image not specified.\n");
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
        int const parseRet = runTaskParseOptions(argc, argv, cmd);
        if (parseRet) return parseRet;

        // persist any changes to the default task attributes
        Task_Attributes* dftAttr = shell_default_attrs();
        if (cmd.specified & CommandLine::TASKNAME_OPTION)
          shell_default_attrs()->name = cmd.name;
        if (cmd.specified & CommandLine::PRIORITY_OPTION)
          shell_default_attrs()->priority = cmd.priority;
        if (cmd.specified & CommandLine::STACKSIZE_OPTION)
          shell_default_attrs()->stack_size = cmd.stacksize;
        if (cmd.specified & CommandLine::ATTRIBUTE_OPTION)
          shell_default_attrs()->attributes = cmd.attributes;
        

        // Make a copy and add argc/v
        Task_Attributes myAttr;
        memcpy(&myAttr, dftAttr, sizeof(Task_Attributes));
        myAttr.argc = cmd.userArgc;
        myAttr.argv = cmd.userArgv;

        // Load and link
        lnk_Status s;
        uint32_t error;
        Ldr_elf *elf = lnk_load(cmd.ns, &myAttr, &error, &s);
        if (!elf) {
          dbg_printv("lnk_load failed with error 0x%x\n", error);
          return 2;
        }

        rtems_id id;
        Task_status result = Task_Run(elf, &myAttr, myAttr.argc, myAttr.argv, &id);
        if (result) {
          dbg_printv("Task_Run for %s failed with RC %d.\n",
                     myAttr.argv[0], result);
        } else {
          set_last_task_id(id);
        }        

        return 0;
      }
    }
  }
}
    
SHELL_COMMAND(RunTask_Cmd, CMD_NAME, service::shell::runTask::usage, CMD_TOPIC, service::shell::runTask::main);
