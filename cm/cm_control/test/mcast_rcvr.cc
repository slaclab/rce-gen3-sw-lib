#include <pthread.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>

using std::cin;

#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>

const unsigned MAX_O_SIZE = 2*1024;  // limit datagrams to 2kB
const int bufferSize = 32*1024;

int main(int argc,char** argv)
{
  const char* host = 0;
  struct in_addr groupAddr;
  struct in_addr interfaceAddr;
  int port = 0;
  int seconds = 0;

  extern char* optarg;
  //  extern int   optind;
  int c;
  while ( (c=getopt( argc, argv, "g:i:p:")) != EOF ) {
    switch(c) {
    case 'g': 
      host = optarg;
      if (!inet_aton(host,&groupAddr)) {
	printf("Error parsing group address\n");
	return -1;
      }
      break;
    case 'i': 
      host = optarg;
      if (!inet_aton(host,&interfaceAddr)) {
	printf("Error parsing interface address %s\n",host);
	return -1;
      }
      break;
    case 'p':
      port = atoi(optarg);
      break;
    }
  }
  if (!port || !host) {
    printf("%s -g <multicast addr> -i <interface addr> -p <port>\n",argv[0]);
    return -1;
  }

  //
  //  Open the socket
  //
  sockaddr_in _servaddr;
  _servaddr.sin_family      = AF_INET;
  _servaddr.sin_port        = htons(port);
  _servaddr.sin_addr.s_addr = groupAddr.s_addr;

  int _sockfd = -1;
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

  const int LINESZ = 1024;
  char line[LINESZ];

  printf("Commands: [j]oin, [r]esign, [^D]\n");
  do {
    cin.getline(line,LINESZ);

    switch(line[0]) {
    case 'j':
      {
	struct ip_mreq ipMreq;
	bzero ((char*)&ipMreq, sizeof(ipMreq));
	ipMreq.imr_multiaddr.s_addr = groupAddr.s_addr;
	ipMreq.imr_interface.s_addr = interfaceAddr.s_addr;
	int error_join = 
	  (setsockopt (_sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&ipMreq,
		       sizeof(ipMreq)) < 0) ? errno : 0; 
	if (error_join)
	  printf("*** error joining: %s\n", 
		 strerror(error_join));
      }
      break;
    case 'r':
      {
	struct ip_mreq ipMreq;
	bzero ((char*)&ipMreq, sizeof(ipMreq));
	ipMreq.imr_multiaddr.s_addr = groupAddr.s_addr;
	ipMreq.imr_interface.s_addr = interfaceAddr.s_addr;  
	int error_resign = (setsockopt (_sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&ipMreq,
					sizeof(ipMreq)) < 0) ? errno : 0;  
	if (error_resign) printf("*** odfMonServer::dtor error resign: %s\n",
				 strerror(error_resign));
      }
      break;
    default:
      break;
    }

  } while(cin.good());

  return 1;
}
