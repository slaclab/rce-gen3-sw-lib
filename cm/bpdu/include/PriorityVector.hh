#ifndef CimBPDUPriorityVector_hh
#define CimBPDUPriorityVector_hh


#include "bpdu/BridgeId.hh"
#include <stdint.h>

namespace cm {
  namespace bpdu {
    class Frame;
    class PriorityVector {
    public:
      PriorityVector();
      PriorityVector(const Frame&,unsigned);
      PriorityVector(const BridgeId&);
      PriorityVector& operator=(const PriorityVector&);
      bool operator==(const PriorityVector&) const;
      bool operator< (const PriorityVector&) const;
      bool operator!=(const PriorityVector&) const;
    public:
      void dump() const;
    public:
      BridgeId rootBridgeId;
      uint32_t rootPathCost;
      BridgeId designatedBridgeId;
      uint16_t designatedPortId;
      uint16_t bridgePortId;
    };

  };
};

#endif
