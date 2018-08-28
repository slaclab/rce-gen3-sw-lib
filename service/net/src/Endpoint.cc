// -*-Mode: C++;-*-
/**
@cond development
@file Endpoint.cc
@brief Implement the encapsulation of the information for a network connection.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "net/Endpoint.hh"


using namespace service::net;

Endpoint& Endpoint::operator=(const Endpoint& clone)     
  {
  
  _endpoint.sin_family      = AF_INET;
  _endpoint.sin_addr.s_addr = clone._endpoint.sin_addr.s_addr;
  _endpoint.sin_port        = clone._endpoint.sin_port; 
  
  return *this;
  }
     
Endpoint::Endpoint(const Endpoint& clone)            
  {
  
  _endpoint.sin_family      = AF_INET; 
  _endpoint.sin_addr.s_addr = clone._endpoint.sin_addr.s_addr;
  _endpoint.sin_port        = clone._endpoint.sin_port;
  
  } 

Endpoint::Endpoint(const Endpoint& clone, uint32_t fixup)            
  {
  
  uint32_t addr = clone._endpoint.sin_addr.s_addr;
  
  if(addr == INADDR_ANY) addr = fixup;
  
  _endpoint.sin_family      = AF_INET; 
  _endpoint.sin_addr.s_addr = addr;
  _endpoint.sin_port        = clone._endpoint.sin_port;
  
  } 
 
Endpoint::Endpoint()                                 
  {
  
  _endpoint.sin_family      = AF_INET;
  _endpoint.sin_addr.s_addr = INADDR_ANY; 
  _endpoint.sin_port        = 0;
  
  }
  
 
Endpoint::Endpoint(uint32_t addr)                    
  {
  
  _endpoint.sin_family      = AF_INET;
  _endpoint.sin_addr.s_addr = addr; 
  _endpoint.sin_port        = 0;
  
  }
 
Endpoint::Endpoint(uint16_t port)                    
  {
  
  _endpoint.sin_family      = AF_INET;
  _endpoint.sin_addr.s_addr = INADDR_ANY;
  _endpoint.sin_port        = htons(port);
  
  }

  
Endpoint::Endpoint(uint32_t addr, uint16_t port)     
  {
  
  _endpoint.sin_family      = AF_INET;
  _endpoint.sin_addr.s_addr = addr;  
  _endpoint.sin_port        = htons(port);
  
  }

#define SUCCESS 1

Endpoint::Endpoint(const char* addr, uint16_t port)
  {
  
  _endpoint.sin_family = AF_INET;
  
  if(inet_pton(AF_INET, addr, (void*)&_endpoint.sin_addr.s_addr) != SUCCESS) _endpoint.sin_addr.s_addr = INADDR_ANY;
 
  _endpoint.sin_port = htons(port);
   
  }

const char* Endpoint::addr_p(char* buffer) const
 {
 
 return inet_ntop(AF_INET, (const void*)&_endpoint.sin_addr.s_addr, buffer, SIZEOF_PSTRING);
 }

static const char* CONTROL = "%u";

const char* Endpoint::port_p(char* buffer) const
 {

 sprintf(buffer, CONTROL, ntohs(_endpoint.sin_port));  

 return buffer;
 }

static const char* BLANK = "";

static const char* LINE_1 = "%*sAddress: %s\n";
static const char* LINE_2 = "%*sPort:    %s (decimal)\n";

#define OWN_INDENT 2

/** @brief
    @param[in] 
 */
void Endpoint::dump(int indent) const
 {

 char str[64];
 
 printf(LINE_1, indent + OWN_INDENT, BLANK, addr_p(str));
 printf(LINE_2, indent + OWN_INDENT, BLANK, port_p(str));  

 return;
 }

/* @endcond */


// =========== Documentation ================

