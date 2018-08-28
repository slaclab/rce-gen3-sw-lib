#ifndef CimNetPauseHandler_hh
#define CimNetPauseHandler_hh


#include "cm_net/Handler.hh"

namespace cm {
  namespace net {
    class PauseHandler : public Handler {
    public:
      PauseHandler ();
      ~PauseHandler();

      bool handle(const FmLCIPacket&);
    };
  };
};

#endif
