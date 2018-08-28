
#include "EthernetConfig.hh"
#include "network/Params.hh"
#include "exception/Exception.hh"

#include <new>

namespace driver {

  namespace network {

    namespace net = driver::network;
    namespace exc = tool::exception;

    void
    EthernetConfig::add(const net::Params& params)
    {
      if (_nfaces < cfg::MaxFaces)
      {
        new (_faces+_nfaces++) net::Params(params);
      }
      else
      {
	throw exc::Error("Too many interfaces registered");
      }
    }

    const net::Params&
    EthernetConfig::interface(unsigned id) const
    {
      return _faces[id];
    }

  } // network

} // driver
