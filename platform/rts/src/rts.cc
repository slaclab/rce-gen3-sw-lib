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
**	    000 - January 23, 2014
**
**  Revision History:
**	    None.
**
** --
*/

#include "elf/linker.h"
#include "exception/terminate.hh"

#include "system/statusCode.h"

using namespace tool::exception;

int lnk_options __attribute__((visibility("default"))) =  LNK_INSTALL;

#if defined(__cplusplus)
extern "C" {
#endif

int lnk_prelude(void *prefs, void *elfHdr) 
  {
  int status = STS_K_SUCCESS;
  
  /* initialize custom C++ exception handling */
  installTerminateHandler();
  
  return status;
  }

#if defined(__cplusplus)
} // extern "C"
#endif
