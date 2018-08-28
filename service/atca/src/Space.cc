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



#include <stdio.h>

#include "conversion/BSWP.h"
#include "conversion/impl/BSWP.ih"

#include "net/Interface.hh"
#include "atca/Space.hh"

#define MAX_ELEMENTS 9

using service::dsl::Location;
using service::dsl::Tag;
using service::atca::Attributes;

using namespace service::atca;

const char* this_name = "DSL_ATCA";

/** @brief Constructor taking an Attributes pointer as an argument
    @param attributes The Attributes of this RCE.
 */
Space::Space(const Attributes* attributes) :
  service::dsl::Space(this_name),
  _attributes(attributes),
  _slot(attributes->cluster()),
  _shelf(Location::hash(attributes->group()))
  {
  }

/** @brief Respond to a Client lookup
    @param tag  The encapsulated packet information.
    @return The location of this RCE, or zero if the match to the tag fails.
 */
const service::dsl::Location*
Space::lookup(const service::dsl::Tag* tag)
  {
  service::net::Interface interface;
  
  /* extract network order tag */
  if(BSWP__swap64b(tag->value.w64[0]) != _shelf) return (Location*)0;
  if(BSWP__swap32b(tag->value.w32[2]) != _slot)  return (Location*)0;
  if(BSWP__swap32b(tag->value.w32[3]) != _attributes->bay())     return (Location*)0;
  if(BSWP__swap32b(tag->value.w32[4]) != _attributes->element()) return (Location*)0;
  
  return new(_result) Location( interface.ipAddr(), interface.hwAddr() << 16);
  }

namespace service {
  namespace atca {

/**
   @class Space
   @brief Space is registered with a DSL server and responds (via lookup())
   when a matching request is sent.

   
   
 */

  }
}
