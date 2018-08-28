// -*-Mode: C;-*-
/**
@file
@brief 

This file contains the definition of the "Telnet Preferences" structure. Instances of this
structure are used to specify the attributes of a telnet.

@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
TELNET

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

#ifndef TELNET_PREFS_H
#define TELNET_PREFS_H

#include <rtems.h>
#include <rtems/shell.h>

typedef struct {
  rtems_task_priority priority;
  size_t stack_size;
  rtems_shell_login_check_t login_check;
} Telnet_Prefs;

#endif
