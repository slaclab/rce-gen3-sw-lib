// See the RTEMS C USer's Guide Chanper 23, "Configuring a System"
// Modified for RTEMS 4.10.0.

#include <rtems.h>

// If you use uint32_t et al. in this file.
#include <stdint.h>

// If you use any C library functions in this file.
#include <stdlib.h>

// Cause <rtrems/confdefs.h> to create the configuration data tables
// using the macros defined below as input. Otherwise it will just
// declare extern references to the tables.
#define CONFIGURE_INIT

/////////////////////////////
// 23.2.1: Library support //
/////////////////////////////

// Keep extra information about malloc, free, and related functions.
// If you use the shell then this information can be viewed using the
// "malloc" shell command.
// #define CONFIGURE_MALLOC_STATISTICS

// Make malloc() "dirty" any memory it allocates with some constant
// unlikely to be a valid initial value. Used to catch the use of
// uninitialized variables.
// #define CONFIGURE_MALLOC_DIRTY

// If the C heap is expandable and the BSP supplies an sbrk() function
// that malloc(), etc., can use when heap space runs out.
// #define CONFIGURE_MALLOC_BSP_SUPPORTS_SBRK

// The number of file descriptors than can be open at the same time.
// The default is 3 to allow stdin, stdout and stderr.
// Libio requires semaphores numbering 1 + (max no. of file descriptors).
// #define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS (3)

// If you know you won't need the POSIX termios TTY control functions.
// #define CONFIGURE_TERMIOS_DISABLED

// The number of termios ports you can have in use at the same time.
// By default this is 1 to allow a console TTY port for use with
// stdin/out/err.  Termios needs semaphores numbering 1 + 4*(max
// no. of termios ports).
// #define CONFIGURE_NUMBER_OF_TERMIOS_PORTS (1)

// The limit on the number of pseudo-ttys open at the same time. Default:0.
// You'll need some for telnetd if you use the RTEMS shell.
// #define CONFIGURE_MAXIMUM_PTYS (0)

// Define this if the application builds its own mount table.
// #define CONFIGURE_HAS_OWN_MOUNT_TABLE

// Define this if you want to disable all filesystem support.
// #define CONFIGURE_APPLICATION_DISABLE_FILESYSTEM

// Use any combination of the following to get the corresponding
// filesystem support if you haven't disables filesystem support. The
// mini-IMFS is a small IMFS that allows only device nodes and
// directories (to organize the device nodes). The DEVFS is a really
// tiny filesystem that allows only device nodes. RFS is the RTEMS
// filesystem, an alternative to DOSFS for hard disks. Hard-disk
// filesystems need libblock.
//
// Use any combination of these or ...
// #define CONFIGURE_FILESYSTEM_MINIIMFS
// #define CONFIGURE_FILESYSTEM_IMFS
// #define CONFIGURE_FILESYSTEM_DEVFS
// #define CONFIGURE_FILESYSTEM_TFTPFS
// #define CONFIGURE_FILESYSTEM_FTPFS
// #define CONFIGURE_FILESYSTEM_NFS
// #define CONFIGURE_FILESYSTEM_DOSFS
// #define CONFIGURE_FILESYSTEM_RFS
// ... use this to get them all.
// #define CONFIGURE_FILESYSTEM_ALL

// Define at most one of the following if you don't want the full IMFS
// to be the base filesystem. If you use either of these options then
// all support for filesystems other than the base will be
// disabled. That's because neither mini-IMFS nor DEVFS allow
// mounting.
// #define CONFIGURE_USE_MINIIMFS_AS_BASE_FILESYSTEM
// #define CONFIGURE_USE_DEVFS_AS_BASE_FILESYSTEM

// If you want automatic stack bounds checking, which adds some
// overhead to task creation and context switching. By default there
// is no checking. If you use a shell then the "stackuse" command will
// show the maximum stack space used if this option is defined.
// #define CONFIGURE_STACK_CHECKER_ENABLED

// If the application needs the block device library with its buffer
// management. You'll need this with IMFS, disk filesystems and
// network filesystems.
// #define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK

