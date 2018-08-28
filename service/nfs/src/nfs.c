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
**	    000 - December 2, 2013
**
**  Revision History:
**	    None.
**
** --
*/



#include <librtemsNfs.h>
#include <rtems/libio.h>

#include "elf/linker.h"
#include "shell/shell.h"

int const lnk_options = LNK_INSTALL;

int lnk_prelude(void *prefs, void *elfHdr) 
  { 
  /* register the nfs driver */
  return rtems_filesystem_register(RTEMS_FILESYSTEM_TYPE_NFS, rtems_nfs_initialize);
  }
