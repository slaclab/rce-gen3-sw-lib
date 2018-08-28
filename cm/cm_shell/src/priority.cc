/* Main function for setting new priority
 *
 *  Author:  <Jack Pines: jackp@slac.stanford.edu>
 *  Created: <2009-11-20>
 *  Time-stamp: <Tue Aug 25 16:00:50 PDT 2009 jackp>
 *
 *
 *  based on work by Jim Panneta
 *
 */

#include <rtems/shell.h>
#define __need_getopt_newlib
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>
using std::string;

#include <list>
using std::list;


#include "debug/Debug.hh"
#include "debug/Message.hh"
#include "container/List.hh"
#include "time/Time.hh"

#include "shell/ShellCommands.hh"

namespace cm {
namespace shell {
static string priorityUsageLong =  "Usage:\n" \
  "  priorty -p <priority> \n"                \
  "  Change priority"                         \
  "  Arguments: \n"                           \
  "    -p <#> priorty to use\n"               \
  "    -h print this message\n";

static string priorityUsage = "priority [-p <#>][-h]";

rtems_task_priority pri = 115;

int priortyParseOptions(int argc, char **argv) {
  int theOpt;
  /* Use reentrant form of getopt as per RTEMS Shell Users Guide (4.9.99.0) */
  struct getopt_data getopt_reent;
  memset(&getopt_reent, 0, sizeof(getopt_data));
  while ( (theOpt = getopt_r(argc, argv, "p:h", &getopt_reent)) != EOF ) {
    switch(theOpt) {
      case 'p':
        //sscanf(getopt_reent.optarg, "%u", &pri);
        pri = strtoul(getopt_reent.optarg, 0, 0);
        break;
      case 'h':
        printf(priorityUsageLong.c_str());
        return 1;
      default:
        printf("Option: -%c is not parsable.\n", theOpt);
        printf(priorityUsageLong.c_str());
        return 2;
    }
  }
  return 0;
}

int priority_main(int argc, char **argv) {

  int parseRet = priortyParseOptions(argc, argv);
  if (parseRet) return parseRet;

  rtems_status_code status;
  rtems_task_priority old;
  status = rtems_task_set_priority(RTEMS_SELF, pri, &old);
  if (status == RTEMS_SUCCESSFUL) {
    if (old != pri) {
      fprintf(stderr, "priorty changed from %d to %d\n", static_cast<int>(old), static_cast<int>(pri));
    }
  } else fprintf(stderr, "priorty was unable to change pty priority\n");


  return 0;
}


std::string Priority::usage() {
  return priorityUsage;
}

service::shell::Command::MainFunc Priority::main() {
  return priority_main;
}

}
}