// The number of bytes per block for in-memory files. There is a fixed
// blocksize-dependent limit on the number of blocks per file so this
// determines the maximum file size.  The default is 128.
// From <rtems/imfs.h>:
//    max_filesize with blocks of   16 is         1,328
//    max_filesize with blocks of   32 is        18,656
//    max_filesize with blocks of   64 is       279,488
//    max_filesize with blocks of  128 is     4,329,344
//    max_filesize with blocks of  256 is    68,173,568
//    max_filesize with blocks of  512 is 1,082,195,456
// #define CONFIGURE_IMFS_MEMFILE_BYTES_PER_BLOCK (128)


//////////////////////////////////////
// 23.2.2: Basic system information //
//////////////////////////////////////

// If the application builds its own configuration table, which has to
// have the same name as the standard RTEMS tables and must be
// extern. You would still define CONFIGURE_INIT since <confdefs.h>
// will need initialize some global variables.
// #define CONFIGURE_HAS_OWN_CONFIGURATION_TABLE

// The starting address of the RTEMS workspace in RAM. By default this
// is NULL which causes the BSP to decided where best to place the
// work area.
// #define CONFIGURE_EXECUTIVE_RAM_WORK_AREA (NULL)

// If you want to use a single memory pool for both the heap and the
// RTEMS workspace.
// #define CONFIGURE_UNIFIED_WORK_AREAS

// If you know how big you want the RTEMS work area to be then you can
// bypass usual calculation by supplying the size here.
// #define CONFIGURE_EXECUTIVE_RAM_SIZE (0)

// The number of microseconds per system clock tick. By default this
// is 10,000.
// #define CONFIGURE_MICROSECONDS_PER_TICK (10000)

// When using time-sliced scheduling for tasks this is the maximum
// number of system clock ticks a task may run each time it gets
// the CPU. By default this is 50.
// #define CONFIGURE_TICKS_PER_TIMESLICE (50)

// The highest priority number (lowest priority) in the system. The
// priority number given to the system idle task. Must be one less
// than a power of two between 4 and 256 inclusive, i.e., one of 3, 7,
// 31, 63, 127 or 255. A smaller set of possible priorities saves
// a little RTEMS work space. By default this is 255.
// #define CONFIGURE_MAXIMUM_PRIORITY (255)

// The stack size (bytes) RTEMS will allocate for a task/thread if you
// ask for the minimum.  By default this is set to the minimum size
// recommended for the kind of CPU you're using.
// #define CONFIGURE_MINIMUM_STACK_SIZE (0)

// The size in bytes of the stack used for ISRs (when they use their
// own stack).  For some systems the ISR stack's size is fixed; this
// just controls how much memory is set aside for it. The default is
// the configured minimum task stack size.
// #define CONFIGURE_INTERRUPT_STACK_SIZE (0)

// A pointer-to-function value for the user routine that allocates
// task stacks.  The default is NULL which means that stacks will be
// allocated from the RTEMS workspace. The allocator's prototype is
// void *(*)( uint32_t );
// NOTE: So far I have not succeeded in using malloc() to allocate
// task stack space. When I try that the system goes into an
// endless recursion of exceptions starting from the Init task.
// extern "C" void *rce_stackalloc(uint32_t nbytes);
// void *rce_stackalloc(uint32_t nbytes) {return malloc(nbytes);}
// #define CONFIGURE_TASK_STACK_ALLOCATOR (rce_stackalloc)

// A pointer-to-function value for the user routine that deallocates
// task stacks.  The default is NULL. If I were you I'd make sure
// to define a deallocator whenever you define an allocator.
// The deallocator's prototype is void (*)(void*).
// NOTE: See the note above about malloc().
// extern "C" void rce_stackfree(void *base);
// void rce_stackfree(void *base) {free(base);}
// #define CONFIGURE_TASK_STACK_DEALLOCATOR (rce_stackfree)

// TRUE or FALSE: Do you want the RTEMS workspace and the C heap to be
// cleared at startup?  The default is FALSE unless it's overridden by the
// BSP.
// #define CONFIGURE_ZERO_WORKSPACE_AUTOMATICALLY (FALSE)

// The total number of bytes needed for all message queues no matter
// which APIs were used to create them. The default is zero. Should be
// defined in terms of calls to
//       CONFIGURE_MESSAGE_BUFFERS_FOR_QUEUE(nmsgs, msgsize)
// For example if you will have two queues each with a capacity of
// 20 4-byte messages and three queues each with a capacity of 10
// 100-byte messages then
// #define CONFIGURE_MESSAGE_BUFFER_MEMORY \
//   (  2 * CONFIGURE_MESSAGE_BUFFERS_FOR_QUEUE(20,4) \
//    + 3 * CONFIGURE_MESSAGE_BUFFERS_FOR_QUEUE(10,100))

