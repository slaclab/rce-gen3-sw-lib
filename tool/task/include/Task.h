// -*-Mode: C;-*-
/**
@file Task.h
@brief The PUBLIC interface to start and stop TASKs. 


@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@author
Michael Huffer, SLAC (mehsys@slac.stanford.edu)
*/

#ifndef TOOL_TASK_TASK_H
#define TOOL_TASK_TASK_H



#include "ldr/Ldr.h"
#include "task/Task_Config.h"

#define TASK_EVENT 31         // This event number is reserved for this package...
#define TASK_RUNDOWN_TMO  100 // maximum execution (in tics) time of rundown function...

typedef uint32_t Task_status; // Status return (zero (0) is always success)...

#if defined(__cplusplus)
extern "C" {
#endif

Task_status Task_Run(Ldr_elf *elf, const Task_Attributes*, int argc, const char** argv, rtems_id*);

Task_status Task_Stop(rtems_id);

void Task_Start(int argc, const char** argv);

void Task_Rundown();

#if defined(__cplusplus)
} // extern "C"
#endif

#endif
