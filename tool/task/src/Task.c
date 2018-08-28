// -*-Mode: C;-*-
/**
@cond development
@file Task.c
@brief Implement RTEMS Task semantics.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

#include "elf/linker.h"
#include "ldr/Ldr.h"
#include "task/Task.h"

/*
**
** Forward declarations for local functions...
**
*/

static void _set(const uint32_t* argv, int argc, rtems_id t);

#define START 0

Task_status Task_Run(Ldr_elf *elf, const Task_Attributes* t, int argc, const char** argv, rtems_id* returned_id)
 {

 uint32_t arg_cnt = argc ? argc : t->argc;
 
 if(arg_cnt > TASK_MAX_ARGS) return RTEMS_INVALID_NUMBER;
 
 Task_status error = rtems_task_create(t->name,
                                       t->priority,
                                       t->stack_size,
                                       t->modes,
                                       t->attributes,
                                       returned_id);
                                       
 if(error != RTEMS_SUCCESSFUL) return error;
  
 rtems_id id = *returned_id;

 const uint32_t* arg_vec = argc ? (const uint32_t*)argv : (const uint32_t*)t->argv;
 
 _set(arg_vec, arg_cnt, id);
    
 rtems_task_entry entry_point = (rtems_task_entry)Ldr_Entry(elf);

 error = rtems_task_start(id, entry_point, START);
 
 if(error)
   {
   rtems_task_delete(id);
   }
 
 return error;
 }


static void _set(const uint32_t* argv, int argc, rtems_id t)
 {
 
 uint32_t remaining = argc;
 uint32_t note      = 0;

 rtems_task_set_note(t, note++, remaining); 

 const uint32_t* next = argv;
 
 while(remaining--) rtems_task_set_note(t, note++, *next++); 
  
 return; 
 }

#define IS_RUNDOWN ((rtems_event_set)(1 << TASK_EVENT))
#define WAIT       (RTEMS_EVENT_ALL | RTEMS_WAIT)

Task_status Task_Stop(rtems_id t)
 {
 
 Task_status error = rtems_task_restart(t, (rtems_task_argument)rtems_task_self());
 
 if(error) return error;
 
 rtems_event_set returned_events;
 
 error = rtems_event_receive(IS_RUNDOWN, WAIT, TASK_RUNDOWN_TMO, &returned_events); 

 return !error ? rtems_task_delete(t) : error;
 }


int Task_Args(uint32_t* argv)
 {
 
 uint32_t  note = 0;
 uint32_t* next = argv;
 
 rtems_task_get_note(RTEMS_SELF, note, next);
 rtems_task_set_note(RTEMS_SELF, note, 0);    // Clean up notepads. See JIRA CCI-78.

 uint32_t argc      = *next;
 uint32_t remaining = argc;
  
 while(remaining--) {
   rtems_task_get_note(RTEMS_SELF, ++note, next++);
   rtems_task_set_note(RTEMS_SELF, note, 0);
 }
  
 return (int)argc; 
 }



/** @endcond */

// Documentation

// #defines
/**
   @def TASK_EVENT
   @brief The event number reserved for Tasking.

   @def TASK_RUNDOWN_TMO
   @brief Maximum execution time (in tics) of Task_Rundown

*/

// typedefs
/**
   @typedef Task_status
   @brief Status returned from Task_Run and Task_Stop.
   Zero (0) is success, all other values refer to an underlying
   error, typically in the <a href="../../ldr/html/index.html">Ldr Facility</a>.

 */

// functions
/**
   @fn Task_status Task_Run(Ldr_elf *elf, const Task_Attributes* attr, int argc, const char** argv, rtems_id* id)
   @brief Construct and start an RTEMS task whose attributes are specified by the caller.
   @param[in] elf A pointer to the
      <a href="../../elf/html/index.html">elf</a> object.
   @param[in] attr A pointer to the Task_Attributes structure which
      describes the RTEMS parameters for the Task (task name, stack size, etc).
   @param[in] argc The number of arguments to the Task.  0 to ::TASK_MAX_ARGS.
   @param[in] argv The argument vector for the Task.  This is a
      pointer to a set of ponters to the argument strings, as is used
      with @c main.  If @c argc has a zero value, this argument is
      ignored (but should be set to NULL (0)).
   @param[out] id The resulting RTEMS ID.
   @return Zero (0) for success, nonzero for failure, in which case
   the value enumerates the reason for failure.

   Abstractly this routine creates the specified task, then starts the
   task, transferring control to the corresponding executable along
   with a specified set of startup parameters.  
   See ::Task_Start for a description on how the executable
   code associated with created tasks is first entered.

   @note The value written to the @c id location will only be valid
   if the function returns successfully.

   
   @fn Task_status Task_Stop(rtems_id id)
   @brief Stop and delete a task started with the ::Task_Run.
   @param[in] id The Task ID of the task to be deleted (as returned by ::Task_Run).
   @return Zero (0) for success (the function was successful and the
   task was deleted), nonzero for failure, in which case the value
   enumerates the reason for failure.

   Note that this function must NOT be called from the task to be
   deleted. Before the specified task is actually deleted the task's
   "rundown" function is first called (that is, in the context of the
   deleted task). Its function is to return any resources previously
   garnered by the (to be) deleted task. The signature for the entry
   point of that function is given by ::Task_Rundown.


   
   @fn void Task_Start(int argc, const char** argv)
   @brief The signature of the initial entry point for a task created through ::Task_Run.
   @param[in] argc The number of arguments to the Task.  0 to ::TASK_MAX_ARGS.
   @param[in] argv The argument vector for the Task.  This is a
      pointer to a set of ponters to the argument strings, as is used
      with @c main.  If @c argc has a zero value, this argument is
      ignored (but should be set to NULL (0)).

   For any given task the user must provide one, and only one
   implementation of this function. This function must be called once,
   and only once and that is automatically done by ::Task_Run
   immediately after the task is created and started, so no user
   calling is required, only implementation.

   This function returns no value. However, if this function returns
   after being called the task will be stopped. It will be stopped as if
   ::Task_Stop had been called. That is, first ::Task_Rundown
   will be invoked and then the corresponding RTEMS task deleted.

   
   @fn void Task_Rundown()
   @brief The signature of the "rundown" entry pont for a task created through ::Task_Run.

   For any given task the user must provide one, and only one
   implementation of this function. This function will then be called
   once, and only once in the lifetime of the corresponding task. Its
   function is return or "rundown" any resources garned by the task
   during its running. This function must never be directly invoked by
   the caller. Instead, its indirectly invoked by either another task
   calling ::Task_Stop or by the task itself returning from
   ::Task_Start.

   @note This function must complete execution within the timeout
   specified by ::TASK_RUNDOWN_TMO.


   


 */