// The number of kilobytes that should be added to the workspace size
// calculated by <confdefs.h>. The default is zero.
// #define CONFIGURE_MEMORY_OVERHEAD (0)

// The number of bytes to add to the task stack space allocation
// calculated by <confdefs.h> (assuming you haven't set
// CONFIGURE_TASK_STACK_ALLOCATOR). The default is zero.  <confdefs.h>
// calculates something like CONFIGURE_MAXIMUM_TASKS*(min. stack
// size).
// #define CONFIGURE_EXTRA_TASK_STACKS (0)

/////////////////////////////////////
// 23.2.3: Idle task configuration //
/////////////////////////////////////

// A pointer-to-function value for the user-supplied idle task
// body. The default is NULL which means to use the idle task body
// supplied by RTEMS or the BSP.
// #define CONFIGURE_IDLE_TASK_BODY (NULL)

// The size in bytes of the stack for the idle task. The default is
// the configured minimum stack size for ordinary tasks.
// #define CONFIGURE_IDLE_TASK_STACK_SIZE (0)

// Define this if you don't want to specify a separate Init task but have
// the idle task body do the initialization before it starts executing
// the idling loop. The default is to require an Init task. There
// are severe restrictions on what the idle task initialization code
// can do, see the C USer's Guide.
// #define CONFIGURE_IDLE_TASK_INITIALIZES_APPLICATION

/////////////////////////////////
// 23.2.4: Device driver table //
/////////////////////////////////

// Note that network drivers don't get entries in this table; they're
// in a class of their own (as they are in Linux).

// If the application will supply its own device driver table.
// The default is that <confdef.h> will supply it.
// #define CONFIGURE_HAS_OWN_DEVICE_DRIVER_TABLE

// The maximum number of device drivers that will be used in this
// RTEMS node.  The default is the number you explicitly turn on in
// this file.
// #define CONFIGURE_MAXIMUM_DRIVERS (11)

// The maximum number of devices that can be registered at one time.
// The default is twenty.
// #define CONFIGURE_MAXIMUM_DEVICES (20)

// If the application needs /dev/console. The default is that it isn't
// defined.
// #define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

// If the application needs (or doesn't need) to have a regular system
// clock tick that causes rtems_clock_tick() to be called.  There is
// no default, exactly, you must define one and only one of these two
// macros.
// #define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
// #define CONFIGURE_APPLICATION_DOES_NOT_NEED_CLOCK_DRIVER

// If the application needs to use interval timers. By default this
// isn't defined. Interval timing requires the clock driver.
// #define CONFIGURE_APPLICATION_NEEDS_TIMER_DRIVER

// If you need to make sure that the last entry in the driver table is
// the null driver entry. This is not the entry for /dev/null but is
// an entry all of whose function pointers are NULL. Normally this
// entry is generated only if needed to make the driver table
// non-empty.  The driver for /dev/null is configured using
// CONFIGURE_APPLICATION_NEEDS_STUB_DRIVER.
// #define CONFIGURE_APPLICATION_NEEDS_NULL_DRIVER

// If your platform has watchdog hardware
// #define CONFIGURE_APPLICATION_NEEDS_WATCHDOG_DRIVER

// If your platform has real-time clock hardware
// #define CONFIGURE_APPLICATION_NEEDS_RTC_DRIVER

// If your BSP defines a frame buffer driver (ours don't; no display!)
// #define CONFIGURE_APPLICATION_NEEDS_FRAME_BUFFER_DRIVER

// If you need a driver for IDE devices like a CD-ROM, etc. Using this
// turns on the ATA driver as well.
// #define CONFIGURE_APPLICATION_NEEDS_IDE_DRIVER

// If you need the ATA driver for disk-like devices.
// #define CONFIGURE_APPLICATION_NEEDS_ATA_DRIVER

// If you need a do-nothing device driver that always succeeds, .i.e.,
// /dev/null. By default this driver is not included.
// #define CONFIGURE_APPLICATION_NEEDS_STUB_DRIVER

