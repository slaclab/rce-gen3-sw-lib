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
#include <string.h>



#include "kvt/Kvt.h"
#include "net/MacAddress.hh"
#include "dsl/Location.hh"

using namespace service::dsl;

/*
** ++
**
**
** --
*/
 
Location::Location(const Location& clone, uint32_t fixup, uint32_t attributes) :  
 layer2(clone.layer2),
 layer3(clone.layer3, fixup),
 _attributes(BSWP__swap32b(attributes))
 {
 }

/*
** ++
**
**
** --
*/
 
Location::Location(const net::Endpoint& clone, uint32_t attributes) :  
 layer2(0),
 layer3(clone),
 _attributes(BSWP__swap32b(attributes))
 {
 }

/*
** ++
**
**
** --
*/

Location::Location(uint32_t addr, uint32_t attributes) : 
  layer2(0), 
  layer3(addr),
  _attributes(BSWP__swap32b(attributes))
  {
  }

/*
** ++
**
**
** --
*/

Location::Location(uint32_t addr, uint16_t port, uint32_t attributes) :
  layer2(0),   
  layer3(addr, port),
  _attributes(BSWP__swap32b(attributes))
  {
  }

/*
** ++
**
**
** --
*/

Location::Location(uint64_t mac, uint32_t attributes) :
  layer2(mac),    
  layer3(),
  _attributes(BSWP__swap32b(attributes))
  {
  }
  
/*
** ++
**
**
** --
*/

Location::Location(uint32_t addr, uint64_t mac, uint32_t attributes) : 
  layer2(mac), 
  layer3(addr),
  _attributes(BSWP__swap32b(attributes))
  {
  }

/*
** ++
**
**
** --
*/

Location::Location(uint32_t addr, uint16_t port, uint64_t mac, uint32_t attributes) :
  layer2(mac),   
  layer3(addr, port),
  _attributes(BSWP__swap32b(attributes))
  {
  }


/*
** ++
**
**
** --
*/

#define SEED    ((uint32_t)1771875)

uint64_t Location::hash(const char* string)       
 {
 return KvtHash(string);
 }

/*
** ++
**
**
** --
*/

const char* BLANK = "";

const char* LINE_1 = "%*sMapped %s to this location...\n";
const char* LINE_2 = "%*sLayer 3 Port: %s (decimal)\n";
const char* LINE_3 = "%*sLayer 3 Address: %s\n";
const char* LINE_4 = "%*sLayer 2 Address: %s\n";

#define OWN_INDENT 2

void Location::dump(const char* tag, int indent) const
 {

 char str[net::Endpoint::SIZEOF_PSTRING];
 
 printf(LINE_1, indent, BLANK, tag);
 printf(LINE_2, indent + OWN_INDENT, BLANK, layer3.port_p(str)); 
 printf(LINE_3, indent + OWN_INDENT, BLANK, layer3.addr_p(str));
 
 net::MacAddress mac_address(layer2);

 printf(LINE_4, indent + OWN_INDENT, BLANK, mac_address.string());  
 
 return;
 }
