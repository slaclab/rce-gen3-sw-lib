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
2014/01/12

@par Last commit:
\$Date: $ by \$Author: $.

@par Revision number:
\$Revision: $

@par Location in repository:
\$HeadURL: $

@par Credits:
SLAC
*/



#define CONFIGURE_SHELL_COMMANDS_INIT
#define CONFIGURE_SHELL_COMMANDS_ALL
#define CONFIGURE_SHELL_NO_COMMAND_NETSTATS
#define CONFIGURE_SHELL_NO_COMMAND_ROUTE
#define CONFIGURE_SHELL_NO_COMMAND_IFCONFIG
#define CONFIGURE_SHELL_NO_COMMAND_PING

#include <rtems/shellconfig.h>

#include "elf/linker.h"
#include "svt/Svt.h"  
#include "shell/shell.h"
#include "shell/shell_prefs.h"

static const char SHELL_PREFS[]   = "SHELL_PREFERENCES";
static const char _prefs_error[]  = "Shell task prefs lookup failed for symbol %s\n";

static Shell_Prefs *_prefs;

int const lnk_options = LNK_INSTALL;

void Shell_Run(const char *device)
  {
  Shell_Prefs *prefs = _prefs;
  rtems_shell_env_t  myShellEnv = rtems_global_shell_env;

  myShellEnv.devname       = device;
  myShellEnv.taskname      = "CSHL";
  myShellEnv.exit_shell    = prefs->exit_shell;
  myShellEnv.forever       = prefs->forever;
  myShellEnv.echo          = prefs->echo;
  myShellEnv.input         = strdup("stdin");  // Must be free-able
  myShellEnv.output        = strdup("stdout"); // Must be free-able
  myShellEnv.output_append = prefs->output_append;
  myShellEnv.login_check   = prefs->login_check;

  rtems_shell_main_loop( &myShellEnv );

  return;
  }
  
int lnk_prelude(Shell_Prefs *prefs, void *elfHdr)
  {
  if(!prefs)
    {
    prefs = (Shell_Prefs*)Svt_Translate(SHELL_PREFS, SVT_SYS_TABLE);
    int error = prefs ? SVT_SUCCESS: SVT_NOSUCH_SYMBOL;
    if(error) dbg_bugcheck(_prefs_error, SHELL_PREFS);
    }
  
  _prefs = prefs;
  
  /* call routine which customizes the shell command set */
  return Shell_Customize();
  }
