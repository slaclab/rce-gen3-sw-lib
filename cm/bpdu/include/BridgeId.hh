#ifndef CimBPDUBridgeId_hh
#define CimBPDUBridgeId_hh

#include <sys/types.h>
#include <net/ethernet.h>

namespace cm {
  namespace bpdu {
    class BridgeId {
    public:
      BridgeId();
      BridgeId(ether_addr& hwaddr, unsigned priority, unsigned localid);
      BridgeId(const BridgeId&);
    public:
      ether_addr hwaddr() const;
    public:
      bool operator==(const BridgeId&) const;
      bool operator!=(const BridgeId&) const;
      bool operator <(const BridgeId&) const;
      BridgeId& operator=(const BridgeId&);
    public:
      void dump(char*) const;
    private:
      u_char _value[8];
    };
  };
};

#endif
