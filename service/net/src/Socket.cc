// -*-Mode: C++;-*-
/**
@cond development
@file Socket.cc
@brief Encapsulate socket semantics for reading and writing.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/


#include <stdlib.h>
#include <stdio.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <netdb.h>
#include <sys/param.h>
#include <arpa/inet.h>

//#include <strings.h>

#include "net/Socket.hh"

#define ERROR -1
#define XMT_FLAGS 0
#define RCV_FLAGS 0
#define NO_DST ((struct sockaddr*)0)
#define NO_LENGTHOF_DST ((socklen_t*)0)

using namespace service::net;

Socket::Socket() :
  local(),
  foreign(),
  _socket(socket(AF_INET, SOCK_DGRAM, 0)),
  _zombie(_set_options())
  {
  }

Socket::Socket(uint16_t local_port) :
  local(local_port),
  foreign(),
  _socket(socket(AF_INET, SOCK_DGRAM, 0)),
  _zombie(_set_options())
  {
  }

Socket::Socket(uint32_t foreign_addr, uint16_t foreign_port) :
  local(),
  foreign(foreign_addr, foreign_port),
  _socket(socket(AF_INET, SOCK_DGRAM, 0)),
  _zombie(_set_options())
  {
  }

Socket::Socket(const char* foreign_addr, uint16_t foreign_port) :
  local(),
  foreign(foreign_addr, foreign_port),
  _socket(socket(AF_INET, SOCK_DGRAM, 0)),
  _zombie(_set_options())
  {
  }

Socket::Socket(uint32_t foreign_addr, uint16_t foreign_port, uint16_t local_port) :
  local(local_port),
  foreign(foreign_addr, foreign_port),
  _socket(socket(AF_INET, SOCK_DGRAM, 0)),
  _zombie(_set_options())
  {
  }

Socket::Socket(uint32_t foreign_addr, uint16_t foreign_port, uint32_t local_addr, uint16_t local_port) :
  local(local_addr, local_port),
  foreign(foreign_addr, foreign_port),
  _socket(socket(AF_INET, SOCK_DGRAM, 0)),
  _zombie(_set_options())
  {
  }

int Socket::_set_options()
 {

 int socket = _socket;

 if(socket == ERROR) return errno;

 int yes = 1;

 if(setsockopt(socket, SOL_SOCKET, SO_BROADCAST, (void*)&yes, sizeof(yes)) != ERROR)
  {
  if(bind(socket, (struct sockaddr*)&local, sizeof(struct sockaddr_in)) != ERROR)
    {
    socklen_t lengthof_own = sizeof(sockaddr_in);
    if(getsockname(socket, (struct sockaddr*)&local, &lengthof_own) != ERROR) return 0;
    }
  }

 close(socket);

 return errno;
 }


int Socket::join(const Multicast& group)
  {

  struct ip_mreq ipMreq;

  ipMreq.imr_multiaddr.s_addr = group.addr();;
  ipMreq.imr_interface.s_addr = group.interface();

  int status = setsockopt (_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&ipMreq, sizeof(ipMreq));

  return status < 0 ? errno : 0;
  }


int Socket::drop(const Multicast& group)
  {

  struct ip_mreq ipMreq;

  ipMreq.imr_multiaddr.s_addr = group.addr();;
  ipMreq.imr_interface.s_addr = group.interface();

  int status = setsockopt (_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&ipMreq, sizeof(ipMreq));

  return status < 0 ? errno : 0;
  }

void Socket::xmt(void* payload, uint32_t sizeof_payload, const Endpoint* dst)
 {

 struct iovec buffer;

 buffer.iov_base = payload;
 buffer.iov_len  = sizeof_payload;

 struct msghdr message;

 message.msg_name       = (void*)dst;
 message.msg_namelen    = sizeof(struct sockaddr_in);
 message.msg_iov        = &buffer;
 message.msg_iovlen     = 1;
 message.msg_control    = (void*)0;
 message.msg_controllen = 0;
 message.msg_flags      = 0;

 int length = sendmsg(_socket, &message, XMT_FLAGS);

 if(length == ERROR) _zombie = errno;

 return;
 }

void Socket::xmt(void* payload, uint32_t sizeof_payload, void* header, uint32_t sizeof_header, const Endpoint* dst)
 {

 struct iovec buffers[2];

 buffers[0].iov_base = header;
 buffers[0].iov_len  = sizeof_header;

 buffers[1].iov_base = payload;
 buffers[1].iov_len  = sizeof_payload;

 struct msghdr message;

 message.msg_name       = (void*)dst;
 message.msg_namelen    = sizeof(Endpoint);
 message.msg_iov        = buffers;
 message.msg_iovlen     = 2;
 message.msg_control    = (void*)0;
 message.msg_controllen = 0;
 message.msg_flags      = 0;

 int length = sendmsg(_socket, &message, XMT_FLAGS);
 if (length == ERROR) _zombie = errno;

 return;
 }

void* Socket::rcv(void* payload, uint32_t sizeof_payload, Endpoint* src)
 {

 struct iovec buffer;

 buffer.iov_base = payload;
 buffer.iov_len  = sizeof_payload;

 struct msghdr message;

 message.msg_name       = (void*)src;
 message.msg_namelen    = sizeof(Endpoint);
 message.msg_iov        = &buffer;
 message.msg_iovlen     = 1;
 message.msg_control    = (void*)0;
 message.msg_controllen = 0;
 message.msg_flags      = 0;

 int socket = _socket;
 int status = recvmsg(socket, &message, RCV_FLAGS);

 while(status == ERROR) status = recvmsg(socket, &message, RCV_FLAGS);

 return payload;
 }

void* Socket::rcv(void* payload, uint32_t sizeof_payload, void* header, uint32_t sizeof_header, Endpoint* src)
 {

 struct iovec buffers[2];

 buffers[0].iov_base = header;
 buffers[0].iov_len  = sizeof_header;

 buffers[1].iov_base = payload;
 buffers[1].iov_len  = sizeof_payload;

 struct msghdr message;

 message.msg_name       = (void*)src;
 message.msg_namelen    = sizeof(Endpoint);
 message.msg_iov        = buffers;
 message.msg_iovlen     = 2;
 message.msg_control    = (void*)0;
 message.msg_controllen = 0;
 message.msg_flags      = 0;

 int socket = _socket;
 int status = recvmsg(socket, &message, RCV_FLAGS);

 while(status == ERROR) status = recvmsg(socket, &message, RCV_FLAGS);

 return header;
 }

int Socket::_wait(uint32_t tmo)
 {
   int status;
   struct timeval timout;

   timout.tv_sec  = 0;
   timout.tv_usec = tmo << 10; // 1 tic = 1024 microseconds;

   do {
     int socket = _socket;

     FD_ZERO(&_ready);
     FD_SET(socket, &_ready);

     status = select(socket + 1, &_ready, (fd_set*)0, (fd_set*)0, &timout);
   } while(status == ERROR);
   return status;
 }


static const char* BLANK  = "";
static const char* LINE_1 = "%*sThis Link has the following local & foreign end points...\n";

#define OWN_INDENT 2

void Socket::dump(int indent) const
 {

 printf(LINE_1, indent, BLANK);

 local.dump(  indent + OWN_INDENT);
 foreign.dump(indent + OWN_INDENT);

 return;
 }

Socket::~Socket()
  {

  int socket = _socket;

  if(socket != ERROR) close(socket);

  }



/* @endcond */


