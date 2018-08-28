#ifndef CimNetDLFHandler_hh
#define CimNetDLFHandler_hh


#include "cm_net/Handler.hh"

class FmTahoeMgr;

namespace cm {
  namespace net {
    class Driver;
    class DLFHandler : public Handler {
    public:
      DLFHandler (FmTahoeMgr&);
      ~DLFHandler();

      bool handle(const FmLCIPacket&);
    private:
      FmTahoeMgr& _fm;
    };
  };
};

#endif
