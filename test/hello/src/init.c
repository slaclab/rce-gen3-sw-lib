/*
 *  COPYRIGHT (c) 1989-2012.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <bsp.h> /* for device driver prototypes */

#include <stdio.h>
#include <stdlib.h>

/* forward declarations to avoid warnings */
rtems_task Init(rtems_task_argument argument);

rtems_task Init(
  rtems_task_argument ignored
)
{
  printf( "\n\n*** HELLO WORLD TEST ***\n" );
  printf( "Hello World\n" );
  printf( "*** END OF HELLO WORLD TEST ***\n" );
  exit( 0 );
}


/* NOTICE: the clock driver is explicitly disabled */
#define CONFIGURE_APPLICATION_DOES_NOT_NEED_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_MAXIMUM_TASKS            1
#define CONFIGURE_USE_DEVFS_AS_BASE_FILESYSTEM

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#include <bsp/vectors.h>

static void fatal_extension(
   rtems_fatal_source source,
   bool is_internal,
   rtems_fatal_code code
)
{
  //if ( source == RTEMS_FATAL_SOURCE_POWERPC_EXCEPTION ) {
  {
     const BSP_Exception_frame *frame =
       (const BSP_Exception_frame *) code;

     ppc_exc_print_frame_and_context( frame );
   }
}

#define CONFIGURE_INIT

#define CONFIGURE_INITIAL_EXTENSIONS { .fatal = fatal_extension }

#include <rtems/confdefs.h>
