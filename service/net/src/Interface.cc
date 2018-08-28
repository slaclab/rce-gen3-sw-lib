// -*-Mode: C++;-*-
/**
@cond development
@file Interface.cc
@brief Implement the encapsulation of network interface information.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/


#include <stdio.h>
#include <string.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

#include "net/Interface.hh"
#include "net/os/Interface.hh"

using namespace service::net;

Interface::Interface() : _hwAddr(0), _ipAddr(0), _nmAddr(0), _bcAddr(0)
  {
  struct ifaddrs *ifAddr, *ifa;
  
  if (getifaddrs(&ifAddr) == -1)
    {
    return;
    }
  for (ifa = ifAddr; ifa; ifa = ifa->ifa_next)
    {
    if ( !ifa->ifa_addr )
      continue;
    if (AF_INET == ifa->ifa_addr->sa_family &&
        strncmp(ifa->ifa_name, LOOPBACK_NAME,strlen(LOOPBACK_NAME)))
      {
      _ifname = ifa->ifa_name;
      _ipAddr = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
      _nmAddr = ((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr.s_addr;
      break;
      }      
    }
  
  _hwAddr = _getHwAddr(0);
  _bcAddr  = _ipAddr | ~_nmAddr;
  }


Interface::Interface(const char* name=0) : _hwAddr(0), _ipAddr(0), _nmAddr(0), _bcAddr(0), _ifname(name)
  {
  struct ifaddrs *ifAddr, *ifa;
  
  if (getifaddrs(&ifAddr) == -1)
    {
    return;
    }
  for (ifa = ifAddr; ifa; ifa = ifa->ifa_next)
    {
    if ( !ifa->ifa_addr )
      continue;
    if (AF_INET == ifa->ifa_addr->sa_family && !strcmp(ifa->ifa_name,_ifname))
      {
      _ipAddr = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
      _nmAddr = ((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr.s_addr;
      break;
      }      
    }
  
  _hwAddr = _getHwAddr(0);
  _bcAddr  = _ipAddr | ~_nmAddr;
  }


uint64_t Interface::_getHwAddr(int this_socket) const
 {
  uint64_t hwAddr = 0;
  struct ifaddrs *ifAddr, *ifa;
  if (getifaddrs(&ifAddr) == -1)
    {
    return 0;
    }
  for (ifa = ifAddr; ifa; ifa = ifa->ifa_next)
    {
    if ( !ifa->ifa_addr )
      continue;
    if (AF_LINK == ifa->ifa_addr->sa_family &&
        strncmp(ifa->ifa_name, LOOPBACK_NAME,strlen(LOOPBACK_NAME)))
      {
      struct sockaddr_dl *sdl = (struct sockaddr_dl *)ifa->ifa_addr;
      unsigned char *cp = (unsigned char *)LLADDR(sdl);
      uint32_t a = (uint8_t)cp[0];
      uint32_t b = (uint8_t)cp[1];
      uint32_t c = (uint8_t)cp[2];
      uint32_t d = (uint8_t)cp[3];
      uint32_t e = (uint8_t)cp[4];
      uint32_t f = (uint8_t)cp[5];

      uint32_t low  = a | (b << 8) | (c << 16) | (d << 24);
      uint32_t high = e | (f << 8);
            
      hwAddr = (uint64_t)(low | ((uint64_t)high << 32));
      break;
      }
    }
  
  return hwAddr;
 }



static const char* BLANK = "";

static const char* LINE_1 = "%*sInterface \"%s\" has following addresses...\n";
static const char* LINE_2 = "%*sIP address: %s\n";
static const char* LINE_3 = "%*sNetmask: %s\n";
static const char* LINE_4 = "%*sHardware address: %02X:%02X:%02X:%02X:%02X:%02X\n";

#define OWN_INDENT 2

void Interface::dump(int indent) const
 {

 printf(LINE_1, indent, BLANK, name());
 
 char string[64];
 
 printf(LINE_2, indent + OWN_INDENT, BLANK, inet_ntop(AF_INET, (const void*)&_ipAddr, string, sizeof(string)));
 printf(LINE_3, indent + OWN_INDENT, BLANK, inet_ntop(AF_INET, (const void*)&_nmAddr, string, sizeof(string)));
 
 uint64_t address = _hwAddr;

 uint8_t a = address;
 uint8_t b = address >> 8;
 uint8_t c = address >> 16;
 uint8_t d = address >> 24;
 uint8_t e = address >> 32;
 uint8_t f = address >> 40;
 
 printf(LINE_4, indent + OWN_INDENT, BLANK, a, b, c, d, e, f);  
 
 return;
 }

/** @endcond */


// =========== Documentation ================
namespace service {
  namespace net {
/**
   @class Interface
   @brief Encapsulates network interface information.

   This class encapsulates the information needed for the operation of
   a network interface, including interface name and IP, netmask,
   broadcast, and hardware addresses.  It also provides a method to
   print this information to stdout.


   @fn Interface::Interface()
   @brief Construct an Interface object using the default network interface.

   The object created here will be populated from the first network
   interface found on the system (other than 'lo' or the loopback
   interface).  That is, if the network interfaces on a system are @c
   eth0 and @c eth1, this constructor will fetch the all address
   information from @c eth0 and ignore @c eth1.  If networking is not
   active, all attributes will be set to zero.


   @fn Interface::Interface(const char* name)
   @brief Construct an Interface object using the named interface.
   @param[in] name  A C string representing the name of the interface.

   Contrasting to the default constructor, the Interface created here
   will be populated from the named interface.  This includes the
   loopback interface.  If networking is not active, all attributes
   will be set to zero.


   @fn const char* Interface::name()   const
   @brief Return the Interface name as a C string.
   @return A const char* pointer to the Interface name.


   @fn uint32_t Interface::ipAddr() const
   @brief Return the Interface's IPV4 address in network byte order.
   @return The Interface's IPV4 address in network byte order.


   @fn uint32_t    Interface::nmAddr() const
   @brief Return the Interface's netmask in network byte order.
   @return The Interface's netmask in network byte order.


   @fn uint32_t    Interface::bcAddr() const
   @brief Return the Interface's broadcast address in network byte order.
   @return Return the Interface's broadcast address in network byte order
   @note The broadcast address is equal to  ipAddr() | ~nmAddr().


   @fn uint64_t    Interface::hwAddr() const
   @brief Return the Interface's hardware (or MAC) address in HOST byte order.
   @return The Interface's hardware (or MAC) address in HOST byte order.


   @fn void Interface::dump(int indent) const
   @brief Dump a representation of this Interface to stdout.
   @param[in] indent  The amount to indent the output (for nice formatting)
*/

  }
}
