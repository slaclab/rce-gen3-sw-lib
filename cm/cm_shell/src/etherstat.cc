/* Main function for dumping the system log
 *
 *  Author:  <Jack Pines: jackp@slac.stanford.edu>
 *  Created: <2009-08-25>
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


//#include "shell/image_tools.hh"    // prototype for open_filespec
#include "debug/Debug.hh"
#include "debug/Message.hh"
#include "container/List.hh"
#include "time/Time.hh"
#include "debug/Print.hh"
#include "debug/State.hh"
#include "debug/Context.hh"
#include "debug/Message.hh"
#include "debug/Missive.hh"
#include <rtems/rtems_bsdnet.h>

#include "shell/ShellCommands.hh"

namespace cm {
namespace shell {
static string etherStatUsageLong =  "Usage:\n" \
  "  etherstat \n"                             \
  "   -h  print this message\n"                \
  "  if no options, show the ethernet status.\n";

static string etherStatUsage = "etherstat [-h]";

bool show;

int etherStatParseOptions(int argc, char **argv) {
  int theOpt;
  /* Use reentrant form of getopt as per RTEMS Shell Users Guide (4.9.99.0) */
  struct getopt_data getopt_reent;
  memset(&getopt_reent, 0, sizeof(getopt_data));
  while ( (theOpt = getopt_r(argc, argv, ":h", &getopt_reent)) != EOF ) {
    show = false;
    switch(theOpt) {
      case 'h':
        printf(etherStatUsageLong.c_str());
        return 1;
      default:
        printf("Option: %c is not parsable.\n", theOpt);
        printf(etherStatUsageLong.c_str());
        return 2;
    }
  }
  return 0;
}

int etherStat_main(int argc, char **argv) {
  show  = true;

  int parseRet = etherStatParseOptions(argc, argv);
  if (parseRet) return parseRet;

  if (show) {
    rtems_bsdnet_show_inet_routes();
    rtems_bsdnet_show_mbuf_stats();
    rtems_bsdnet_show_if_stats();
    rtems_bsdnet_show_ip_stats();
    rtems_bsdnet_show_icmp_stats();
    rtems_bsdnet_show_udp_stats();
    rtems_bsdnet_show_tcp_stats();
  }
  return 0;
}


std::string EtherStat::usage() {
  return etherStatUsage;
}

service::shell::Command::MainFunc EtherStat::main() {
  return etherStat_main;
}

}
}
