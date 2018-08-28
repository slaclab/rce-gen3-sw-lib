// -*-Mode: C++;-*-
/**
@file reboot.cc
@verbatim
                               Copyright 2010
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
Service %Shell

@par Abstract:
Command to reboot the RCE

@author
Jim Panetta <panetta@slac.stanford.edu>

@par Date created:
2010/10/04

@par Last commit:
\$Date: 2015-07-30 13:31:00 -0700 (Thu, 30 Jul 2015) $ by \$Author: smaldona $.

@par Revision number:
\$Revision: 4245 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/service/shell/src/rtems/reboot.cc $

@par Credits:
SLAC
*/

#define __need_getopt_newlib
#include <getopt.h>
#include <stdlib.h>

#include "shell/Command.h"
#include "cpu/cpu.h"
#include "debug/print.h"
#include "system/fs.h"

#include "shell/bootmode.h"

#define DEFAULT_RESET_LEVEL 0x2

#define CMD_NAME  "reboot"
#define CMD_TOPIC "shell"

namespace service {
  namespace shell {
    namespace reboot {

      const char* usage =
        "Usage:  " CMD_NAME " [-t <rtems|linux|ramdisk>] [-b <0|1>]\n"
        "  Reboot the system\n"
        "  Optional arguments: \n"
        "  -t <rtems|linux|ramdisk>: Sets the default boot operating system\n"
        "  -b <0|1>: Sets the default bitfile load directive\n"
        "\n";
        
      /** @internal @brief What was parsed from the command line. */
      struct CommandLine {
        const char*  os;         ///< @internal @brief The target OS argument.
        const char*  loadbit;    ///< @internal @brief The target loadbit argument.        
      };
        
      int rebootParseOptions(int argc, const char **argv, CommandLine &cmd) {
        int status = 0;
        int theOpt;
        /* Use reentrant form of getopt as per RTEMS Shell Users Guide (4.9.99.0) */
        struct getopt_data getopt_reent;
        memset(&getopt_reent, 0, sizeof(getopt_data));
        while ( (theOpt = getopt_r(argc,
                                   const_cast<char* const*>(argv),
                                   "t:b:", &getopt_reent)) != EOF ) {
          switch(theOpt) {
          printf("The opt %s\n",theOpt);
          case 't':
            cmd.os = getopt_reent.optarg;
            status |= 1;
            break;
          case 'b':
            cmd.loadbit = getopt_reent.optarg;
            status |= 2;
            break;
          default:
            dbg_printv("Option: -%c is not parsable.\n", theOpt);
            dbg_printv(usage);            
            return -1;
          }
        }
        return status;
      }
        
      int main(int argc, const char** argv) {
        CommandLine cmd;
        int status = 0;
        unsigned level __attribute__((unused));
        
        // Scan the command line.
        int const parseRet = rebootParseOptions(argc, argv, cmd);
        if (parseRet == -1)
          return parseRet;        

        if (parseRet & 1)
          {
          if(strcmp(cmd.os,"rtems") == 0)
            cmd.os = RTEMS;
          else if(strcmp(cmd.os,"linux") == 0)
            cmd.os = LINUX;
          else if(strcmp(cmd.os,"ramdisk") == 0)
            cmd.os = RAMDISK;
          else
            {
            dbg_printv("Invalid reboot target os %s:\n",cmd.os);            
            return 2;
            }
          }
        else
          cmd.os = NULL;
        
        if(parseRet & 2)
          {
          if(strcmp(cmd.loadbit,"0") == 0)
            cmd.loadbit = LOADBIT0;
          else if(strcmp(cmd.loadbit,"1") == 0)
            cmd.loadbit = LOADBIT1;
          else
            {
            dbg_printv("Invalid loadbit %s:\n",cmd.loadbit);            
            return 2;
            }
          }
        else
          cmd.loadbit = NULL;
            
        if(parseRet & 3)
          {
          status = set_bootmode(cmd.os,cmd.loadbit);
          }  

        if (status)
          {
          dbg_printv("Error setting boot options: %d\n",status);
          return 2;
          }
  
        /* unmount the rtems application partition */
        unmount(SD_MOUNT_POINT_RTEMSAPP);
       
        /* unmount the rtems partition */
        unmount(SD_MOUNT_POINT_RTEMS);

        /* unmount the scratch partition */
        unmount(SD_MOUNT_POINT_SCRATCH);
           
        rtems_interrupt_disable(level);
        TOOL_CPU_reset(DEFAULT_RESET_LEVEL);

      }
    }
  }
}

SHELL_COMMAND(Reboot_Cmd, CMD_NAME, service::shell::reboot::usage, CMD_TOPIC, service::shell::reboot::main);

