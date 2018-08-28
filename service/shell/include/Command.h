// -*-Mode: C++;-*-
/**
@file Command.hh
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

@par Credits:
SLAC
*/
#ifndef SERVICE_SHELL_COMMAND_H
#define SERVICE_SHELL_COMMAND_H

#include <rtems/shell.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef int (*Shell_MainFunc)(int, const char**);

/// @brief Wrapper structure for new shell commands
/// Encapsulates semantics for adding shell commands
/// to an existing RTEMS shell
typedef struct Shell_Command Shell_Command;
struct Shell_Command {
  /// @brief The name of the command to be used on the command line
  const char* name;

  /// @brief the usage string for this command
  const char* usage;

  /// @brief The RTEMS help topic where the usage string will be stored
  const char* topic;

  /// @brief return a function pointer to a proper main()
  Shell_MainFunc main;
};

int shell_command_install(Shell_Command* cmd);

#define SHELL_COMMAND(nm, cmd, usg, tpc, mn)        \
  Shell_Command nm = { cmd, usg, tpc, mn }

  
#if defined(__cplusplus)
}
#endif


#endif
