#include "pds/cm_svc/SysClk.hh"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>

using std::cin;
using Pds::SysClk;

#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>

const unsigned bufferSize = 6*1024;

struct thread_args {
  int sockfd;
  sockaddr_in addr;
};

const unsigned ReportStep=10000;

static unsigned report(unsigned npkt, unsigned dropped, 
		       unsigned last)
{
  unsigned curr;
  if ((npkt%ReportStep)==0) {
    curr = SysClk::sample();
    unsigned diff = curr > last ? curr - last : curr + 1000000000 - last;
    double delta = diff*SysClk::nsPerTick()*1.e-9;
    printf("%8d [%8d] : %g s : %g Hz\n",
	   npkt,dropped,delta,double(ReportStep)/delta);
  }
  else
    curr = last;
  return curr;
}

void *send(void *arg)
{
  struct thread_args* _thread_args = (struct thread_args*)arg;

  unsigned char* buff = new unsigned char[bufferSize];
  for(unsigned k=0; k<bufferSize; k++)
    buff[k] = k&0xff;

  timespec tv;
  tv.tv_sec = 0;
  tv.tv_nsec = 10000;

  int npkt=0;
  unsigned last = SysClk::sample();
  while(1) {						
    buff[0] = (npkt++)&0xff;
    int nbytes = sendto(_thread_args->sockfd,buff,bufferSize,
			0,reinterpret_cast<const sockaddr*>(&_thread_args->addr),sizeof(sockaddr_in));
    if (nbytes<0)
      printf("Error sending: %s\n",strerror(nbytes));
    last = report(npkt,0,last);
    if (npkt%500 == 0)
      nanosleep(&tv,0);
  }
  delete[] buff;
}

void *receive(void *arg)
{
  struct thread_args* _thread_args = (struct thread_args*)arg;

  unsigned char* buff = new unsigned char[bufferSize];
  unsigned npkt=0;
  unsigned char ipkt=0;
  unsigned last = SysClk::sample();
  unsigned dropped=0;
  while(1) {
    int nbytes = recvfrom(_thread_args->sockfd,buff,bufferSize,0,0,0);
    if (nbytes<0)
      printf("Error in recvfrom\n");
    else if (nbytes!=bufferSize)
      printf("Recvd %d bytes\n",nbytes);
    else if (buff[0] != ipkt) {
      //      printf("Found pkt %x [%x]\n",buff[0],ipkt);
      ipkt = buff[0];
      dropped++;
      npkt++;
    }
    else {
//       for(unsigned k=1; k<bufferSize; k++)
// 	if (buff[k] != (k&0xff)) {
// 	  printf("byte %x  found %x (%x)\n",k,buff[k],k&0xff);
// 	  break;
// 	}
      npkt++;
    }
    ipkt++;
    last = report(npkt,dropped,last);
  }
}

int main(int argc,char** argv)
{
  const char* host = 0;
  struct in_addr groupAddr;
  struct in_addr interfaceAddr;
  int port = 0;
  int seconds = 0;
  bool receiver=false;
  bool sender  =false;

  extern char* optarg;
  //  extern int   optind;
  int c;
  while ( (c=getopt( argc, argv, "rsg:i:p:")) != EOF ) {
    switch(c) {
    case 'r':
      receiver = true; 
      break;
    case 's':
      sender = true; 
      break;
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
  if (!port || !host || (receiver == sender)) {
    printf("%s (-s | -r) -g <multicast addr> -i <interface addr> -p <port>\n",argv[0]);
    return -1;
  }

  //
  //  Open the socket
  //
  sockaddr_in _servaddr;
  _servaddr.sin_family      = AF_INET;
  _servaddr.sin_port        = htons(port);
  _servaddr.sin_addr.s_addr = sender ? interfaceAddr.s_addr : groupAddr.s_addr;

  int _sockfd = -1;
  if ( (_sockfd = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    printf("Error opening socket\n");
  else
    printf("Opened socket (%d)\n",_sockfd);

  int result = ::bind(_sockfd, (sockaddr*)&_servaddr, sizeof(_servaddr));
  if (result < 0)
    printf("Error binding socket: %s\n", strerror(-result));
  else {
    sockaddr_in name;
    socklen_t name_len=sizeof(name);
    getsockname(_sockfd,(sockaddr*)&name,&name_len); 
    printf("Socket %d bound to %lu.%lu.%lu.%lu/%d (%d)\n",
	   _sockfd,
	   (name.sin_addr.s_addr>>24)>> 0,(name.sin_addr.s_addr<< 8)>>24,
	   (name.sin_addr.s_addr<<16)>>24,(name.sin_addr.s_addr<<24)>>24,
	   name.sin_port,(int)name_len);

    int parm = 4*1024*1024;
    if(setsockopt(_sockfd, SOL_SOCKET, sender ? SO_SNDBUF : SO_RCVBUF, (char*)&parm, sizeof(parm)) == -1)
      printf("Error setting sobuf : %s\n",strerror(errno));
  }

  if (groupAddr.s_addr != interfaceAddr.s_addr && receiver) {
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

  _servaddr.sin_addr.s_addr = groupAddr.s_addr;
  struct thread_args _thread_args;
  _thread_args.sockfd = _sockfd;
  _thread_args.addr   = _servaddr;
  pthread_t th2;
  if (sender)
    pthread_create(&th2, NULL, send   , (void*)&_thread_args);
  else
    pthread_create(&th2, NULL, receive, (void*)&_thread_args);

  const int LINESZ = 1024;
  char line[LINESZ];

  do {
    cin.getline(line,LINESZ);
  } while(cin.good());

  if (groupAddr.s_addr != interfaceAddr.s_addr && receiver) {
    struct ip_mreq ipMreq;
    bzero ((char*)&ipMreq, sizeof(ipMreq));
    ipMreq.imr_multiaddr.s_addr = groupAddr.s_addr;
    ipMreq.imr_interface.s_addr = interfaceAddr.s_addr;  
    int error_resign = (setsockopt (_sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&ipMreq,
				    sizeof(ipMreq)) < 0) ? errno : 0;  
    if (error_resign) printf("*** odfMonServer::dtor error resign: %s\n",
			     strerror(error_resign));
  }

  return 1;
}
