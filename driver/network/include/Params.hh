#if !defined(DRIVER_NETWORK_PARAMS_HH)
#define      DRIVER_NETWORK_PARAMS_HH

#include <rtems/rtems_bsdnet.h>

#include "string/ip2dot.hh"
#include "string/id2node.hh"


namespace driver {

  namespace network {

    enum {MaxName = 32, HwAddrLen = 6};

    typedef int (*AttachFn)(struct rtems_bsdnet_ifconfig *conf, int attaching);

    class Params {
    public:
      Params();
      Params(const Params&);
      Params(const char* name,
	     const char* mac,
             AttachFn    attach);
      Params(const char*   name,
             const uint8_t mac[HwAddrLen],
             AttachFn      attach);

      unsigned       unit()   const;
      const char*    name()   const {return _name;}
      const uint8_t* hwaddr() const {return _hwaddr;}
      AttachFn       attach() const {return _attach;}

    private:
      char     _name[MaxName];
      uint8_t  _hwaddr[HwAddrLen];
      AttachFn _attach;
    };


    class Routing {
    public:
      Routing();
      Routing(const char* host,
	      const char* domain,
	      const char* gateway,
	      const char* dns,
	      const char* ntp);

    public:
      const char* host() const;
      const char* domain() const;
      const char* gateway() const;
      const char* dns() const;
      const char* ntp() const;

    private:
      enum {MaxDotted=tool::string::MaxDotted, MaxName=tool::string::MaxName};
      char _host[MaxName];
      char _domain[MaxName];
      char _gateway[MaxDotted];
      char _dns[MaxDotted];
      char _ntp[MaxDotted];
    };
  }
}
#endif
