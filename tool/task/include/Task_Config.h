// -*-Mode: C;-*-
/**
@file Task_Config.h
@brief Define the Task_Attributes structure.  Instances of this
structure are used to specify the attributes of a task created through the Task facility.
The public interface for that facility is found in Task.h.

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

#ifndef TOOL_TASK_CONFIG_H
#define TOOL_TASK_CONFIG_H

#include "rtems.h"

#define TASK_MAX_ARGS 15 // Maximum number of parameters to be passed to a created task.

typedef struct {
  rtems_name          name;       // Task name (four characters)
  size_t              stack_size; // Task stack size (e.g. "RTEMS_MINIMUM_STACK_SIZE")
  rtems_task_priority priority;   // Task priority (255 (lowest) to 1 (highest)) 
  rtems_attribute     attributes; // Task attributes (e.g. "RTEMS_DEFAULT_ATTRIBUTES")
  rtems_mode          modes;      // Task mode (e. g. "RTEMS_DEFAULT_MODES")
  const char*         image;      // Path specification for executable image
  int                 argc;       // # of task parameters (MBZ (0) if none)
  const char**        argv;       // Vector of task parameters...
  } Task_Attributes;    

#endif
