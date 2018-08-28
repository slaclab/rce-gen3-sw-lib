/* Main function for dumping the system log
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
#include <stdio.h>
#include <string.h>

#include <memory>
using std::auto_ptr;



#include "shell/Command.h"

#include "io/rwall.hh"
using tool::io::writeAll;

#include "debug/os/syslog.h"

#include "ShellCommon.h"

#define CMD_NAME "syslog"
#define CMD_TOPIC "shell"

namespace service {
  namespace shell {
    namespace syslog {

      const char* usage =  CMD_NAME " [-c] \n"
    "  Show the system log.\n"
    "  Arguments: \n"
    "  -c Clear the system log.\n";

      int syslogParseOptions(int argc, const char **argv,
                             int &first, int &last, int &clear) {
        int theOpt;
        /* Use reentrant form of getopt as per RTEMS Shell Users Guide (4.9.99.0) */
        struct getopt_data getopt_reent;
        memset(&getopt_reent, 0, sizeof(getopt_data));
        while ( (theOpt = getopt_r(argc,
                                   const_cast<char* const*>(argv),
                                   "c", &getopt_reent)) != EOF ) {
          switch(theOpt) {
          case 'c':
            clear = 1;
            break;
          default:
            printf("Option: -%c is not parsable.\n", theOpt);
            printf(usage);
            return 2;
          }
        }
        return 0;
      }

      int main(int argc, const char **argv) {
        // Note:: One should print the syslog only if stdout is *not*
        // the syslog, e.g., is a telnet socket.
        int first = -1;
        int last = -1;
        int clear = -1;

        int parseRet = syslogParseOptions(argc, argv, first, last, clear);
        if (parseRet) return parseRet;

        if (clear < 0) {
          dbg_SyslogChunker* const chunker(dbg_SyslogChunker_construct());
          if (!chunker) return 3;
          // We'll use unbuffered I/O since terminating NULs are not guaranteed, so flush buffers first.
          fflush(stdout);
          for (; dbg_SyslogChunker_chunkAddress(chunker); dbg_SyslogChunker_advance(chunker)) {
            writeAll(fileno(stdout),
                     dbg_SyslogChunker_chunkAddress(chunker),
                     dbg_SyslogChunker_chunkSize(chunker));
          }
        }
        else {
          dbg_clearSyslog();
        }
        return 0;
      }

    }
  }
}

SHELL_COMMAND(Syslog_Cmd, CMD_NAME, service::shell::syslog::usage, CMD_TOPIC, service::shell::syslog::main);
