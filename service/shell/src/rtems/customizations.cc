// -*-Mode: C++;-*-
/**
@file customizations.cc
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
RTEMS shell standard customizations

@author
Jim Panetta <panetta@slac.stanford.edu>

@par Date created:
2010/10/04

@par Last commit:
\$Date: 2015-07-14 10:55:53 -0700 (Tue, 14 Jul 2015) $ by \$Author: smaldona $.

@par Revision number:
\$Revision: 4078 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/service/shell/src/rtems/customizations.cc $

@par Credits:
SLAC
*/

#include "shell/Command.h"

extern "C" {
#include "shell/shell.h"
#include "ShellCommon.h"
}

extern Shell_Command Lookup_Cmd;
extern Shell_Command NsAssign_Cmd;
extern Shell_Command NsMap_Cmd;
extern Shell_Command NsRemove_Cmd;
extern Shell_Command NsRename_Cmd;
extern Shell_Command Reboot_Cmd;
extern Shell_Command ResUse_Cmd;
extern Shell_Command ResumeTask_Cmd;
extern Shell_Command RunTask_Cmd;
extern Shell_Command StopTask_Cmd;
extern Shell_Command SuspendTask_Cmd;
extern Shell_Command Syslog_Cmd;
extern Shell_Command SysInfo_Cmd;
extern Shell_Command LoadShareable_Cmd;
extern Shell_Command STACKSPACE_Cmd;

Shell_Command *_customCommands[] = {
  &Lookup_Cmd,
  &NsAssign_Cmd,
  &NsMap_Cmd,
  &NsRemove_Cmd,
  &NsRename_Cmd,
  &Reboot_Cmd,
  &ResUse_Cmd,
  &ResumeTask_Cmd,
  &RunTask_Cmd,
  &StopTask_Cmd,
  &SuspendTask_Cmd,
  &Syslog_Cmd,
  &SysInfo_Cmd,
  &LoadShareable_Cmd,
  &STACKSPACE_Cmd,
  NULL
};

  
int Shell_Customize() {

  // Retrieve the default task attributes from app svt and persist
  // a local copy
  init_shell_default_attrs();

  // install all command extensions
  Shell_Command **c;
  for (c=_customCommands; *c; c++)
    shell_command_install(*c);
  
  return 0;
}
