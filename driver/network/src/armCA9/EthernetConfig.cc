#include <string.h>
#include <stdio.h>

#include <bsp.h>


#include "../EthernetConfig.hh"
#include "network/Params.hh"
#include "exception/Exception.hh"
#include "bsi/Bsi_Cfg.h"
#include "bsi/Bsi.h"


namespace driver {

  namespace network {

    using namespace tool    :: exception;

    void EthernetConfig::retrieve(void *attach)
    {
      if (_nfaces < cfg::MaxFaces) {
        uint64_t   macAddr;
        char*      ifName = strdup("ethN");  ifName[3] = '0' + _nfaces;
        Bsi bsi = LookupBsi();
        if(!bsi) throw Error("BSI lookup failure");
        macAddr = BsiRead64(bsi, BSI_MAC_ADDR_OFFSET);
        Params params(ifName, (uint8_t*)&macAddr, (AttachFn)attach);

        _faces[_nfaces] = params;
        _nfaces++;
      } else {
	throw Error("Invalid interface requested");
      }
    }

    void EthernetConfig::store() const
    {
      // Not used
    }

  } // network

} // driver
