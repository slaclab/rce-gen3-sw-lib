// -*-Mode: C++;-*-
/**
@file Command.cc
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
Service %Shell

@par Abstract:


@author
S Maldonado <smaldona@slac.stanford.edu>

@par Date created:
2014/01/18

@par Last commit:
\$Date: $ by \$Author: $.

@par Revision number:
\$Revision: $

@par Location in repository:
\$HeadURL: $

@par Credits:
SLAC
*/

#include <stdio.h>



#include <rtems/shell.h>

#include "task/Task_Config.h"
#include "task/Task.h"
#include "elf/linker.h"
#include "ldr/Ldr.h"
#include "svt/Svt.h"

#define NO_ARGC 0                 // startup task has does not use run-time arguments...
#define NO_ARGV (const char**)0   // ditto...

int const lnk_options = LNK_INSTALL;

extern rtems_shell_cmd_t rtems_shell_IFCONFIG_Command;
extern rtems_shell_cmd_t rtems_shell_ROUTE_Command;
extern rtems_shell_cmd_t rtems_shell_NETSTATS_Command;
extern rtems_shell_cmd_t rtems_shell_PING_Command;

rtems_shell_cmd_t *rtems_shell_ext_commands[] = 
  {
  &rtems_shell_IFCONFIG_Command,
  &rtems_shell_ROUTE_Command,
  &rtems_shell_NETSTATS_Command,
  &rtems_shell_PING_Command, 
  NULL
  };
  
int lnk_prelude(void *prefs, void *elfHdr)
  {
  rtems_shell_cmd_t **c;
  
  for ( c = rtems_shell_ext_commands ; *c  ; c++ )
    {
    rtems_shell_add_cmd_struct( *c );
    }

  return 0;
  }
  
