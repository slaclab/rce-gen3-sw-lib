/* Headers for common shell routines
 *
 *  Author:  <Jim Panetta: panetta@slac.stanford.edu>
 *  Created: <2009-03-24>
 *  Time-stamp: <2009-05-01 14:38:32 panetta>
 *
*/

#ifndef SERVICE_SHELL_RTEMS_SHELL_COMMON_H
#define SERVICE_SHELL_RTEMS_SHELL_COMMON_H



#include "task/Task_Config.h"

#if defined(__cplusplus)
extern "C" {
#endif

  // Perform a lookup on a task by ID
  int task_exists(rtems_id);

  // Suspend a task by ID
  int task_suspend(rtems_id);

  // Resume a suspended task by ID
  int task_resume(rtems_id);

 
  // Initialize shell default task attributes
  void init_shell_default_attrs(void);

  // Retrieve the shell default task attributes
  Task_Attributes* shell_default_attrs(void);

  // Find and return the last shell task started
  rtems_id  last_task_id(void);

  // Cache the last started task ID
  void set_last_task_id(rtems_id);

  // Perform a lookup on a task by name, returning the ID
  rtems_id task_by_name(const char*);

  // Safely convert a string style name to the RTEMS name type.
  // If the string length is less than 4, the returned name
  // will be padded with spaces.
  rtems_name name_to_name(const char*);


  
#if defined(__cplusplus)
}
#endif
#endif
