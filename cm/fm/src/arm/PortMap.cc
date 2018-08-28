
#include "fm/PortMap.hh"

static PortMap* _base_map   = 0;
static PortMap* _fabric_map = 0;

PortDescription PortMap::port_desc(unsigned user_index) const throw(tool::exception::Event)
{
  PortDescription::Type type(PortDescription::Unknown);
  unsigned type_index(0);
  if      (user_index == PortMap::CPU     ) { type = PortDescription::CPU; }
  else if (user_index  < PortMap::RTM_Base) { type = PortDescription::Zone2; type_index = user_index-PortMap::Z2_Base; }
  else if (user_index  < PortMap::HUB_Base) { type = PortDescription::RTM;   type_index = user_index-PortMap::RTM_Base; }
  else if (user_index  < PortMap::FP_RJ45 ) { type = PortDescription::Hub;   type_index = user_index-PortMap::HUB_Base; }
  else if (user_index == PortMap::FP_RJ45)  { type = PortDescription::FP_RJ45; }
  else if (user_index == PortMap::FP_XFP )  { type = PortDescription::FP_XFP; }
  return PortDescription( _network, user_index, type,  type_index, port(user_index));
}

unsigned PortMap::cpu_index() const { return PortMap::CPU; }
unsigned PortMap::fp_rj45_index() const { return PortMap::FP_RJ45; }
unsigned PortMap::fp_xfp_index () const { return PortMap::FP_XFP; }
unsigned PortMap::zone2_index(unsigned channel) const { return Z2_Base +channel-1; }
unsigned PortMap::rtm_index  (unsigned channel) const { return RTM_Base+channel; }
unsigned PortMap::hub_index  (unsigned channel) const { return HUB_Base+channel; }


class BasePortMap_C00 : public PortMap {
public:
  BasePortMap_C00() { _network = PortDescription::Base; }
public:
  FmPort          port     (unsigned user_index) const throw(tool::exception::Event)
  {
    static unsigned map[] = { 0, 							// CPU
			      8, -1UL, 6, 4, 2, 1, 3, 5, 7, 9, 11, 15, 13, 19, 21, 17,  // Zone2 Ch1-16
			      16, 14, 10, 12, -1UL,					// RTM A-E
			      18, 22, 20, -1UL,       					// Zone3 A-D
			      -1UL, -1UL,
			      24, 23, -1UL, -1UL };
    if (map[user_index]==-1UL) throw tool::exception::Event("BasePortMap_C01::port_desc No port mapped to user index %d",user_index);
    return FmPort(map[user_index]);
  }
};

class BasePortMap_C01 : public PortMap {
public:
  BasePortMap_C01() { _network = PortDescription::Base; }
public:
  FmPort          port     (unsigned user_index) const throw(tool::exception::Event)
  {
    static unsigned map[] = { 0, 							// CPU
			      8, -1UL, 6, 4, 2, 19,  9, 11, 15, 13, 1, 3, 5, 7, 21, -1UL,	// Zone2 Ch1-16
			      14, 10, 12, -1UL, -1UL,					// RTM A-E
			      18, 22, -1UL, -1UL, 						// Zone3 A-D
			      -1UL, -1UL,
			      24, 23, -1UL, -1UL };
    if (map[user_index]==-1UL) throw tool::exception::Event("BasePortMap_C01::port_desc No port mapped to user index %d",user_index);
    return FmPort(map[user_index]);
  }
};

class FabricPortMap_C00 : public PortMap {
public:
  FabricPortMap_C00() { _network = PortDescription::Fabric; }
public:
  FmPort          port     (unsigned user_index) const throw(tool::exception::Event)
  {
    static unsigned map[] = { 0, 							// CPU
			      17, -1UL, 21, 19, 13, 15, 11, 9, 7, 5, 3, 1, 2, 4, 6, 8, 	// Zone2 Ch1-16
			      16, 14, 10, 12, -1UL,   					// RTM A-E
			      18, 22, 20, -1UL, 					// Zone3 A-D
			      -1UL, -1UL,
			      24, 23, -1UL, -1UL };
    if (map[user_index]==-1UL) throw tool::exception::Event("FabricPortMap_C00::port_desc No port mapped to user index %d",user_index);
    return FmPort(map[user_index]);
  }
};

class FabricPortMap_C01 : public PortMap {
public:
  FabricPortMap_C01() { _network = PortDescription::Fabric; }
public:
  FmPort          port     (unsigned user_index) const throw(tool::exception::Event)
  {
    static unsigned map[] = { 0, 							// CPU
			      17, -1UL, 21, 19, 13, 15, 11, 9, 7, 5, 3, 1, 2, 4, -1UL, -1UL,// Zone2 Ch1-16
			      16, 14, 10, 12, 8, 					// RTM A-E
			      18, 22, 20, 6,    					// Zone 3 A-D
			      -1UL, -1UL,
			      24, 23, -1UL, -1UL };
    if (map[user_index]==-1UL) throw tool::exception::Event("FabricPortMap_C01::port_desc No port mapped to user index %d",user_index);
    return FmPort(map[user_index]);
  }
};


PortMap* PortMap::base_map()
{
  if (!_base_map) {
    _base_map = (PortMap*)new BasePortMap_C01;
  }
  return _base_map;
}

PortMap* PortMap::fabric_map()
{
  if (!_fabric_map) {
    _fabric_map = (PortMap*)new FabricPortMap_C01;
  }
  return _fabric_map;
}
