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
 
#ifndef SERVICE_DSL_PROXY
#define SERVICE_DSL_PROXY

#include "csi/Proxy.hh"
#include "udp/Link.hh"
#include "dsl/impl/Request.hh"
#include "dsl/impl/Response.hh"

namespace service {
namespace dsl  {

class Proxy : public tool::csi::Proxy {
public:
  enum {MAX_RETRIES = 4};
  enum {INITIAL_TIMOUT = 200}; // 2 tics = 2048 microseconds
public: 
  Proxy(tool::csi::Link* link) : tool::csi::Proxy(link, INITIAL_TIMOUT, MAX_RETRIES) {}
public:
  virtual ~Proxy() {}
public:
  void post(Request* message) {_post((void*)message, sizeof(Request));}
public:  
  Response* wait() {return (Response*)_wait();}
private:
  void     _tidIs(uint32_t value, void* request);
  uint32_t _tid(void* packet) const;  
};

}}

#endif
