// -*-Mode: C;-*-
/*!
*
* @brief   Functions for bootstrapping the core
*
* @author  S. Maldonado - (smaldona@slac.stanford.edu)
*
* @date    January 7, 2014 -- Created
*
* $Revision: $
*
* @verbatim
*                               Copyright 2014
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

#include <rtems.h>



#include "startup/init.h"
#include "rtems/default.h"
#include "system/fs.h"
#include "debug/platform/init.h"
#include "task/Task.h"
#include "svt/Svt.h"
#include "ldr/Ldr.h"
#include "elf/linker.h"
#include "debug/print.h"
#include "sd/init.h"

const char SYS_NAMESPACE[]        = "system"; 
const char SYS_MOUNT_POINT[]      = "/mnt/rtems";
const char SYS_PATH[]             = "/mnt/rtems/";
const char SYS_NAME[]             = "system:rtems.so";
const char SYS_STARTUP_SERVICES[] = "INIT_STARTUP_SERVICES";
const char CFG_NAMESPACE[]        = "config"; 
const char CFG_MOUNT_POINT[]      = "/mnt/rtemsapp";
const char CFG_PATH[]             = "/mnt/rtemsapp/config/";
const char SYS_PREFERENCES[]      = "config:sys.svt";
const char APP_PREFERENCES[]      = "config:app.svt";

static void _dbgInit(void);
static void _startSd(void);
static void _fixupRtems(Ldr_name name);
static void _mount_sys(const char* t);
static void _mount_app(const char* t);
static void _define_sys(Ldr_path own_path);
static void _define_cfg(Ldr_path own_path);
static void _installSysSvt(Ldr_name t);
static void _installAppSvt(Ldr_name t);
static void _startup();

// Make sure all debugger breakpoints are cleared before flashing an
// executable, or the instructions at those addresses will be modified
// to be trap instructions which will be included in the flashed
// executable.


// The functions are arranged in the order in which they are called.
//
// boot_card()
//   bsp_start()
//     __wrap_bsp_start()
//       brd_start()
//   bsp_pretasking_hook()
//     __wrap_bsp_pretasking_hook()
//       brd_pretasking_hook()
//   bsp_predriver_hook()
//     __wrap_bsp_predriver_hook()
//       brd_predriver_hook()
//   bsp_postdriver_hook()
//     __wrap_bsp_postdriver_hook()
//       brd_postdriver_hook()
//
// The --wrap linker option arranges for the __wrap_* functions to be called
// instead of the original functions, and renames the original functions to
// __real_*.  It then becomes our responsibility to ensure that the original
// functions are called or replaced as appropriate.
//
// C++ static constructors are not called until the first context
// switch, so none of the code run from these functions can use
// statically allocated objects that have other than do-nothing
// constructors. printk(), vprintk(), and putk() are the only I/O
// routines that should be called.

/** @brief Early hook called by app_bsp_start().

    This function is called by app_start() after the interrupt stack
    has been allocated and internal (but not external) interrupts have
    been enabled. RTEMS and the C/C++ library (especially malloc() and
    new) are not yet available.  The RTEMS workspace has not yet been
    set up.
*/
extern void brd_start()
  {                                                             
  BOOT_STATE(BOOT_BRDSTART);
  }


/** @brief Extra work to be done before tasking is enabled.

    This function is called by app_pretasking_hook(). Most of RTEMS
    and the standard C/C++ library are available, except that I/O
    involving RTEMS device drivers can't be done yet as drivers have
    not yet been initialized.
*/
extern void brd_pretasking_hook()
  {                                                            
  BOOT_STATE(BOOT_PRETASKHOOK);
  }


/** @brief Extra work to be done before device driver initialization.

    This function is called by app_predriver_hook(). The following
    subsystems are available:
    - Basic RTEMS
    - Internal interrupts
    - C/C++ standard library
    - NO external interrupts
    - NO multitasking

*/
extern void brd_predriver_hook()
  {                                                             
  BOOT_STATE(BOOT_PREDRVHOOK);
  }


/** @brief Extra work to be done after device driver initialization.

    This function is called by app_postdriver_hook().
 */
extern void brd_postdriver_hook()
  {                                                             
  BOOT_STATE(BOOT_POSTDRVHOOK);
  }