// =========== Documentation ================

namespace service {
  namespace net {
/**
   @class Socket
   @brief Encapsulate a network socket connection.

   This class encapsulates the information needed for the operation of an
   IPV4 network datagram socket.  It abstracts away the libc functions and structures
   (such as @c setsockopt, @c bind, and @c struct @c sockaddr) for ease
   of use.  This sacrifices some flexibility (i.e. Unix sockets, IPV6), but
   this should be sufficient for most uses in an embedded system.


   @var Socket::WKS_BASE
   @brief Base address of a set of "Well Known" socket ports for the RPT system.

   @var Socket::WKS_SIZE
   @brief The size of the set of well-known socket port set.

   @var Socket::WKS_DSL
   @brief The well-known socket for the DSL system.

   @var Socket::WKS_CDI
   @brief The well-known socket for the CDI system.

   @var Socket::WKS_RRI
   @brief The well-known socket for the RRI system.

   @fn Socket::Socket()
   @brief Construct a Socket with default options.

   Socket::local and Socket::foreign will be default Endpoint objects
   (i.e. port=0 & addr=INADDR_ANY)


   @fn Socket::Socket(uint16_t local_port)
   @brief Construct a Socket with the local port equal to the argument.
   @param[in] local_port  The local socket port number in host byte order.


   @fn Socket::Socket(uint32_t foreign_addr, uint16_t foreign_port)
   @brief Construct a Socket with Socket::foreign created with the arguments.
   @param[in] foreign_addr  IP address of foreign socket in network byte order.
   @param[in] foreign_port  Port number of foreign socket in host byte order.
   @note The Socket::local member will be bound to the default Endpoint.


   @fn Socket::Socket(const char* foreign_addr, uint16_t foreign_port)
   @brief Construct a Socket with Socket::foreign created with the arguments.
   @param[in] foreign_addr The IPV4 address string in dot format.
   @param[in] foreign_port The network port (host order).


   @fn Socket::Socket(uint32_t    foreign_addr, uint16_t foreign_port, uint16_t local_port)
   @brief Construct a Socket with specific arguments for Socket::foreign and Socket::local on a specific port (with address=INADDR_ANY).
   @param[in] foreign_addr  IP address of foreign socket in network byte order.
   @param[in] foreign_port Port number of foreign socket in host byte order.
   @param[in] local_port The local socket port number in host byte order.

   @fn Socket::Socket(uint32_t    foreign_addr, uint16_t foreign_port, uint32_t local_addr, uint16_t local_port = 0)
   @brief Construct a Socket with specific arguments for Socket::foreign and Socket::local on a specific address and an optional port number.
   @param[in] foreign_addr  IP address of foreign socket in network byte order.
   @param[in] foreign_port Port number of foreign socket in host byte order.
   @param[in] local_addr Bind the socket to a specific local address (for when the system has more than one Interface).
   @param[in] local_port The local socket port number in host byte order.

   @fn Socket::~Socket()
   @brief Closes the socket.

   @fn int Socket::zombie() const
   @brief Indication of whether or not the Socket had trouble in starting.
   @return Zero (0) for success, otherwise the errno of the underlying call.

   Socket::zombie is meant to be called after either one of the constructors
   fails or one of the functions which return void fails.  If the
   setting of the underlying socket options fails, the errno of the underlying
   call is returned.


   @fn void  Socket::xmt(void* payload, uint32_t sizeof_payload, const Endpoint* dst)
   @brief Transmit a packet to a specific Endpoint.
   @param[in] payload  The buffer to transmit.
   @param[in] sizeof_payload  The number of bytes to transmit.
   @param[in] dst A pointer to the remote Endpoint to send the packet to.
   If dst is zero (0), the Socket is assumed to be connected.


   @fn void  Socket::xmt(void* payload, uint32_t sizeof_payload, void* header, uint32_t sizeof_header, const Endpoint* dst)
   @brief Transmit a packet with header and payload.
   @param[in] payload  The payload of the packet to transmit.
   @param[in] sizeof_payload  The number of bytes in the payload to transmit.
   @param[in] header  The header of the packet to transmit.
   @param[in] sizeof_header The number of bytes in the header.
   @param[in] dst A pointer to the remote Endpoint to send the packet to.
   If dst is zero (0), the Socket is assumed to be connected.
   @note The header is transmitted first, then the payload.

   @fn void  Socket::xmt(void* payload, uint32_t sizeof_payload)
   @brief Transmit a packet to Socket::foreign.
   @param[in] payload  The buffer to transmit.
   @param[in] sizeof_payload  The number of bytes to transmit.


   @fn void  Socket::xmt(void* payload, uint32_t sizeof_payload, void* header, uint32_t sizeof_header)
   @brief Transmit a packet with header and payload to Socket::foreign.
   @param[in] payload  The payload of the packet to transmit.
   @param[in] sizeof_payload  The number of bytes in the payload to transmit.
   @param[in] header  The header of the packet to transmit.
   @param[in] sizeof_header The number of bytes in the header.
   @note The header is transmitted first, then the payload.


   @fn void* Socket::rcv(void* payload, uint32_t sizeof_payload, Endpoint* src = (Endpoint*)0)
   @brief Receive data from a socket.  Wait forever for data.
   @param[out] payload  A buffer in which to put the received data.  Must be at least sizeof_payload in length.
   @param[in] sizeof_payload The number of bytes of data to receive.
   @param[in] src A pointer to the specific Endpoint to listen to.  May be 0.
   @return A pointer to the received @b payload.


   @fn void* Socket::rcv(void* payload, uint32_t sizeof_payload, void* header, uint32_t sizeof_header, Endpoint* src = (Endpoint*)0)
   @brief Receive data from a socket where the payload and header are separated.  Wait forever for data.
   @param[out] payload  A buffer in which to put the received payload.  Must be at least sizeof_payload bytes in length.
   @param[in] sizeof_payload The number of bytes of payload to receive.
   @param[out] header A buffer in which to put the received header.  Must be at least sizeof_header bytes in length.
   @param[in] sizeof_header The number of bytes of header to receive.
   @param[in] src A pointer to the specific Endpoint to listen to.  May be 0.
   @return A pointer to the received @b header.
   @note The header is received first, then the payload.


   @fn void* Socket::rcv(void* payload, uint32_t sizeof_payload, uint32_t tmo, Endpoint* src = (Endpoint*)0)
   @brief Receive data from a socket.  Wait until the timeout and if no data, return 0.
   @param[out] payload  A buffer in which to put the received payload.  Must be at least sizeof_payload bytes in length.
   @param[in] sizeof_payload The number of bytes of payload to receive.
   @param[in] tmo Timeout in milliseconds.
   @param[in] src A pointer to the specific Endpoint to listen to.  May be 0.
   @return A pointer to the received @b payload, or 0 if there was a timeout.


   @fn void* Socket::rcv(void* payload, uint32_t sizeof_payload, void* header, uint32_t sizeof_header, uint32_t tmo)
   @brief Receive data from a socket where the payload and header are separated.  Wait until the timeout and if no data, return 0.
   @param[out] payload  A buffer in which to put the received payload.  Must be at least sizeof_payload bytes in length.
   @param[in] sizeof_payload The number of bytes of payload to receive.
   @param[out] header A buffer in which to put the received header.  Must be at least sizeof_header bytes in length.
   @param[in] sizeof_header The number of bytes of header to receive.
   @param[in] tmo Timeout in milliseconds.
   @return A pointer to the received @b header.
   @note The header is received first, then the payload.


   @fn void* Socket::rcvUpdate(void* payload, uint32_t sizeof_payload)
   @brief Receive data from the Socket::foreign endpoint.
   @param[out] payload  A buffer in which to put the received payload.  Must be at least sizeof_payload bytes in length.
   @param[in] sizeof_payload The number of bytes of payload to receive.
   @return A pointer to the received @b payload.


   @fn void* Socket::rcvUpdate(void* payload, uint32_t sizeof_payload, uint32_t tmo)
   @brief Receive data from the Socket::foreign endpoint and time out if data is not received.
   @param[out] payload  A buffer in which to put the received payload.  Must be at least sizeof_payload bytes in length.
   @param[in] sizeof_payload The number of bytes of payload to receive.
   @param[in] tmo Timeout in milliseconds.
   @return A pointer to the received @b payload, or 0 if there was a timeout.


   @fn int Socket::join(const Multicast& group)
   @brief Join a multicast group.
   @param[in] group  The multicast group to join.
   @return Zero (0) for success, otherwise the errno value which results from the underlying setsockopt call.


   @fn int Socket::drop(const Multicast& group)
   @brief Tell the socket to stop listening for datagrams from a specified multicast group.
   @param[in] group  The Multicast group to stop listening to.
   @return Zero (0) for success, otherwise the errno value which results
   from the underlying setsockopt call is returned.


   @fn void Socket::dump(int offset = 0) const
   @brief Pretty-print the Socket information to stdout.
   @param[in] offset  The amount to indent the output.


   @var Socket::local
   @brief The local Endpoint of the Socket.

   @var Socket::foreign
   @brief The foreign Endpoint of the Socket.


*/

}
}
