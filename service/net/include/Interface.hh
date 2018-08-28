// -*-Mode: C++;-*-
/**
@file Interface.hh
@brief Encapsulate the relevant information needed to talk to a network interface.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

#ifndef SERVICE_NET_INTERFACE
#define SERVICE_NET_INTERFACE
#include <inttypes.h>

namespace service {
namespace net  {

class Interface {
public:
  Interface();
  Interface(const char* name);
public:
 ~Interface() {}
public:
  const char* name()   const {return _ifname;} 
  uint32_t    ipAddr() const {return _ipAddr;} 
  uint32_t    nmAddr() const {return _nmAddr;} 
  uint32_t    bcAddr() const {return _bcAddr;} 
  uint64_t    hwAddr() const {return _hwAddr;}  
public:
  void dump(int offset = 0) const;
private:
  uint64_t _getHwAddr(int) const;
  uint32_t _getIpAddr(int) const;
  uint32_t _getNmAddr(int) const;
  void     _getIfName(int, char* name) const;
private:
 uint64_t     _hwAddr;
 uint32_t     _ipAddr;
 uint32_t     _nmAddr;
 uint32_t     _bcAddr;
 const char*  _ifname;
};

}}

#endif
