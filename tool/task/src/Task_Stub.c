// -*-Mode: C;-*-
/**
@cond development
@file Task_Stub.c
@brief Implement the task stub
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/


 
#include "task/Task.h"

/*
**
** Forward declarations for functions...
**
*/

int Task_Args(uint32_t* argv);


#define IS_RUNDOWN ((rtems_event_set)(1 << TASK_EVENT))

void Task_Entry(rtems_id delete_requestor)
 {

 uint32_t argv[TASK_MAX_ARGS];
 
 if(!delete_requestor) Task_Start(Task_Args(argv), (const char**)argv);
 
 Task_Rundown();
 
 if(delete_requestor)
   {
   rtems_event_send(delete_requestor, IS_RUNDOWN);
   rtems_task_suspend(RTEMS_SELF);
   }
 else
   rtems_task_delete(RTEMS_SELF);  
 }



/** @endcond */
