#ifndef CimNetHandler_hh
#define CimNetHandler_hh


#include "container/Link.hh"

class FmLCIPacket;

namespace cm {
  namespace net {
    class Handler {
    public:
      virtual ~Handler() {}
      virtual bool handle(const FmLCIPacket& pkt)=0;
    };
    class HandlerPtr: public tool::container::Link<cm::net::HandlerPtr> {
    public:
      HandlerPtr(Handler* h) : handler(h) {}
      ~HandlerPtr() { delete handler; }

      Handler* handler;
    };
  };
}

#endif