// If the BSP defines some drivers set this macro to the
// comma-separated list of driver table entries. They will be inserted
// at the front of the driver table and be initialized before any
// other drivers. By default this list is empty.
// #define CONFIGURE_BSP_PREREQUISITE_DRIVERS

// If the application defines some drivers set this macro to the
// comma-separated list of driver table entries. They will be at the
// front of the driver table right after the BSP drivers and be
// initialized before any other drivers except BSP drivers. By default
// this list is empty.
// #define CONFIGURE_APPLICATION_PREREQUISITE_DRIVERS

// If the application defines some drivers that needn't be initialized
// early set this macro to the comma-separated list of driver table
// entries.  These will be inserted at the end of the driver table
// just before the null driver entry (if any). By default this list is
// empty.
// #define CONFIGURE_APPLICATION_EXTRA_DRIVERS


//////////////////////////////
// 23.2.5:  Multiprocessing //
//////////////////////////////
// Skipped. Not used for RCE work.


/////////////////////////
// 23.2.6: Classic API //
/////////////////////////

// Limits for the numbers of RTEMS objects such as tasks, semaphores,
// message queues, etc., must be between 0 and 32,767 inclusive. The
// rest of the word is reserved for flag bits.

// The limit for the number of a given type of RTEMS object may be
// given as rtems_resource_unlimited(N) instead of just N. This will
// cause RTEMS to make an initial allocation of N and add an
// additional N whenever needed, space in the RTEMS workspace
// permitting. The size of the workspace is calculated using only the
// initial allocations, but most applications won't need all the
// initial allocation for each kind of object. This way unused space
// for one kind of object may be used for another. The macro
// rtems_resource_unlimited() uses one of the flag bits reserved in
// each object count limit.
// NOTE: The RCE BSP and core code won't work properly if you use this
// feature for the number of tasks or the number of user extensions.
// They don't mask off the flag bit in the limit.

// Max number of tasks. The default is zero.
// #define CONFIGURE_MAXIMUM_TASKS (0)

// Will tasks have notepads (a limited kind of task-specific storage)?
// By default they will.
// #define CONFIGURE_DISABLE_CLASSIC_NOTEPADS

// Max interval timers. The default is zero.
// #define CONFIGURE_MAXIMUM_TIMERS (0)

// Max semaphores needed in addition to those needed by RTEMS. The default is zero.
// See CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS.
// See CONFIGURE_NUMBER_OF_TERMIOS_PORTS.
// #define CONFIGURE_MAXIMUM_SEMAPHORES (0)

// Max message queues. The default is zero.
// #define CONFIGURE_MAXIMUM_MESSAGE_QUEUES  (0)

// Max partitions (heaps of fixed-size blocks). The default is zero.
// #define CONFIGURE_MAXIMUM_PARTITIONS (0)

// Max regions (heaps of variable-sized blocks). The default is zero.
// #define CONFIGURE_MAXIMUM_REGIONS (0)

// Max ports (used for dual-ported memories). The default is zero.
// #define CONFIGURE_MAXIMUM_PORTS (0)

// Max periods (used for scheduling tasks that must run a regular
// intervals). The default is zero.
// #define CONFIGURE_MAXIMUM_PERIODS (0)

// Max user extensions. The default is zero.
// #define CONFIGURE_MAXIMUM_USER_EXTENSIONS (0)


//////////////////////////////////////////
// 23.2.7: Classic API Init-tasks table //
//////////////////////////////////////////

// The init-task is used to set up the user application. It's created
// and started after the BSP's initialization code has run. Apparently
// you have a choice of API for which you wish to construct an
// init-tasks table. That is to say you decide which thread API is
// used to create the init-task and you supply the arguments needed by
// the appropriate function(s).  Here we define the values in the
// "classic" API table which supplies the arguments to
// rtems_task_create() and rtems_task_start(). You can decide to
// short-circuit the standard table-building and supply your own table
// instead, although each part of the standard table is separately
// configurable using the macros described below. If you need more
// than one init-task then you'll have to build your own table (which
// must be named Initialization_tasks).
//
// If you use the standard "classic" table with all-default values
// then RTEMS will create one non-preemptable task named 'UI1 '
// executing function Init() with argument 0, the minimum configured
// stack size, priority number 1 (highest priority) and attributes
// RTEMS_DEFAULT_ATTRIBUTES.

