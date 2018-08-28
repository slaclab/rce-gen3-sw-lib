// -*-Mode: C;-*-
/**
@file
@brief 

This file contains the definitions and prototypes for the shell.

@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
CONSOLE

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

#ifndef SERVICE_SHELL_SHELL_H
#define SERVICE_SHELL_SHELL_H

void Shell_Run(const char *device);

int Shell_Customize(void);

#endif
