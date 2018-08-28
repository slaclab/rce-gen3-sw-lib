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
 


#include "dsl/impl/Proxy.hh"

using namespace service::dsl;

/*
** ++
**
**
** --
*/

void Proxy::_tidIs(uint32_t value, void* request)
{

Message* message = (Message*)request;

message->tidIs(value);

return;
}

/*
** ++
**
**
** --
*/

uint32_t Proxy::_tid(void* packet) const      
{

Message* message = (Message*)packet;

return message->tid();
}