// Define this to choose the RTEMS Classic API for init-task creation.
// Not defined by default since the user must make an explicit choice
// of API.
// #define CONFIGURE_RTEMS_INIT_TASKS_TABLE

// If you are supplying your own table. The default is to build the
// standard table.
// # define CONFIGURE_HAS_OWN_INIT_TASK_TABLE

// Overrides the default init-task name,
// #define CONFIGURE_INIT_TASK_NAME rtems_build_name('U', 'I', '1', ' ')

// Overrides the default init-task stack size.
// #define CONFIGURE_INIT_TASK_STACK_SIZE (configured_minimum_stack_size)

// Overrides the default init-task priority.
// #define CONFIGURE_INIT_TASK_PRIORITY (1)

// Overrides the default init-task attibutes.
// #define CONFIGURE_INIT_TASK_ATTRIBUTES (RTEMS_DEFAULT_ATTRIBUTES)

// Overrides the default init-task entry point (pointer-to-function).
// #define CONFIGURE_INIT_TASK_ENTRY_POINT (Init)

// Overrides the default init-task sheduling modes.
// # define CONFIGURE_INIT_TASK_INITIAL_MODES (RTEMS_NO_PREEMPT)

// Overrides the single 32-bit argument that rtems_task_start will
// pass to the task entry point.
// #define CONFIGURE_INIT_TASK_ARGUMENTS (0)


///////////////////////
// 23.2.8: POSIX API //
///////////////////////
// Skipped. POSIX threading is not used for the RCE. We may still use
// other POSIX functions supplied by newlib.


//////////////////////////////////////////
// 23.2.9: POSIX API Init-threads table //
//////////////////////////////////////////
// Skipped.


////////////////////////
// 23.2.10: ITRON API //
////////////////////////
// Skipped. The ITRON API is not used for the RCE.


/////////////////////////////////////////
// 23.2.11: ITRON API Init-tasks table //
/////////////////////////////////////////
// Skipped.


///////////////////////////////////
// 23.2.12: Ada task API (GNATS) //
///////////////////////////////////
// Skipped. The GNATS API is not used for the RCE.


//////////////////////////////
// 23.3 Configuration table //
//////////////////////////////

// Define this if you want the table named Configuration_Memory_Debug
// to be defined and global (e.g., inspectable using GDB). See
// confdefs.h for the table's structure.
// #define CONFIGURE_CONFDEFS_DEBUG


//////////////////////////////////
// Standard RTEMS configuration //
//////////////////////////////////

// Declare the function to be run by the user initialization task,
// RTEMS Classic version.  Must have C linkage.
//extern "C" rtems_task Init(rtems_task_argument);

// If CONFIGURE_INIT is defined then create the configuration tables
// using the other macro definitions as input. From those definitions
// <confdefs.h> will estimate the space needed for the RTEMS executive
// workspace. If CONFIGURE_INIT is not defined then <confdefs.h> will
// generate external references to the various configuration
// variables.
#include <rtems/confdefs.h>


/////////////////
// RTEMS shell //
/////////////////
// The shell adds its own set of configuration tables. The sets of
// commands and aliases defined here are the INITIAL set that may be
// configured statically; more can be added later (and we do that for
// the RCE-specific commands).  See the RTEMS Shell User's Guide
// chapter 1.2.

// Include all standard shell commands that:
// (1) Are not aliases for other commands.
// (2) Do not REQUIRE networking.
// (3) Are not user-defined.
// #define CONFIGURE_SHELL_COMMANDS_ALL

// Configure all standard command aliases that have not been
// explicitly excluded. An alias is defined using a data structure
// different from the one used to define a regular command.
// #define CONFIGURE_SHELL_COMMANDS_INIT

// Include the networking commands ifconfig, netstats and route.
// #define CONFIGURE_SHELL_COMMANDS_ALL_NETWORKING

// If you're going to configure user aliases or commands in this file
// then at this point you should have extern declarations for the
// definition structures you're going to refer to.

// The comma-separated list of user command alias table entries.
// Each entry is the address of an alias definition structure (rtems_shell_alias_t).
// The default is not to define this macro.
// #define CONFIGURE_SHELL_USER_ALIASES

