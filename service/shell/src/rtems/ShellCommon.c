/* Support code for configuring shell commands in an RTEM
 *
 *  Author:  <Jim Panetta: panetta@slac.stanford.edu>
 *  Created: <2009-03-24>
 *  Time-stamp: <2009-05-01 14:38:28 panetta>
 *
 *
 *
 *
*/


#include <string.h>

#include <string.h>
#include <stdlib.h>

#include <rtems/rtems/tasks.h>

#include "ShellCommon.h" // this file's prototypes

#include "debug/print.h"  // for bugcheck
#include "svt/Svt.h"
#include "task/Task_Config.h"


// The location in the SVT to find the default task attrs
#define DEFAULT_TASK "DEFAULT_TASK_ATTRS"

Task_Attributes* _shell_task_attr = 0;
rtems_id         _last_task_id    = 0;

void
init_shell_default_attrs()
{
  uint32_t error;
  Task_Attributes* t =
    (Task_Attributes*)Svt_Translate(DEFAULT_TASK, SVT_APP_TABLE);

  error = t ? SVT_SUCCESS : SVT_NOSUCH_SYMBOL;
  if (!error) {
    if (!_shell_task_attr)
      _shell_task_attr = malloc(sizeof(Task_Attributes));
    memcpy(_shell_task_attr, t, sizeof(Task_Attributes));
  } else {
    dbg_bugcheck("Error looking up SVT symbol %s, cannot continue", DEFAULT_TASK);
  }  
}

Task_Attributes*
shell_default_attrs()
{
  return _shell_task_attr;
}

rtems_id
last_task_id()
{
  return _last_task_id;
}

void
set_last_task_id(rtems_id id)
{
  _last_task_id = id;
}

int task_suspend(rtems_id id) {
  rtems_status_code stat = rtems_task_suspend(id);
  if ((stat != RTEMS_SUCCESSFUL) ||
      (stat != RTEMS_ALREADY_SUSPENDED)) {
    dbg_printv("Shell task suspend for id %x failed\n", id);
  }
  return stat;
}

int task_resume(rtems_id id) {
  rtems_status_code stat = rtems_task_resume(id);
  if (stat != RTEMS_SUCCESSFUL) {
    dbg_printv("Shell task suspend for id %x failed\n", id);
  }
  return stat;
}

rtems_name
name_to_name(const char* name)
{
  if (!name) return rtems_build_name(' ', ' ', ' ', ' ');
  int l = strlen(name);
  rtems_name nm = rtems_build_name( (l>0 ? name[0] : ' '),
                                    (l>1 ? name[1] : ' '),
                                    (l>2 ? name[2] : ' '),
                                    (l>3 ? name[3] : ' '));
  return nm;
}

rtems_id
task_by_name(const char* name) {
  if (!name) return 0;
  rtems_name tmp = name_to_name(name);
  rtems_id tmpid;
  if (rtems_task_ident(tmp, 0, &tmpid) == RTEMS_SUCCESSFUL)
    return tmpid;
  return 0;
}

int task_exists(rtems_id id) {
  // This will break for SMP since is_suspended returns an error code
  rtems_status_code s = rtems_task_is_suspended(id);
  if (s == RTEMS_INVALID_ID)
    return 0;
  return 1;
}
