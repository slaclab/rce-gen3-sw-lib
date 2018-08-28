/* Main function for stopping an RTEMS task
 *
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
#include "task/Task.h"

#include "ShellCommon.h"


#define CMD_NAME  "stop"
#define CMD_TOPIC "shell"

namespace service {
  namespace shell {
    namespace stopTask {

      const char* usage = CMD_NAME " [<id>/<name>]\n"
        "  Stop one RTEMS task\n" 
        "  If id/name is not specified, the last task started by run will be used\n"
        ;

      int main(int argc, const char **argv) {
        rtems_id id = 0;

        if (argc > 1) {
          id = strtol(argv[1], NULL, 0);
          if (!id)
            id = task_by_name(argv[1]);
        }
        if (!id)
          id = last_task_id();
        if (!id) {
          dbg_printv("Error: ID specified is null\n");
          return -1;
        }

        if (!task_exists(id)) {
          dbg_printv("Error: Task ID 0x%x does not exist.\n", id);
          return -2;
        }
        
        Task_Stop(id);

        return 0;
      }
    }
  }
}

SHELL_COMMAND(StopTask_Cmd, CMD_NAME, service::shell::stopTask::usage, CMD_TOPIC, service::shell::stopTask::main);
