
#include "control/SocketTest.hh"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
//#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using namespace cm::control;

SocketTest::SocketTest(unsigned interface, unsigned group, unsigned short port)
{
  _groupAddr    .s_addr = group;
  _interfaceAddr.s_addr = interface;

  //
  //  Open the socket
  //
  sockaddr_in _servaddr;
  _servaddr.sin_family      = AF_INET;
  _servaddr.sin_port        = htons(port);
  _servaddr.sin_addr.s_addr = _groupAddr.s_addr;

  _sockfd = -1;
  if ( (_sockfd = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    printf("Error opening socket\n");
  else
    printf("Opened socket (%d)\n",_sockfd);

  int result = ::bind(_sockfd, (sockaddr*)&_servaddr, sizeof(_servaddr));
  if (result < 0)
    printf("Error binding RemoteConsole socket: %s\n", strerror(result));
  else {
    sockaddr_in name;
    socklen_t name_len=sizeof(name);
    getsockname(_sockfd,(sockaddr*)&name,&name_len);
    printf("Socket %d bound to %lu.%lu.%lu.%lu/%d (%d)\n",
	   _sockfd,
	   (name.sin_addr.s_addr>>24)>> 0,(name.sin_addr.s_addr<< 8)>>24,
	   (name.sin_addr.s_addr<<16)>>24,(name.sin_addr.s_addr<<24)>>24,
	   name.sin_port,(int)name_len);
  }
}

SocketTest::~SocketTest()
{
  if (_sockfd>=0)
    ::close(_sockfd);
}

int SocketTest::join()
{
	struct ip_mreq ipMreq;
	memset ((char*)&ipMreq, 0, sizeof(ipMreq));
	ipMreq.imr_multiaddr.s_addr = _groupAddr.s_addr;
	ipMreq.imr_interface.s_addr = _interfaceAddr.s_addr;
	int error_join =
	  (setsockopt (_sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&ipMreq,
		       sizeof(ipMreq)) < 0) ? errno : 0;
	if (error_join)
	  printf("*** error joining: %s\n",
		 strerror(error_join));
	return error_join;
}

int SocketTest::resign()
{
	struct ip_mreq ipMreq;
	memset ((char*)&ipMreq, 0, sizeof(ipMreq));
	ipMreq.imr_multiaddr.s_addr = _groupAddr.s_addr;
	ipMreq.imr_interface.s_addr = _interfaceAddr.s_addr;
	int error_resign = (setsockopt (_sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&ipMreq,
					sizeof(ipMreq)) < 0) ? errno : 0;
	if (error_resign) printf("*** error resign: %s\n",
				 strerror(error_resign));
	return error_resign;
}
