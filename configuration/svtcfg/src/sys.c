// -*-Mode: C;-*-
/**
@file
@brief 

This file contains the definitions and prototypes for the system symbol value table.

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
#include "network/BsdNet_Config.h"
#include "shell/shell_prefs.h"
#include "telnet/telnet_prefs.h"

/* system task attributes */

Task_Attributes const INIT_CONSOLE_TASK = {
  .name       = rtems_build_name('C','S','H','L'),
  .stack_size = RTEMS_MINIMUM_STACK_SIZE * 10,
  .priority   = 100,
  .attributes = RTEMS_DEFAULT_ATTRIBUTES,
  .modes      = RTEMS_DEFAULT_MODES,
  .image      = NULL, 
  .argc       = 0,
  .argv       = (const char**)0
  };    

Task_Attributes const INIT_DSL_TASK = {
  .name       = rtems_build_name('D','S','L','D'),
  .stack_size = RTEMS_MINIMUM_STACK_SIZE * 10,
  .priority   = 150,
  .attributes = RTEMS_DEFAULT_ATTRIBUTES,
  .modes      = RTEMS_DEFAULT_MODES,
  .image      = NULL, 
  .argc       = 0,
  .argv       = (const char**)0
  };    

Task_Attributes const PGP0_TASK_ATTRS = {
  .name       = rtems_build_name('P','G','P','0'),
  .stack_size = RTEMS_MINIMUM_STACK_SIZE * 10,
  .priority   = 80,
  .attributes = RTEMS_DEFAULT_ATTRIBUTES,
  .modes      = RTEMS_DEFAULT_MODES,
  .image      = (const char*)0,
  .argc       = 0,
  .argv       = (const char**)0
};

Task_Attributes const PGP1_TASK_ATTRS = {
  .name       = rtems_build_name('P','G','P','1'),
  .stack_size = RTEMS_MINIMUM_STACK_SIZE * 10,
  .priority   = 80,
  .attributes = RTEMS_DEFAULT_ATTRIBUTES,
  .modes      = RTEMS_DEFAULT_MODES,
  .image      = (const char*)0,
  .argc       = 0,
  .argv       = (const char**)0
};

Task_Attributes const PGP2_TASK_ATTRS = {
  .name       = rtems_build_name('P','G','P','2'),
  .stack_size = RTEMS_MINIMUM_STACK_SIZE * 10,
  .priority   = 80,
  .attributes = RTEMS_DEFAULT_ATTRIBUTES,
  .modes      = RTEMS_DEFAULT_MODES,
  .image      = (const char*)0,
  .argc       = 0,
  .argv       = (const char**)0
};

Task_Attributes const XAUI_TASK_ATTRS = {
  .name       = rtems_build_name('X','A','R','X'),
  .stack_size = RTEMS_MINIMUM_STACK_SIZE * 10,
  .priority   = 80,
  .attributes = RTEMS_DEFAULT_ATTRIBUTES,
  .modes      = RTEMS_DEFAULT_MODES,
  .image      = (const char*)0,
  .argc       = 0,
  .argv       = (const char**)0
};

/* library preferences */

Shell_Prefs const SHELL_PREFERENCES = {
  .exit_shell    = 0,
  .forever       = 0,
  .echo          = 0,
  .output_append = 0,
  .login_check   = 0,
  };

Telnet_Prefs const TELNET_PREFERENCES = {
  .priority      = 100,
  .stack_size    = RTEMS_MINIMUM_STACK_SIZE * 10,
  .login_check   = NULL
  };

/* network configuration */

BsdNet_Attributes const BSDNET_ATTRS = {
  .hostname               = NULL, // NULL: use dhcp default
  .domainname             = NULL, // NULL: use dhcp default
  .gateway                = NULL, // NULL: use dhcp default
  .log_host               = NULL, // NULL: use dhcp default
  .name_server1           = NULL, // NULL: use dhcp default
  .name_server2           = NULL, // NULL: use dhcp default
  .name_server3           = NULL, // NULL: use dhcp default
  .ntp_server1            = NULL, // NULL: use dhcp default
  .ntp_server2            = NULL, // NULL: use dhcp default
  .ntp_server3            = NULL, // NULL: use dhcp default
  .use_dhcp               = true, // true: enable dhcp for BOOTP
  .priority               = 80,   // zero: use rtems default 
  .sb_efficiency          = 0,    // zero: use rtems default
  .udp_tx_buf_size        = 0,    // zero: use rtems default
  .udp_rx_buf_size        = 0,    // zero: use rtems default
  .tcp_tx_buf_size        = 0,    // zero: use rtems default
  .tcp_rx_buf_size        = 0,    // zero: use rtems default
  .mbuf_bytecount         = 0,    // zero: use driver default
  .mbuf_cluster_bytecount = 0,    // zero: use driver default
  };
  
/* dsl services */

const char DSL_ATCA[] = "system:atca.so";