namespace service {
  namespace net {
/**
   @class Endpoint
   @brief Encapsulates a single network socket information (address, socket).

   This class encapsulates network socket information in order to be
   able to transfer and reference that information atomically.  It
   provides methods for retreiving the address and socket information
   in whichever byte ordering is needed.  Unless otherwise specified,
   the address arguments are in network order and port arguments are
   in host order.
   
   All Endpoints are designed as IPV4.

   @var Endpoint::SIZEOF_PSTRING
   @brief The maximum buffer size for an internet address presentation string.
   
   @fn Endpoint& Endpoint::operator=(const Endpoint& clone) 
   @brief Equals operator.  Copy one Endpoint to another.
   @param[in] clone  The endpoint to copy.
   @return A reference to the copied Endpoint.

   
   @fn Endpoint::Endpoint(const Endpoint& clone)
   @brief Copy constructor.
   @param[in] clone The Endpoint to copy.

   
   @fn Endpoint::Endpoint(const Endpoint& clone, uint32_t fixup)
   @brief Copy constructor with new address
   @param[in] clone The Endpoint to copy.
   @param[in] fixup The new IPV4 address (network byte order).

   
   @fn Endpoint::Endpoint()
   @brief Construct an Endpoint with default attributes (port=0, addr=INADDR_ANY).


   @fn Endpoint::Endpoint(uint32_t addr)
   @brief Construct an Endpoint with port 0 and a specific address.
   @param[in] addr The IPV4 address (network order).
   

   @fn Endpoint::Endpoint(uint16_t port)
   @brief Construct an Endpoint with a specific port and address==INADDR_ANY.
   @param[in] port The network port number (host order).


   @fn Endpoint::Endpoint(uint32_t addr, uint16_t port)     
   @brief Construct an Endpoint with a specific port and address.
   @param[in] addr The IPV4 address (network order).
   @param[in] port The network port number (host order).


   @fn Endpoint::Endpoint(const char* addr, uint16_t port)
   @brief Construct an Endpoint with a specific port and address.
   @param[in] addr The IPV4 address string in dot format.
   @param[in] port The network port number (host order).


   @fn const char* Endpoint::addr_p(char* buffer) const
   @brief Convert the IPV4 address of this Endpoint to a presentation string (dot format).
   @param[in] buffer  The string buffer in which to construct the presentation address.  This buffer must be SIZEOF_PSTRING in length.
   @return A pointer to the presentation string.

   
   @fn const char* Endpoint::port_p(char* buffer) const
   @brief Convert the port of this Endpoint to a presentation string (integer string).
   @param[in] buffer The string buffer in which to construct the presentation address.  This buffer must be at least 6 bytes in length.
   @return A pointer to the presentation string.

   
   @fn void Endpoint::dump(int indent) const
   @brief Dump a representation of this Endpoint to stdout.
   @param[in] indent  The amount to indent the output (for nice formatting).


   @fn uint32_t Endpoint::addr() const
   @brief Return the IPV4 address in network byte order.
   @return The address in network byte order.

   
   @fn uint16_t Endpoint::port() const
   @brief Return the port number in network byte order.
   @return The port number in network byte order.

   
   @fn uint32_t Endpoint::addr_h() const
   @brief Return the IPV4 address in host byte order.
   @return The address in host byte order.

   
   @fn uint16_t Endpoint::port_h() const
   @brief Return the port number in host byte order.
   @return The port number in host byte order.

   
   @fn void Endpoint::fixup(uint32_t address)
   @brief If the current address is INADDR_ANY, replace it with a new address.
   @param[in] address The new IPV4 address in network byte order.

   
   @fn static uint32_t Endpoint::swap(uint32_t in)
   @brief Swap the argument from host order to network byte order.
   @param[in] in The input argument.
   @return The input argument, swapped to network byte order.

   
   @fn static uint16_t Endpoint::swap(uint16_t in)
   @brief Swap the argument from host order to network byte order.
   @param[in] in The input argument.
   @return The input argument, swapped to network byte order.
   
*/

  }
}
