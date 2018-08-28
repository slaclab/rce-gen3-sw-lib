/*
**  Package:
**	
**
**  Abstract:
**      
**
**  Author:
**      Sergio Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	    000 - April 17, 2014
**
**  Revision History:
**	    None.
**
** --
*/

/*
** ++
**
**
** --
*/

#include <rtems.h>
#include <rtems/fsmount.h>
#include <errno.h>
#include "system/fs.h"
#include "svt/Svt.h"
#include "ldr/Ldr.h"
#include "debug/print.h"

const char SCRATCH_MOUNT_POINT[]  = "/mnt/scratch";
//const char USER_NFS_PATH[]         = "<your_server_ip>:<your_nfs_path>";
//const char USER_NAMESPACE[]        = "user";
//const char USER_PATH[]             = "/mnt/nfs/";
//const char USER_SVT[]              = "user:user.svt";
//const char USER_CONFIG[]           = "USER_SYMBOL";

//#define SVT_USER_TABLE_NUM 28
//#define SVT_USER_TABLE (1 << SVT_USER_TABLE_NUM)

static void _define_ns(Ldr_name t, Ldr_path own_path);
static void _mount_nfs(const char* mount, const char* name);
static void _install_svt(Ldr_name name);

/*
** ++
**
**
** --
*/

const char _mount_error1[] = "SD mount point create failure 0x%x errno 0x%x: %s\n";
const char _mount_error2[] = "SD mount failure 0x%x: %s to %s\n";

static void _mount_scratch(const char* name)
  {
  int error = rtems_mkdir(name, S_IRWXU|S_IRWXG|S_IRWXO);

  if(error) dbg_bugcheck(_mount_error1, error, strerror(errno), name);

  error = mount(SD_PARTITION_SCRATCH, name, SD_PARTITION_TYPE_SCRATCH, RTEMS_FILESYSTEM_READ_WRITE, NULL);

  if(error) dbg_bugcheck(_mount_error2, error, SD_PARTITION_SCRATCH, name);
  }

/*
** ++
**
**
** --
*/

static void _mount_nfs(const char* mnt, const char* name)
  {
  int error = rtems_mkdir(name, S_IRWXU|S_IRWXG|S_IRWXO);

  if(error) dbg_bugcheck(_mount_error1, error, strerror(errno), name);

  error = mount(mnt, name, "nfs", RTEMS_FILESYSTEM_READ_WRITE, NULL);

  if(error) dbg_bugcheck(_mount_error2, error, mnt, name);
  }

/*
** ++
**
**
** --
*/

const char _install_error[] = "Cannot install svt: %s error 0x%x\n";

static void _install_svt(Ldr_name name)
  {
//  int error = Svt_Install(SVT_USER_TABLE_NUM, name);

//  if(error) dbg_bugcheck(_install_error, name, error);
  }

/*
** ++
**
**
** --
*/

const char _define_error[] = "Cannot assign namespace: %s with path: %s\n";

static void _define_ns(Ldr_name t, Ldr_path own_path)
  {
  Ldr_path prefix = Ldr_Assign(t, own_path);

  if(!prefix) dbg_bugcheck(_define_error, t, own_path);
  }


/*
** ++
**
**
** --
*/

int lnk_prelude(void *prefs, void *elfHdr) 
  {
  /* mount the scratch partition */
//  _mount_scratch(SCRATCH_MOUNT_POINT);
  
  /* create and mount an user defined nfs directory */
//  _mount_nfs(USER_NFS_PATH,USER_PATH);

  /* assign a user defined namespace */
//  _define_ns(USER_NAMESPACE, USER_PATH);
      
  /* install a user defined SVT */
//  _install_svt(USER_SVT);
  
  /* lookup a symbol in the user defined SVT */
//  const char *user = (const char*)Svt_Translate(USER_CONFIG, SVT_USER_TABLE);
  
//  dbg_printv("User defined SVT symbol %s\n",user);

  return 0;
  }
