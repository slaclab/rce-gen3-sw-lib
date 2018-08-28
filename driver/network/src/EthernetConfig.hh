#ifndef DRIVER_NETWORK_ETHERNETCONFIG_HH
#define DRIVER_NETWORK_ETHERNETCONFIG_HH


#include "network/Params.hh"
#include "networkcfg/params.hh"
#include "network/BsdNet_Config.h"


namespace driver {

  namespace network {

    namespace net = driver        :: network;
    namespace cfg = configuration :: network;

    class EthernetConfigFile;           // Forward declaration

    class EthernetConfig {
    public:
      EthernetConfig() : _nfaces(0) {}

    public:
      void retrieve(void *attach);
      void store() const;

    public:
      void               add(const net::Params& params);
      const net::Params& interface(unsigned id) const;

      const unsigned     numInterfaces() const  {return _nfaces;}
      unsigned           bufferSize()    const  {return _nfaces*sizeof(net::Params);}

    private:
      friend class EthernetConfigFile;
      unsigned    _nfaces;
      net::Params _faces[cfg::MaxFaces];
    };

  }

}
#endif