/** @brief Early hook called by bsp_start().

    This function is called by bsp_start() after the interrupt stack
    has been allocated and internal (but not external) interrupts have
    been enabled. RTEMS and the C/C++ library (especially malloc() and
    new) are not yet available.  The RTEMS workspace has not yet been
    set up.
*/
extern void __real_bsp_start();
extern void __wrap_bsp_start();

extern void __wrap_bsp_start()
  {
  // Call the default bsp_start()
  __real_bsp_start();

  // Do platform-specific board initialization

  // Do platform-specific CPU initialization

  // Do the board-specific start routine
  brd_start();                                                 

  BOOT_STATE(BOOT_WRAPBSPSTART);
  }


/** @brief Wrapper for bsp_pretasking_hook().

    This function calls the original, "real",
    bsp_pretasking_hook(). Most of RTEMS and the standard C/C++
    library are available, except that I/O involving RTEMS device
    drivers can't be done yet as drivers have not yet been
    initialized.
*/
extern void __real_bsp_pretasking_hook();
extern void __wrap_bsp_pretasking_hook();
extern void __wrap_bsp_pretasking_hook()
  {                                                              
  // Call the default bsp_pretasking_hook()
  __real_bsp_pretasking_hook();

  // Do the board-specific pretasking routine
  brd_pretasking_hook();                                       

  BOOT_STATE(BOOT_WRAPPRETASKHOOK);
  }


/** @brief Extra work to be done before device driver initialization.

    This function is called by bsp_predriver_hook(). The following
    subsystems are available:
    - Basic RTEMS
    - Internal interrupts
    - C/C++ standard library
    - NO external interrupts
    - NO multitasking

    After bsp_predriver_hook() exits comes the time when the device
    drivers will be initialized, C++ static constructors will be
    called, external interrupts will be enabled and multitasking will
    be active.
*/
extern void __real_bsp_predriver_hook();
extern void __wrap_bsp_predriver_hook();
extern void __wrap_bsp_predriver_hook()
  {                                                              
  // Call the default bsp_predriver_hook()
  __real_bsp_predriver_hook();

  // Do the board-specific predriver routine
  brd_predriver_hook();
  
  BOOT_STATE(BOOT_WRAPPREDRVHOOK);
  }


/** @brief Extra work to be done after device driver initialization.

    This function is called by bsp_postdriver_hook().
*/
extern void __real_bsp_postdriver_hook();
extern void __wrap_bsp_postdriver_hook();
extern void __wrap_bsp_postdriver_hook()
  {                                                             
  // Call the default bsp_postdriver_hook()
  __real_bsp_postdriver_hook();

  // Do the board-specific predriver routine
  brd_postdriver_hook();                                      

  BOOT_STATE(BOOT_WRAPPOSTDRVHOOK);
  }


/*! The body of the RTEMS Init task, which performs application
 *  initialization after multitasking is enabled (and C++ static
 *  constructors have been run).
 */
 
void mem_rsInitialize(void);

extern rtems_task Init(rtems_task_argument arg)
  {
  _dbgInit();

  _fixupRtems(SYS_NAME);

  _startSd();

  _mount_sys(SYS_MOUNT_POINT);
  
  _mount_app(CFG_MOUNT_POINT);

  _define_sys(SYS_PATH);
  
  _define_cfg(CFG_PATH);

  _installSysSvt(SYS_PREFERENCES);

  _installAppSvt(APP_PREFERENCES);

  // Initialize atomic resource management. We know that the SAS at
  // least will be using it so it can't come any later than this.
  mem_rsInitialize();

  _startup();

  /* the system has successfully completed the boot process! */
  BOOT_STATE(BOOT_SUCCESS);

  rtems_task_delete(RTEMS_SELF); // Our work is done, so we voluntarily delete ourselves...
  }

/*
** ++
**
** Install our task-exit handler.
** 
** --
*/

static void _dbgInit(void)
  {
  dbg_lateInit(); // Install our task-exit handler. Set a flag indicating that tasks are enabled.

  BOOT_STATE(BOOT_DBGINIT);
  }

/*
** ++
**
**
** --
*/

static void _startSd(void)
  {
  sd_initialize();

  BOOT_STATE(BOOT_SDINIT);
  }

/*
** ++
**
** Initialize the "Ldr" facility. Along the way this will validate that the image
** that represents ourselves is indeed consistant as an ELF image with the required name.
** At the successful conclusion this will also correct our OWN memory permissions...
**
** --
*/