// The comma-separated list of user command table entries.
// Each entry is the address of a command definition structure (rtems_shell_cmd_t).
// The default is not to define this macro.
// #define CONFIGURE_SHELL_USER_COMMANDS

// The standard aliases and commands may be included or excluded
// individually, overriding the more general selection macros.

// #define CONFIGURE_SHELL_COMMAND_DIR
// #define CONFIGURE_SHELL_NO_COMMAND_DIR

// #define CONFIGURE_SHELL_COMMAND_CD
// #define CONFIGURE_SHELL_NO_COMMAND_CD

// #define CONFIGURE_SHELL_COMMAND_EXIT
// #define CONFIGURE_SHELL_NO_COMMAND_EXIT

// #define CONFIGURE_SHELL_COMMAND_JOEL
// #define CONFIGURE_SHELL_NO_COMMAND_JOEL

// #define CONFIGURE_SHELL_COMMAND_DATE
// #define CONFIGURE_SHELL_NO_COMMAND_DATE

// #define CONFIGURE_SHELL_COMMAND_ECHO
// #define CONFIGURE_SHELL_NO_COMMAND_ECHO

// #define CONFIGURE_SHELL_COMMAND_SLEEP
// #define CONFIGURE_SHELL_NO_COMMAND_SLEEP

// #define CONFIGURE_SHELL_COMMAND_ID
// #define CONFIGURE_SHELL_NO_COMMAND_ID

// #define CONFIGURE_SHELL_COMMAND_TTY
// #define CONFIGURE_SHELL_NO_COMMAND_TTY

// #define CONFIGURE_SHELL_COMMAND_WHOAMI
// #define CONFIGURE_SHELL_NO_COMMAND_WHOAMI

// #define CONFIGURE_SHELL_COMMAND_LOGOFF
// #define CONFIGURE_SHELL_NO_COMMAND_LOGOFF

// #define CONFIGURE_SHELL_COMMAND_SETENV
// #define CONFIGURE_SHELL_NO_COMMAND_SETENV

// #define CONFIGURE_SHELL_COMMAND_UNSETENV
// #define CONFIGURE_SHELL_NO_COMMAND_CRLENV

// #define CONFIGURE_SHELL_COMMAND_GETENV
// #define CONFIGURE_SHELL_NO_COMMAND_GETENV

// #define CONFIGURE_SHELL_COMMAND_MDUMP
// #define CONFIGURE_SHELL_NO_COMMAND_MDUMP

// #define CONFIGURE_SHELL_COMMAND_WDUMP
// #define CONFIGURE_SHELL_NO_COMMAND_WDUMP

// #define CONFIGURE_SHELL_COMMAND_LDUMP
// #define CONFIGURE_SHELL_NO_COMMAND_LDUMP

// #define CONFIGURE_SHELL_COMMAND_MEDIT
// #define CONFIGURE_SHELL_NO_COMMAND_MEDIT

// #define CONFIGURE_SHELL_COMMAND_MFILL
// #define CONFIGURE_SHELL_NO_COMMAND_MFILL

// #define CONFIGURE_SHELL_COMMAND_MMOVE
// #define CONFIGURE_SHELL_NO_COMMAND_MMOVE

// #define CONFIGURE_SHELL_COMMAND_CP
// #define CONFIGURE_SHELL_NO_COMMAND_CP

// #define CONFIGURE_SHELL_COMMAND_PWD
// #define CONFIGURE_SHELL_NO_COMMAND_PWD

// #define CONFIGURE_SHELL_COMMAND_LS
// #define CONFIGURE_SHELL_NO_COMMAND_LS

// #define CONFIGURE_SHELL_COMMAND_CHDIR
// #define CONFIGURE_SHELL_NO_COMMAND_CHDIR

// #define CONFIGURE_SHELL_COMMAND_MKDIR
// #define CONFIGURE_SHELL_NO_COMMAND_MKDIR

// #define CONFIGURE_SHELL_COMMAND_RMDIR
// #define CONFIGURE_SHELL_NO_COMMAND_RMDIR

// #define CONFIGURE_SHELL_COMMAND_CHROOT
// #define CONFIGURE_SHELL_NO_COMMAND_CHROOT

// #define CONFIGURE_SHELL_COMMAND_CHMOD
// #define CONFIGURE_SHELL_NO_COMMAND_CHMOD

