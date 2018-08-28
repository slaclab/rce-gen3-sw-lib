
/*
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
**	    000 - April 06, 2011
**
**  Revision History:
**	    None.
**
** --
*/
 
#ifndef SERVICE_DSL_LOCATION
#define SERVICE_DSL_LOCATION



#include "conversion/BSWP.h"
#include "conversion/impl/BSWP.ih"
#include "net/Endpoint.hh"

namespace service {
namespace dsl  {

class Location {
public:
  static uint64_t hash(const char* string); 
public:
  Location(const Location&, uint32_t fixup, uint32_t attributes=0);  
  Location(const net::Endpoint&, uint32_t attributes=0);                                                            
  Location(uint32_t addr, uint32_t attributes=0); 
  Location(uint32_t addr, uint16_t port, uint32_t attributes=0);   
  Location(uint64_t mac, uint32_t attributes=0); 
  Location(uint32_t addr, uint64_t mac, uint32_t attributes=0); 
  Location(uint32_t addr, uint16_t port, uint64_t mac, uint32_t attributes=0);  
public:
 ~Location() {}
public: 
  uint64_t      layer2;
  net::Endpoint layer3;
public:
  uint32_t attributes() const {return BSWP__swap32b(_attributes);} 
public:
  void* operator new(size_t size, uint8_t* buffer) {return (void*)buffer;}
public:
  void dump(const char* tag, int offset = 0) const;  
private:
  uint32_t _attributes;  
};

}}

#endif

