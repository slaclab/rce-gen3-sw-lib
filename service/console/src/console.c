// -*-Mode: C;-*-
/**
@file
@brief 

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

@par Location in repository:
\$HeadURL: $

@par Credits:
SLAC

*/



#include "task/Task.h"
#include "svt/Svt.h"
#include "debug/print.h"
#include "shell/shell.h"

static const char CONSOLE_TASK[] = "INIT_CONSOLE_TASK";
static const char _prefs_error[] = "Console task prefs lookup failed for %s\n";

/*
** ++
**
**
** --
*/

void Task_Start(int argc, const char** argv)
  {
  Shell_Run("/dev/console");
  }

/*
** ++
**
**
** --
*/

void Task_Rundown()
 {
 return;
 }

/*
** ++
**
**
** --
*/

int lnk_prelude(Task_Attributes *t, void *elfHdr) 
  {
  rtems_id id;
  
  if(!t)
    {
    t = (Task_Attributes*)Svt_Translate(CONSOLE_TASK, SVT_SYS_TABLE);  
    if(!t) dbg_bugcheck(_prefs_error, CONSOLE_TASK);
    }
      
  /* launch the console task */
  return Task_Run((Ldr_elf *)elfHdr,t,t->argc,t->argv,&id);
  }
