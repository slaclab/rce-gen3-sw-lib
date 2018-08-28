// -*-Mode: C;-*-
/**
@file
@brief Manage a set of fixed size buffers all with specified memory attributes. The 
collection of functions described below allows a set (or pool) of fixed-size buffers to be
created from a specified memory region. The size and number of buffers is determined by 
the user. Once the pool is created buffers can be arbitrary allocated and deallocated 
from and to the created pool.  

@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
POOL

@author
Michael Huffer, SLAC (mehsys@slac.stanford.edu)

@par Date created:
2013/10/28

@par Last commit:
\$Date: 2013-10-07 18:56:03 -0700 (Mon, 07 Oct 2013) $ by \$Author: tether $.

@par Revision number:
\$Revision: 2191 $

@par Location in repository:
\$HeadURL: file:///afs/slac/g/cci/repositories/ctkrepo/tool/trunk/memory/src/armCA9/mem.h $

@par Credits:
SLAC

*/

#ifndef SOCKET
#define SOCKET

#include <stdint.h>
#include "PseudoARP.hh"
    
/*
**
*/

#define SIZEOF_DATAGRAM 11 // 2K (Maximum size of datagram as a power of two...)

class Datagram {
public:
  Datagram() {};
 ~Datagram() {};
public:
  uint32_t  length()  {return  _content[0];}
  uint32_t  host()    {return  _content[1];}
  uint32_t  port()    {return  _content[2];}
  uint32_t* content() {return &_content[4];}
public:
  void free();  
private:
  uint32_t _content[1 << SIZEOF_DATAGRAM]; 
  }; 
 
/*
**
*/

class Socket {
public:
  Socket(unsigned numof_buffers);
  ~Socket() {}; 
public:
  void      sendTo(uint32_t dst_adr, uint32_t dst_port, uint32_t* datagram, uint32_t sizeof_datagram);
  Datagram* wait();
  void      close();
private:
  uint64_t    _own_mac;
  uint32_t    _own_adr;
  uint32_t    _own_port;
  uint32_t    _mib;
  uint32_t*   _inbound_datagram;
  SAS_Session _inbound_session;
  SAS_Mbx     _inbound_mbx;
  SAS_Session _outbound_session;
  SAS_ObMbx   _outbound_mbx;
  examples::PseudoARP* _arp;
  };  

/*
**
*/
  
#define IPV4_NUMBER  4 // Header version
#define IPV4_LENGTH  5 // Header length
#define IPV4_VERSION ((IPV4_NUMBER << 4) | IPV4_NUMBER)

#define IPV4_UDP    17 // IP protocol number for UDP

/*
**
*/
    
typedef struct  __attribute__((__packed__)) {
  uint8_t   version;
  uint8_t   tos;      // Type Of Service
  uint16_t  length;   // Total datagram length;
  uint16_t  id;
  uint16_t  offset;
  uint8_t   ttl;
  uint8_t   protocol;
  uint16_t  checksum;
  uint32_t  src;
  uint32_t  dst;
  } IpHeader;

/*
**
*/
  
typedef struct {
  uint16_t src;
  uint16_t dst;
  uint16_t length;   // Total datagram length;
  uint16_t checksum;
  } UdpHeader;
  
/*
**
*/

typedef struct {
 Xaui_Header ethernet;
 IpHeader    ip;
 UdpHeader   udp;
 uint32_t    mbz;
 } UdpPacket;
 
#endif
