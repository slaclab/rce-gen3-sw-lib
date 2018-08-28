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

  return space;
  }

