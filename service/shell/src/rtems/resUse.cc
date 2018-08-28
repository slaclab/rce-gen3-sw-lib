/*!
*
* @brief     Print RTEMS resource usage
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      October 17, 2013 -- Created
*
* $Revision: 3123 $
*
* @verbatim:
*                               Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

#include <stdio.h>
#include <string.h>

#include <rtems.h>


#include "shell/Command.h"

#define CMD_NAME  "resUse"
#define CMD_TOPIC "rtems"

namespace service {
  namespace shell {
    namespace resUse {

      const char* usageLong =  "Usage:\n"
        "  resuse \n"
        "  Show the number of RTEMS objects configured and in use.\n";

      const char* usage = "Show the number of RTEMS objects configured and in use";


      int main(int argc, const char **argv) {
        for (int c  = rtems_object_api_minimum_class(OBJECTS_CLASSIC_API);
             c <= rtems_object_api_maximum_class(OBJECTS_CLASSIC_API);
             ++c)
        {
          rtems_object_api_class_information info;
          rtems_object_get_class_information(OBJECTS_CLASSIC_API, c, &info);
          printf("%lu out of %lu %ss in use (%s limit)\n",
                 info.maximum - info.unallocated,
                 info.maximum,
                 rtems_object_get_api_class_name(OBJECTS_CLASSIC_API, c),
                 info.auto_extend ? "extensible" : "fixed");
        }
        printf("\n");

        return 0;
      }
    }
  }
}

SHELL_COMMAND(ResUse_Cmd, CMD_NAME, service::shell::resUse::usage, CMD_TOPIC, service::shell::resUse::main);
