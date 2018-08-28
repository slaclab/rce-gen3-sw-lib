#ifndef CimNetBPDUHandler_hh
#define CimNetBPDUHandler_hh


#include "cm_net/Handler.hh"

class FmTahoeMgr;
namespace cm { namespace bpdu { class RSTPManager; }; };
namespace cm {
  namespace net {
    class Driver;
    class BPDUHandler : public Handler {
    public:
      BPDUHandler (FmTahoeMgr&, cm::net::Driver&);
      ~BPDUHandler();

      bool handle(const FmLCIPacket&);
    private:
      cm::bpdu::RSTPManager* _rstp;
      unsigned _id;
    };
  };
};

#endif
