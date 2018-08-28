// -*-Mode: C++;-*-
/**
@file Endpoint.hh
@brief Specify the encapsulation of the information for a network connection.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

#ifndef SERVICE_NET_ENDPOINT
#define SERVICE_NET_ENDPOINT

#include <netinet/in.h>

namespace service {
namespace net  {
/*
**   Warning: The convention, unless otherwise stated for constructor arguments is that 
**   *address* arguments are assumed be in *network* order, while *port* arguments are
**   considered to be in *host* order. Beware...
*/
  
class Endpoint {
public:
  enum {SIZEOF_PSTRING = INET_ADDRSTRLEN};
public: // The following methods assume any arguments are in host order and return network order...
  static uint32_t swap(uint32_t in) {return htonl(in);} 
  static uint16_t swap(uint16_t in) {return htons(in);}
public:
  Endpoint& operator=(const Endpoint&);
public:
  Endpoint(const Endpoint&);         
  Endpoint(const Endpoint&, uint32_t fixup);
  Endpoint();                                
  Endpoint(uint32_t addr);                    
  Endpoint(uint16_t port);                    
  Endpoint(uint32_t    addr, uint16_t port);    
  Endpoint(const char* addr, uint16_t port = 0);
public: 
 ~Endpoint() {}
public:  // The following methods return values in network order...
  uint32_t addr() const {return _endpoint.sin_addr.s_addr;}  
  uint16_t port() const {return _endpoint.sin_port;}      
public:  // The following methods return values in host order...
  uint32_t addr_h() const {return ntohl(_endpoint.sin_addr.s_addr);}    
  uint16_t port_h() const {return ntohs(_endpoint.sin_port);}
public:  // The following methods return values as presentation strings...
  const char* addr_p(char* buffer) const;    
  const char* port_p(char* buffer) const;  
public:
  void fixup(uint32_t address) {if(_endpoint.sin_addr.s_addr == 0) _endpoint.sin_addr.s_addr = address;}
public:  
  void dump(int indent = 0) const;
private:
  sockaddr_in _endpoint;
};

}}

#endif
