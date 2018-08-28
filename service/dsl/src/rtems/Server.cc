/*
** ++
**  Package:
**	
**
**  Abstract:
**      
**
**  Author:
**      Michael Huffer, SLAC (mehsys@slac.stanford.edu)
**
**  Creation Date:
**	000 - January 09, 2007
**
**  Revision History:
**	None.
**
** --
*/

#include "dsl/Server.hh"
#include "svt/Svt.h"
#include "elf/linker.h"
#include "kvt/Kvt.h"

using namespace service::dsl;


/*
** ++
**
**
** --
*/

Space* Server::_lookup(const char *service)  
  {
  Space *space = (Space*)KvtLookup(Location::hash(service),_kvtable);

  if(!space)
    {
    const char* image;
    Ldr_elf*    elf;
    uint32_t    error;
 
    /* service not in kvt, so lookup service symbol in svt */
    image = (const char*)Svt_Translate(service, SVT_ANY_TABLE);

    error = image ? SVT_SUCCESS: SVT_NOSUCH_SYMBOL;

    if(error)
      {
      return NULL;
      }

    elf = lnk_load(image, NULL, &error, NULL);
    if(!elf)
      {
      return NULL;
      }
    }
  /* service is now loaded, lookup space */
  space = (Space *)KvtLookup(Location::hash(service),_kvtable);

  return space;
  }

