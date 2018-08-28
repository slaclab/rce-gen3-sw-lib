#ifndef PortMap_hh
#define PortMap_hh


#include "fm/FmPort.hh"
#include "exception/Exception.hh"

#include <stdio.h>

class PortDescription {
public:
  enum Network  { Base, Fabric };
  enum Type { CPU, Zone2, RTM, Hub, FP_RJ45, FP_XFP, Unknown };

  PortDescription(Network netw, unsigned user_index,
		  Type    type, unsigned type_index,
		  const FmPort& port) : _port(port)
  {
    sprintf(_name    ,"%c%d" , (netw==Base) ? 'b' : 'f', user_index);
    switch(type) {
    case CPU    : sprintf(_location,"CPU"); break;
    case Zone2  :
      if (!type_index) sprintf(_location,"Z2-Slot 1/2");
      else             sprintf(_location,"Z2-Slot %d", type_index+1);
      break;
    case RTM    : sprintf(_location,"RTM-%d"    , type_index+1); break;
    case Hub    : sprintf(_location,"Z3Hub-%c"  , 'A' + type_index); break;
    case FP_RJ45: sprintf(_location,"FP-1"); break;
    case FP_XFP : sprintf(_location,"FP-2"); break;
    default     : sprintf(_location,"UNK"); break;
    }
  }
public:
  const char* name    () const { return _name; }
  const char* location() const { return _location; }
  FmPort      port    () const { return _port; }
private:
  char   _name    [16];
  char   _location[16];
  FmPort _port;
};

class PortMap {
public:
  virtual ~PortMap() {}
  virtual FmPort          port     (unsigned user_index) const = 0;
public:
  PortDescription port_desc(unsigned user_index) const throw(tool::exception::Event);
public:
  enum { CPU=0, Z2_Base=1, RTM_Base=17, HUB_Base=22, FP_RJ45=28, FP_XFP=29 };
  unsigned cpu_index    () const;
  unsigned fp_rj45_index() const;
  unsigned fp_xfp_index () const;
  unsigned zone2_index(unsigned channel) const; // channel=1-16
  unsigned rtm_index  (unsigned channel) const; // channel=0-4 (A-E)
  unsigned hub_index  (unsigned channel) const; // channel=0-3 (A-D)
public:
  static PortMap* base_map  ();
  static PortMap* fabric_map();
protected:
  PortDescription::Network _network;
};

#endif
