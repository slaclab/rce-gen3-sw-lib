// -*-Mode: C;-*-
/**
@file
@brief 

This file contains the definition of the "Shell Preferences" structure. Instances of this
structure are used to specify the attributes of a shell.

@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
SHELL

@author
Sergio Maldonado, SLAC (smaldona@slac.stanford.edu)

@par Date created:
2014/01/13

@par Last commit:
\$Date: $ by \$Author: $.

@par Revision number:
\$Revision: $

@par Credits:
SLAC

*/

#ifndef SERVICE_SHELL_SHELL_PREFS_H
#define SERVICE_SHELL_SHELL_PREFS_H

#include <rtems/shell.h>

typedef struct {
  bool exit_shell;
  bool forever;
  bool echo;
  bool output_append;
  rtems_shell_login_check_t login_check;
} Shell_Prefs;

#endif
