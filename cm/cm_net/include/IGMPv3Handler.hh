#ifndef CimNetIGMPv3Handler_hh
#define CimNetIGMPv3Handler_hh


#include "cm_net/Handler.hh"

#include "fm/FmPort.hh"

class FmTahoeMgr;
struct ip_mreq;

namespace cm {
  namespace net {
    class IGMPv3Handler : public Handler {
    public:
      IGMPv3Handler (FmTahoeMgr& fm, unsigned ip);
      ~IGMPv3Handler();

      bool handle(const FmLCIPacket&);
    private:
      void _setup_trigger();
    private:
      FmTahoeMgr&     _fm;
      FmPort          _routerPort;
      int             _trigger_id;
      int             _socket;
    };
  };
};

#endif
