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
RTS

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
#include "dsl/Server.hh"         

using service::dsl::Server;

#if defined(__cplusplus)
extern "C" {
#endif

static const char DSL_TASK[]      = "INIT_DSL_TASK";
static const char DSL_SERVICES[]  = "INIT_DSL_SERVICES";
static const char _prefs_error[]  = "Dsl task prefs lookup failed for symbol %s\n";

/*
** ++
**
**
** --
*/

void Task_Start(int argc, const char** argv)
  {
  Server::execute();
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
  const char** services;
  
  if(!t)
    {
    t = (Task_Attributes*)Svt_Translate(DSL_TASK, SVT_SYS_TABLE);  
    if(!t) dbg_bugcheck(_prefs_error, DSL_TASK);
    }
    
  /* find the list of services to start */
  services = (const char**)Svt_Translate(DSL_SERVICES, SVT_APP_TABLE);  

  /* instantiate the server */
  Server::startup(services);
  
  /* launch the server task */
  return Task_Run((Ldr_elf *)elfHdr,t,t->argc,t->argv,&id);
  }

#if defined(__cplusplus)
} // extern "C"
#endif