// #define CONFIGURE_SHELL_COMMAND_CAT
// #define CONFIGURE_SHELL_NO_COMMAND_CAT

// #define CONFIGURE_SHELL_COMMAND_MKRFS
// #define CONFIGURE_SHELL_NO_COMMAND_MKRFS

// #define CONFIGURE_SHELL_COMMAND_MSDOSFMT
// #define CONFIGURE_SHELL_NO_COMMAND_MSDOSFMT

// #define CONFIGURE_SHELL_COMMAND_MV
// #define CONFIGURE_SHELL_NO_COMMAND_MV

// #define CONFIGURE_SHELL_COMMAND_RM
// #define CONFIGURE_SHELL_NO_COMMAND_RM

// #define CONFIGURE_SHELL_COMMAND_LN
// #define CONFIGURE_SHELL_NO_COMMAND_LN

// #define CONFIGURE_SHELL_COMMAND_MKNOD
// #define CONFIGURE_SHELL_NO_COMMAND_MKNOD

// #define CONFIGURE_SHELL_COMMAND_UMASK
// #define CONFIGURE_SHELL_NO_COMMAND_UMASK

// #define CONFIGURE_SHELL_COMMAND_MOUNT
// #define CONFIGURE_SHELL_NO_COMMAND_MOUNT

// #define CONFIGURE_SHELL_COMMAND_UNMOUNT
// #define CONFIGURE_SHELL_NO_COMMAND_UNMOUNT

// #define CONFIGURE_SHELL_COMMAND_BLKSYNC
// #define CONFIGURE_SHELL_NO_COMMAND_BLKSYNC

// #define CONFIGURE_SHELL_COMMAND_FDISK
// #define CONFIGURE_SHELL_NO_COMMAND_FDISK

// #define CONFIGURE_SHELL_COMMAND_DD
// #define CONFIGURE_SHELL_NO_COMMAND_DD

// #define CONFIGURE_SHELL_COMMAND_HEXDUMP
// #define CONFIGURE_SHELL_NO_COMMAND_HEXDUMP

// #define CONFIGURE_SHELL_COMMAND_DEBUGRFS
// #define CONFIGURE_SHELL_NO_COMMAND_DEBUGRFS

// #define CONFIGURE_SHELL_COMMAND_HALT
// #define CONFIGURE_SHELL_NO_COMMAND_HALT

// #define CONFIGURE_SHELL_COMMAND_CPUUSE
// #define CONFIGURE_SHELL_NO_COMMAND_CPUUSE

// #define CONFIGURE_SHELL_COMMAND_STACKUSE
// #define CONFIGURE_SHELL_NO_COMMAND_STACKUSE

// #define CONFIGURE_SHELL_COMMAND_PERIODUSE
// #define CONFIGURE_SHELL_NO_COMMAND_PERIODUSE

// #define CONFIGURE_SHELL_COMMAND_WKSPACE_INFO
// #define CONFIGURE_SHELL_NO_COMMAND_WKSPACE_INFO

// #define CONFIGURE_SHELL_COMMAND_MALLOC_INFO
// #define CONFIGURE_SHELL_NO_COMMAND_MALLOC_INFO

// #define CONFIGURE_SHELL_COMMAND_IFCONFIG
// #define CONFIGURE_SHELL_NO_COMMAND_IFCONFIG

// #define CONFIGURE_SHELL_COMMAND_IFCONFIG
// #define CONFIGURE_SHELL_NO_COMMAND_IFCONFIG

// #define CONFIGURE_SHELL_COMMAND_ROUTE
// #define CONFIGURE_SHELL_NO_COMMAND_ROUTE

// #define CONFIGURE_SHELL_COMMAND_NETSTATS
// #define CONFIGURE_SHELL_NO_COMMAND_NETSTATS

// #define CONFIGURE_SHELL_COMMAND_RTC
// #define CONFIGURE_SHELL_NO_COMMAND_RTC

// #define CONFIGURE_SHELL_COMMAND_MOUNT
// #define CONFIGURE_SHELL_NO_COMMAND_MOUNT

// #define CONFIGURE_SHELL_COMMAND_UNMOUNT
// #define CONFIGURE_SHELL_NO_COMMAND_UNMOUNT

#include <rtems/shellconfig.h>
