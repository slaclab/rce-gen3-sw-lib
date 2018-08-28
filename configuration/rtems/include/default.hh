#ifndef CONFIGURATION_RTEMS_DEFAULT_HH
#define CONFIGURATION_RTEMS_DEFAULT_HH

// See the RTEMS C User's Guide Chapter 23, "Configuring a System"
// Modified for RTEMS 4.10.0.

// The file configure_template.cc lists all the available options and
// gives a brief explanation of each.

#include <rtems.h>

#define CONFIGURE_INIT

/////////////////////////////
// 23.2.1: Library support //
/////////////////////////////
#define CONFIGURE_MALLOC_STATISTICS
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS (20)
#define CONFIGURE_MAXIMUM_PTYS (4)
#define CONFIGURE_STACK_CHECKER_ENABLED
#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK
#define CONFIGURE_FILESYSTEM_DOSFS

//////////////////////////////////////
// 23.2.2: Basic system information //
//////////////////////////////////////

// These RCE-specific definitions will be used both here and in the
// section setting limits on the numbers of classic RTEMS objects. We
// expect that most applications will use the queue types defined in
// the RCE core so our limits here are rather small.
#ifdef __cplusplus
namespace {
#endif
  enum {RCE_MAX_RTEMS_MSG_QUEUES =  10,
        RCE_MAX_BYTES_PER_MSG    =   8, // Enough for a regular pointer or a std::tr1::smart_ptr.
        RCE_MAX_MSGS_PER_QUEUE   = 100
  };
#ifdef __cplusplus
}
#endif

#define CONFIGURE_MESSAGE_BUFFER_MEMORY \
  (RCE_MAX_RTEMS_MSG_QUEUES * \
   CONFIGURE_MESSAGE_BUFFERS_FOR_QUEUE(RCE_MAX_MSGS_PER_QUEUE,RCE_MAX_BYTES_PER_MSG)\
  )

// Some applications are known to require extra stack space, e.g.,
// Martin Kocian's PGP test.
#define CONFIGURE_EXTRA_TASK_STACKS (256*1024)

/////////////////////////////////
// 23.2.4: Device driver table //
/////////////////////////////////
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
// RiC 130619 - Commented out b/c 4.11 can't have both CLOCK _and_ TIMER
//#define CONFIGURE_APPLICATION_NEEDS_TIMER_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_NULL_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_STUB_DRIVER
#define CONFIGURE_MAXIMUM_DRIVERS (11)

/////////////////////////
// 23.2.6: Classic API //
/////////////////////////
#define CONFIGURE_MAXIMUM_TASKS (50)
#define CONFIGURE_MAXIMUM_TIMERS (20)
#define CONFIGURE_MAXIMUM_SEMAPHORES (100)
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES (RCE_MAX_RTEMS_MSG_QUEUES)
#define CONFIGURE_MAXIMUM_PARTITIONS (10)
#define CONFIGURE_MAXIMUM_REGIONS (10)
#define CONFIGURE_MAXIMUM_PORTS (10)
#define CONFIGURE_MAXIMUM_PERIODS (10)
#define CONFIGURE_MAXIMUM_USER_EXTENSIONS (5)
// One of the user extensions is defined by the BSP to delete tasks
// that exit from their main functions (entry points). Normally RTEMS
// treats that as an error.

//////////////////////////////////////////
// 23.2.7: Classic API Init-tasks table //
//////////////////////////////////////////
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT_TASK_STACK_SIZE (1024*1024)

//////////////////////////////
// 23.3 Configuration table //
//////////////////////////////
#define CONFIGURE_CONFDEFS_DEBUG

// Parameters set to improve SD performance
#define CONFIGURE_BDBUF_MAX_READ_AHEAD_BLOCKS                2
#define CONFIGURE_BDBUF_MAX_WRITE_BLOCKS                     2
#define CONFIGURE_BDBUF_BUFFER_MAX_SIZE             (32 * 1024)
#define CONFIGURE_BDBUF_CACHE_MEMORY_SIZE         (1024 * 1024)

/////////////////////////////////////////////////////////////
// Enable to print information about the work area and heap//
/////////////////////////////////////////////////////////////
//#define BSP_GET_WORK_AREA_DEBUG

#include <rtems/confdefs.h>

#endif
