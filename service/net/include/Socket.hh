// -*-Mode: C++;-*-
/**
@file Socket.hh
@brief Encapsulate socket semantics for reading and writing.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/


#ifndef SERVICE_NET_SOCKET
#define SERVICE_NET_SOCKET



#include <netinet/in.h>
#include <sys/select.h>

#include "net/Endpoint.hh"
#include "net/Multicast.hh"

namespace service {
namespace net  {

class Socket  {
public:
  enum {WKS_BASE = 47040, WKS_SIZE = 512}; // here is the block of well-known port numbers to allocate from...
  enum {WKS_DSL = WKS_BASE + 0};
  enum {WKS_CDI = WKS_BASE + 1};
  enum {WKS_RRI = WKS_BASE + 64};
public:
  Socket();
  Socket(uint16_t local_port);
  Socket(uint32_t    foreign_addr, uint16_t foreign_port);
  Socket(const char* foreign_addr, uint16_t foreign_port);
  Socket(uint32_t    foreign_addr, uint16_t foreign_port, uint16_t local_port); 
  Socket(uint32_t    foreign_addr, uint16_t foreign_port, uint32_t local_addr, uint16_t local_port = 0); 
public:
 ~Socket();
public:
  int zombie() const {return _zombie;} 
public: 
  void  xmt(void* payload, uint32_t sizeof_payload, const Endpoint*);
  void  xmt(void* payload, uint32_t sizeof_payload, void* header, uint32_t sizeof_header, const Endpoint*);  
  void  xmt(void* payload, uint32_t sizeof_payload)                                                     {xmt(payload, sizeof_payload, &foreign);} 
  void  xmt(void* payload, uint32_t sizeof_payload, void* header, uint32_t sizeof_header)               {xmt(payload, sizeof_payload, header, sizeof_header, &foreign);}
public:
  void* rcv(void* payload, uint32_t sizeof_payload, Endpoint* = (Endpoint*)0); 
  void* rcv(void* payload, uint32_t sizeof_payload, void* header, uint32_t sizeof_header, Endpoint* = (Endpoint*)0);  
  void* rcv(void* payload, uint32_t sizeof_payload, uint32_t tmo, Endpoint* endpoint = (Endpoint*)0)    {return _wait(tmo) ? rcv(payload, sizeof_payload, endpoint) : (void*)0;}
  void* rcv(void* payload, uint32_t sizeof_payload, void* header, uint32_t sizeof_header, uint32_t tmo) {return _wait(tmo) ? rcv(payload, sizeof_payload, header, sizeof_header) : (void*)0;}  
public: 
  void* rcvUpdate(void* payload, uint32_t sizeof_payload)                                               {return rcv(payload, sizeof_payload, &foreign);}   
  void* rcvUpdate(void* payload, uint32_t sizeof_payload, uint32_t tmo)                                 {return _wait(tmo) ? rcv(payload, sizeof_payload, &foreign) : (void*)0;} 
public:
  int join(const Multicast& group);
  int drop(const Multicast& group);
public:
  Endpoint local;
  Endpoint foreign; 
public:
  void dump(int offset = 0) const;
private:
  int _wait(uint32_t tmo);
  int _set_options();
private:
  int    _socket;
  int    _zombie;
  fd_set _ready;
};

}}

#endif
