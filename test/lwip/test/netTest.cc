/*
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 */

#include <rtems.h>
#include <cstring>

#include <bsp.h>

#include "datCode.hh"
#include DAT_PUBLIC( tool,          time,        Time.hh )
#include DAT_PUBLIC( tool,          cpu,         cpu.h)
#include DAT_PUBLIC( tool,          debug,       init.hh)
#include DAT_PUBLIC( test,          lwip,        init.hh)

extern "C" rtems_task Init(rtems_task_argument argument);

#include <rtems/error.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <rtems/shell.h>


#ifdef __cplusplus
extern "C" {
#endif

  //#include "httpd.h"
  //#include "udpecho.h"
  //#include "tcpecho.h"
#include "shell.h"

#include "lwip/stats.h"

#ifdef __cplusplus
}
#endif


namespace {

/*
 * Thread-safe output routines
 */
static rtems_id printMutex;
static void printSafe(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  rtems_semaphore_obtain(printMutex, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
  vprintf(fmt, args);
  rtems_semaphore_release(printMutex);
  va_end(args);
}
#define printf printSafe

static int network_stats(int argc, char *argv[])
{
  printk("Network statistics:\n");
  stats_display();
  return 0;
}
} // namespace


/*
 * RTEMS Startup Task
 */
extern "C" rtems_task Init(rtems_task_argument ignored)
{
  rtems_status_code sc;

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

  rtems_shell_add_cmd("netstats", "network",
                      "Display network statistics", network_stats);

  sc = rtems_semaphore_create(rtems_build_name('P','m','t','x'),
              1,
              RTEMS_PRIORITY|RTEMS_BINARY_SEMAPHORE|RTEMS_INHERIT_PRIORITY|
                                  RTEMS_NO_PRIORITY_CEILING|RTEMS_LOCAL,
              0,
              &printMutex);
  if (sc != RTEMS_SUCCESSFUL)
      rtems_panic("Can't create printf mutex:", rtems_status_text(sc));

#if LWIP_TCP
  //tcpecho_init();
  shell_init();
  //httpd_init();
#endif
#if LWIP_UDP
  //udpecho_init();
#endif

  printf("Applications started.\n");

#ifdef MEM_PERF
  mem_perf_init("/tmp/memstats.client");
#endif /* MEM_PERF */

  printk("\n%s: Exiting\n", __func__);

  sleep(999999);
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

#define CONFIGURE_MAXIMUM_TASKS             20
#define CONFIGURE_MAXIMUM_TIMERS            20
#define CONFIGURE_MAXIMUM_SEMAPHORES        20
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES    20
#define CONFIGURE_MAXIMUM_USER_EXTENSIONS    5

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT_TASK_STACK_SIZE    (4 * RTEMS_MINIMUM_STACK_SIZE)

#define CONFIGURE_INIT
#include <rtems/confdefs.h>
