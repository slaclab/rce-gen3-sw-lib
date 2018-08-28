// -*-Mode: C++;-*-
/**
@file Command.cc
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
Framework for adding new commands "on the fly" to the RTEMS shell

@author
Jim Panetta <panetta@slac.stanford.edu>

@par Date created:
2010/10/04

@par Last commit:
\$Date: 2015-07-14 10:55:53 -0700 (Tue, 14 Jul 2015) $ by \$Author: smaldona $.

@par Revision number:
\$Revision: 4078 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/service/shell/src/rtems/Command.c $

@par Credits:
SLAC
*/

#include "shell/Command.h"

#include <stdio.h>

int shell_command_install(Shell_Command* cmd)
{                                   
  rtems_shell_cmd_t * retCmd;
  // Call the long form for adding this command.  This
  // allocates the structure space inside RTEMS
  if (!(retCmd = rtems_shell_add_cmd(cmd->name,
                                     cmd->topic,
                                     cmd->usage,
                                     (rtems_shell_command_t)cmd->main)))
    {
      printf("Error installing shell command %s",cmd->name);
      return;
    }
  }

