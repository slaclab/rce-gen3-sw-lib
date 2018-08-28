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
 
#ifndef SERVICE_DSL_SERVER
#define SERVICE_DSL_SERVER


#include "kvt/Kvt.h"
#include "concurrency/Thread.hh"
#include "udp/Link.hh"
#include "dsl/impl/Proxy.hh"
#include "dsl/Location.hh"
#include "dsl/Space.hh"
#include "dsl/impl/Response.hh"

namespace service {
  namespace dsl  {

    class Server : public tool::concurrency::Thread {
    public:
      enum {MAX_SERVICES = 128};
    public:
      static void startup();
      static void startup(const char **services);
      static void bind(Space*);
      static void run();
      static void execute();
    public: 
      Server();
    public:
      ~Server() {}
    public:
      virtual void body(); 
    private:  
      uint64_t _hash(const char* string) const;  
      Space*   _lookup(const char* service);
    private:
      uint64_t        _magic_cookie;
      KvTable         _kvtable;
      udp::Link       _link;
      uint8_t         _buffer[sizeof(Response)]; 
    };

  }
}

#endif


