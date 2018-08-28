
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
 
#ifndef SERVICE_DSL_MESSAGE
#define SERVICE_DSL_MESSAGE

#include <inttypes.h>
#include <stddef.h>

#include "net/Socket.hh"
#include "conversion/BSWP.h"
#include "conversion/impl/BSWP.ih"

namespace service {
namespace dsl  {

enum {SERVER_PORT = service::net::Socket::WKS_DSL};

enum {VERSION = 0};

class Message {
public:
  Message(uint64_t magic_cookie, uint32_t tid = 0) : 
   _magic_cookie(BSWP__swap64b(magic_cookie)),
   _version(BSWP__swap32b(VERSION)), 
   _tid(BSWP__swap32b(tid)) {}
public:
 ~Message() {}
public:  
  uint32_t version()      const {return BSWP__swap32b(_version);}  
  uint64_t magic_cookie() const {return BSWP__swap64b(_magic_cookie);}
public:
  uint32_t tid() const {return _tid;}
public:
  void tidIs(uint32_t tid) {_tid = tid;}  
private:
  uint64_t _magic_cookie;
  uint32_t _version;
  uint32_t _tid;
};

}}

#endif

