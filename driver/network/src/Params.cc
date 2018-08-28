

#include "network/Params.hh"

#include <string.h>
#include <stdio.h>

namespace driver {

  namespace network {

    Params::Params()
    {
      memset(this, 0, sizeof(*this));
    }

    Params::Params(const Params& p)
    {
      strncpy(_name, p._name, MaxName);
      memcpy(_hwaddr, p._hwaddr, HwAddrLen);
      _attach = p._attach;
    }

    Params::Params(const char* iname,
                   const char* imac,
                   AttachFn    attach)
    {
      strncpy(_name, iname, MaxName);
      unsigned mac[HwAddrLen];
      sscanf(imac, "%x:%x:%x:%x:%x:%x",
             &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
      for (unsigned i=0; i<HwAddrLen; i++) {
	_hwaddr[i] = mac[i];
      }
      _attach = attach;
    }

    Params::Params(const char*   iname,
                   const uint8_t imac[HwAddrLen],
                   AttachFn      attach)
    {
      strncpy(_name, iname, MaxName);
      for (unsigned i=0; i<HwAddrLen; i++) {
	_hwaddr[i] = imac[i];
      }
      _attach = attach;
    }

    unsigned Params::unit() const
    {
      const char* name = _name;
      char c;
      while ((c = *name++)) {
	if ((c >= '0') && (c <= '9')) {
	  return c - '0';
	}
      }
      return 0;
    }


    Routing::Routing()
    {
      memset(this, 0, sizeof(*this));
    }

    Routing::Routing(const char* host,
                     const char* domain,
                     const char* gateway,
                     const char* dns,
                     const char* ntp)
    {
      strncpy(_host,    host,    MaxName);
      strncpy(_domain,  domain,  MaxName);
      strncpy(_gateway, gateway, MaxDotted);
      strncpy(_dns,     dns,     MaxDotted);
      strncpy(_ntp,     ntp,     MaxDotted);
    }

    const char* Routing::host() const {return _host;}
    const char* Routing::domain() const {return _domain;}
    const char* Routing::gateway() const {return _gateway;}
    const char* Routing::dns() const {return _dns;}
    const char* Routing::ntp() const {return _ntp;}

  }
}
