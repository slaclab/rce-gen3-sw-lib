/**
* @file      iperf.cc
*
* @brief     An RTEMS application for running iperf.
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      November 26, 2013 -- Created
*
* $Revision: 2921 $
*
* @verbatim                    Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#include <rtems.h>

#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <rtems/shell.h>

#include "datCode.hh"
#include DAT_PUBLIC( benchmark,     iperf205,    iperf.hh )
#include DAT_PUBLIC( test,          lwip,        init.hh )
#include DAT_PUBLIC( tool,          time,        Time.hh )
#include DAT_PUBLIC( tool,          cpu,         cpu.h)
#include DAT_PUBLIC( tool,          debug,       init.hh)
#include DAT_PUBLIC( service,       cmb,         init.hh)

extern "C" void* POSIX_Init(void* argument);

static pthread_t tid;

extern "C" void* POSIX_Init(void* ignored)
{
  uint64_t t0 = tool::time::lticks();

  service::cmb::initialize();

  TOOL_CPU_initialize();

  tool::debug::initialize();

  test::lwip::initialize();

  rtems_shell_init("CSHL",                          /* task_name */
                   RTEMS_MINIMUM_STACK_SIZE * 4,    /* task_stacksize */
                   100,                             /* task_priority */
                   "/dev/console",                  /* devname */
                   true,                            /* forever */
                   false,                           /* wait */
                   NULL                             /* login */
                   );

  int status = pthread_create(&tid, NULL, iperfThread, NULL);
  if (status != 0)
    printk("%s: pthread_create returned bad status: %08x\n",  __func__, status);

  struct timespec t = tool::time::t2s(tool::time::lticks() - t0);
  printk("iperf: Exiting; Run time: %u.%03u seconds\n",
         t.tv_sec, t.tv_nsec / 1000000);

  return NULL;
}


#define CONFIGURE_MALLOC_STATISTICS
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS (20)
#define CONFIGURE_STACK_CHECKER_ENABLED
#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK
#define CONFIGURE_IMFS_MEMFILE_BYTES_PER_BLOCK (256)
#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM
#define CONFIGURE_FILESYSTEM_IMFS

#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

#define CONFIGURE_MAXIMUM_POSIX_THREADS            10
#define CONFIGURE_MAXIMUM_POSIX_TIMERS              5
#define CONFIGURE_MAXIMUM_POSIX_QUEUED_SIGNALS      5
#define CONFIGURE_MAXIMUM_POSIX_SEMAPHORES         10
#define CONFIGURE_MAXIMUM_POSIX_MUTEXES             5
#define CONFIGURE_MAXIMUM_POSIX_CONDITION_VARIABLES 5

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

#define CONFIGURE_MAXIMUM_TASKS (50)
#define CONFIGURE_MAXIMUM_TIMERS (20)
#define CONFIGURE_MAXIMUM_SEMAPHORES (100)
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES (RCE_MAX_RTEMS_MSG_QUEUES)
#define CONFIGURE_MAXIMUM_PARTITIONS (10)
#define CONFIGURE_MAXIMUM_REGIONS (10)
#define CONFIGURE_MAXIMUM_PORTS (10)
#define CONFIGURE_MAXIMUM_PERIODS (10)
#define CONFIGURE_MAXIMUM_USER_EXTENSIONS (5)

#define CONFIGURE_POSIX_INIT_THREAD_TABLE
#define CONFIGURE_POSIX_INIT_THREAD_STACK_SIZE \
        (RTEMS_MINIMUM_STACK_SIZE * 4)

#define CONFIGURE_SHELL_COMMANDS_INIT
#define CONFIGURE_SHELL_COMMANDS_ALL
#define CONFIGURE_SHELL_COMMANDS_ALL_NETWORKING
#define CONFIGURE_SHELL_MOUNT_NFS

#include <rtems/shellconfig.h>

#define CONFIGURE_INIT
#include <rtems/confdefs.h>
