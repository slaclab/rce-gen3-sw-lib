#ifndef CimNetLACPDUHandler_hh
#define CimNetLACPDUHandler_hh


#include "net/Handler.hh"

class FmTahoeMgr;
namespace cm { namespace lacp { class Manager; }; };
namespace cm {
  namespace net {
    class Driver;
    class LACPDUHandler : public Handler {
    public:
      LACPDUHandler (FmTahoeMgr&, cm::net::Driver&);
      ~LACPDUHandler();

      bool handle(const FmLCIPacket&);
    private:
      cm::lacp::Manager* _mgr;
      unsigned _id;
    };
  };
};

#endif
