/*
** ++
**  Package:
**	
**
**  Abstract:
**      
**
**  Author:
**      Michael Huffer, SLAC (mehsys@slac.stanford.edu)
**
**  Creation Date:
**	000 - , 2007
**
**  Revision History:
**	None.
**
** --
*/
 
#ifndef SERVICE_UDP_LINK
#define SERVICE_UDP_LINK



#include "net/Socket.hh"
#include "csi/Link.hh"

namespace service {
namespace udp  {

class Link : public tool::csi::Link {
public:
  Link();
  Link(uint16_t local_port);
  Link(uint32_t    foreign_addr, uint16_t foreign_port);
  Link(const char* foreign_addr, uint16_t foreign_port);
  Link(uint32_t    foreign_addr, uint16_t foreign_port, uint16_t local_port); 
  Link(uint32_t    foreign_addr, uint16_t foreign_port, uint32_t local_addr, uint16_t local_port = 0); 
public:
  virtual ~Link();
public:  
  void  post(void* packet, uint32_t length);
public:  
  void* wait(uint32_t tmo);
  void* wait();
public:  
  void* waitUpdate(uint32_t tmo);
  void* waitUpdate();  
public:
  const net::Endpoint& foreign() const {return _socket.foreign;}
  const net::Endpoint& local()   const {return _socket.local;}
public:
  uint32_t fixup() const {return _received.addr();}   
public:
  void dump(int offset = 0) const;
private:
  net::Socket   _socket;
  net::Endpoint _received;
  uint8_t*      _buffer;
};

}}

#endif
