// -*-Mode: C++;-*-
/**
@file Multicast.hh
@brief Define an Multicast Endpoint.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

#ifndef SERVICE_NET_MULTICAST
#define SERVICE_NET_MULTICAST

#include "net/Endpoint.hh"

namespace service {
namespace net  {

class Multicast : public service::net::Endpoint {
public:
  Multicast(uint32_t mcst, uint16_t port,  uint32_t interface)    : service::net::Endpoint(Endpoint::swap(mcst), port), _interface(interface) {}
  Multicast(uint32_t mcst, uint16_t port)                         : service::net::Endpoint(Endpoint::swap(mcst), port), _interface()          {}
  Multicast(const char* mcst, uint16_t port,  uint32_t interface) : service::net::Endpoint(mcst, port),                 _interface(interface) {}
  Multicast(const char* mcst, uint16_t port)                      : service::net::Endpoint(mcst, port),                 _interface() {}
public:
 ~Multicast() {}
public:
  uint32_t interface() const {return  _interface.addr();}   
private:
  Endpoint _interface; 
};

}}

#endif
