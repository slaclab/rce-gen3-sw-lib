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
**	000 - January 09, 2007
**
**  Revision History:
**	None.
**
** --
*/



#include "udp/Link.hh"

#define MAX_DATAGRAM 8192

using namespace tool;
using namespace service::udp;

/*
** ++
**
**
** --
*/

Link::Link() :
  csi::Link(),
  _socket(),
  _received(),
  _buffer(new uint8_t[MAX_DATAGRAM])
  {
  }

/*
** ++
**
**
** --
*/

Link::Link(uint16_t local_port) :
  csi::Link(),
  _socket(local_port),
  _received(),
  _buffer(new uint8_t[MAX_DATAGRAM])
  {
  }

/*
** ++
**
**
** --
*/

Link::Link(uint32_t foreign_addr, uint16_t foreign_port) :
  csi::Link(),
  _socket(foreign_addr, foreign_port),
  _received(),
  _buffer(new uint8_t[MAX_DATAGRAM])
  {
  }

/*
** ++
**
**
** --
*/

Link::Link(const char* foreign_addr, uint16_t foreign_port) :
  csi::Link(),
  _socket(foreign_addr, foreign_port),
  _received(),
  _buffer(new uint8_t[MAX_DATAGRAM])
  {
  }

/*
** ++
**
**
** --
*/

Link::Link(uint32_t foreign_addr, uint16_t foreign_port, uint16_t own_port) :
  csi::Link(),
  _socket(foreign_addr, foreign_port, own_port),
  _received(),
  _buffer(new uint8_t[MAX_DATAGRAM])
  {
  }

/*
** ++
**
**
** --
*/

Link::Link(uint32_t foreign_addr, uint16_t foreign_port, uint32_t local_addr, uint16_t local_port) : 
  csi::Link(),
  _socket(foreign_addr, foreign_port, local_addr, local_port),
  _received(),
  _buffer(new uint8_t[MAX_DATAGRAM])
  {
  }

/*
** ++
**
**
** --
*/

void  Link::post(void* packet, uint32_t length)
 {
 
 _socket.xmt(packet, length);
 
 return;
 }

/*
** ++
**
**
** --
*/

void* Link::wait()
 {
 
 return _socket.rcv((void*)_buffer, MAX_DATAGRAM, &_received);
 }

/*
** ++
**
**
** --
*/

void* Link::wait(uint32_t tmo)
 {
 
 return _socket.rcv((void*)_buffer, MAX_DATAGRAM,  tmo, &_received);
 }

/*
** ++
**
**
** --
*/

void* Link::waitUpdate()
 {

 return _socket.rcvUpdate((void*)_buffer, MAX_DATAGRAM); 
 }

/*
** ++
**
**
** --
*/

void* Link::waitUpdate(uint32_t tmo)
 {
 
 return _socket.rcvUpdate((void*)_buffer, MAX_DATAGRAM, tmo);
 }

/*
** ++
**
**
** --
*/

Link::~Link()
  {
  
  delete _buffer;

  }
   