const char fixupRtems_error[] = "Ldr_Install_Rtems returned error: 0x%x\n";

static void _fixupRtems(Ldr_name name)
  {
  uint32_t error = Ldr_LoadRtems(name);

  if(error) dbg_bugcheck(fixupRtems_error, error);

  BOOT_STATE(BOOT_RTEMSLDR);   
  }

/*
** ++
**
**
** --
*/

const char _mount_error1[] = "SD mount point create failure 0x%x errno 0x%x: %s\n";
const char _mount_error2[] = "SD mount failure 0x%x: %s to %s\n";

static void _mount_sys(const char* name)
  {
  int error = rtems_mkdir(name, S_IRWXU|S_IRWXG|S_IRWXO);

  if(error) dbg_bugcheck(_mount_error1, error, strerror(errno), name);

  error = mount(SD_PARTITION_RTEMS, name, SD_PARTITION_TYPE_RTEMS, RTEMS_FILESYSTEM_READ_ONLY, NULL);

  if(error) dbg_bugcheck(_mount_error2, error, SD_PARTITION_RTEMS, name);

  BOOT_STATE(BOOT_SYS_SDMOUNT);
  }

/*
** ++
**
**
** --
*/

static void _mount_app(const char* name)
  {
  int error = rtems_mkdir(name, S_IRWXU|S_IRWXG|S_IRWXO);

  if(error) dbg_bugcheck(_mount_error1, error, strerror(errno), name);

  error = mount(SD_PARTITION_RTEMSAPP, name, SD_PARTITION_TYPE_RTEMSAPP, RTEMS_FILESYSTEM_READ_WRITE, NULL);

  if(error) dbg_bugcheck(_mount_error2, error, SD_PARTITION_RTEMSAPP, name);

  BOOT_STATE(BOOT_APP_SDMOUNT);
  }

/*
** ++
**
**
** --
*/

const char _define_error[] = "Cannot assign namespace: %s with path: %s\n";

static void _define_sys(Ldr_path own_path)
  {
  Ldr_path prefix = Ldr_Assign(SYS_NAMESPACE, own_path);

  if(!prefix) dbg_bugcheck(_define_error, SYS_NAMESPACE, own_path);

  BOOT_STATE(BOOT_SYSNAMESPACE);
  }

/*
** ++
**
**
** --
*/

static void _define_cfg(Ldr_path own_path)
  {
  Ldr_path prefix = Ldr_Assign(CFG_NAMESPACE, own_path);

  if(!prefix) dbg_bugcheck(_define_error, CFG_NAMESPACE, own_path);

  BOOT_STATE(BOOT_CFGNAMESPACE);
  }

/*
** ++
**
**
** --
*/

const char _installSysSvt_error[] = "Cannot install sys preference image: %s error 0x%x\n";

static void _installSysSvt(Ldr_name name)
  {
  int error = Svt_Install(SVT_SYS_TABLE_NUM, name);

  if(error) dbg_bugcheck(_installSysSvt_error, name, error);

  BOOT_STATE(BOOT_SYSSVT);
  }

/*
** ++
**
**
** --
*/

const char _installAppSvt_error[] = "Cannot install app preference image: %s error 0x%x\n";

static void _installAppSvt(Ldr_name name)
  {
  int error = Svt_Install(SVT_APP_TABLE_NUM, name);

  if(error) dbg_bugcheck(_installAppSvt_error, name, error);

  BOOT_STATE(BOOT_APPSVT);
  }

/*
** ++
**
**
** --
*/

const char _startupPrefs_error[] = "Cannot load startup preferences: %s\n";
const char _startupSvc_error[]   = "Link/Load/Start of: %s failed with status: 0x%x\n";

static void _startup()
  {
  const char **services;
  const char **svc;
  uint32_t   status;

  /* find the list of services to start */
  services = (const char**)Svt_Translate(SYS_STARTUP_SERVICES, SVT_APP_TABLE);

  if(!services) dbg_bugcheck(_startupPrefs_error, SYS_STARTUP_SERVICES);

  BOOT_STATE(BOOT_STARTPREFS);
  
  /* load services */
  for(svc = services; *svc; svc++)
    {
    Ldr_elf* elf = lnk_load((Ldr_name)*svc, NULL, &status, NULL);
    if(!elf) dbg_bugcheck(_startupSvc_error, *svc, status);
    }

  BOOT_STATE(BOOT_STARTSVC);
  }


