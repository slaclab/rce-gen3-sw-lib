// -*-Mode: C;-*-
/**
@file
@brief 

This file contains the definitions and prototypes for the application symbol value table.

@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
configuration

@author
Sergio Maldonado, SLAC (smaldona@slac.stanford.edu)

@par Date created:
2014/01/13

@par Last commit:
\$Date: $ by \$Author: $.

@par Revision number:
\$Revision: $

@par Location in repository:
\$HeadURL: $

@par Credits:
SLAC

*/



#include <rtems.h>

#include "task/Task_Config.h"

/* Defaults used by tasking commands in shell */

Task_Attributes const DEFAULT_TASK_ATTRS = {
  .name       = rtems_build_name('D', 'F', 'L', 'T'),
  .stack_size = RTEMS_MINIMUM_STACK_SIZE * 10,
  .priority   = 150,
  .attributes = RTEMS_DEFAULT_ATTRIBUTES,
  .modes      = RTEMS_DEFAULT_MODES,
  .image      = (const char*)0,
  .argc       = 0,
  .argv       = (const char**)0
};

/* System startup services */

const char* INIT_STARTUP_SERVICES[] = {
  "system:console.exe",
  "system:ethXaui.so",
  /* "system:ethEmacPs.so",*/
  "system:nfs.so",
  "system:shellx.so",
  "system:telnet.so",
  "system:dsld.exe",
  "system:dsl_reboot.so",
  "system:dsl_update.so",  
  "system:dsl_identifier.so",
  "config:appinit.so",
  NULL
  };

/* dsl services */

const char* INIT_DSL_SERVICES[] = {
  "DSL_ATCA",
  NULL
  };

