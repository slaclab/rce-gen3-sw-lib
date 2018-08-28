
#include "bpdu/PriorityVector.hh"
#include "bpdu/Frame.hh"

using namespace cm::bpdu;

PriorityVector::PriorityVector() {}

PriorityVector::PriorityVector(const Frame& f,unsigned p) :
  rootBridgeId      (f.rootId),
  rootPathCost      (f.rootPathCost),
  designatedBridgeId(f.bridgeId),
  designatedPortId  (f.portId),
  bridgePortId      (p)
{
}

PriorityVector::PriorityVector(const BridgeId& bridgeId) :
  rootBridgeId      (bridgeId),
  rootPathCost      (0),
  designatedBridgeId(bridgeId),
  designatedPortId  (0),
  bridgePortId      (0)
{
}

PriorityVector& PriorityVector::operator=(const PriorityVector& v)
{
  rootBridgeId       = v.rootBridgeId;
  rootPathCost       = v.rootPathCost;
  designatedBridgeId = v.designatedBridgeId;
  designatedPortId   = v.designatedPortId;
  bridgePortId       = v.bridgePortId;
  return *this;
}

bool PriorityVector::operator<(const PriorityVector& b) const
{
  const PriorityVector& a = *this;
  if (a.rootBridgeId < b.rootBridgeId) return true;
  if (a.rootBridgeId ==b.rootBridgeId) {
    if (a.rootPathCost <b.rootPathCost) return true;
    if (a.rootPathCost==b.rootPathCost) {
      if (a.designatedBridgeId <b.designatedBridgeId) return true;
      if (a.designatedBridgeId==b.designatedBridgeId &&
	  a.designatedPortId<=b.designatedPortId) return true;
    }
  }
  return false;
}

bool PriorityVector::operator==(const PriorityVector& b) const
{
  const PriorityVector& a = *this;
  return
    a.rootBridgeId      ==b.rootBridgeId &&
    a.rootPathCost      ==b.rootPathCost &&
    a.designatedBridgeId==b.designatedBridgeId &&
    a.designatedPortId  ==b.designatedPortId;
}

bool PriorityVector::operator!=(const PriorityVector& b) const
{
  return !operator==(b);
}

#include <stdio.h>

void PriorityVector::dump() const
{
  char buff[32];

  rootBridgeId.dump(buff);
  printf("rootId: %s cost %u\n",
	 buff,
	 rootPathCost);
  designatedBridgeId.dump(buff);
  printf("desgId: %s port %x/%x\n",
	 buff,
	 designatedPortId,
	 bridgePortId);
}
