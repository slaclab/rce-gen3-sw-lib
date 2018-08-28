/*
** ++
**  Package:
**	
**  Abstract:
**
**  Author:
**      Matt Weaver, SLAC <weaver@slac.stanford.edu>
**
**  Creation Date:
**	000 - , 2014
**
**  Revision History:
**	None.
**
** --
*/



#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <pthread.h>

#include "atca/Attributes.hh"

#include "bsi/Bsi_Cfg.h"
#include "bsi/Bsi.h"
#include "map/Lookup.h"

#include "shelf_ping.hh"

using shelf::ping::Command;
using shelf::ping::Tag;
using shelf::ping::Response;
using shelf::ping::Report;
using shelf::ping::Service;

static service::atca::Attributes* attr = 0;
static int _verbose = 0;

void show_usage(const char* p)
{
  printf("Usage: %s -i <interface name/ip> -s <shelf> <slots>\n",p);
}

in_addr_t get_interface(const char* s_intf)
{
  in_addr_t intf = 0;
  int s=::socket(AF_INET, SOCK_DGRAM, 0);
  if (s==-1) {
    perror("Opening socket");
    return intf;
  }

  if (s_intf[0]<'0' || s_intf[0]>'9') {
    struct ifreq ifr;
    strcpy( ifr.ifr_name, s_intf );
    if (ioctl( s, SIOCGIFADDR, (char*)&ifr )==0)
      intf = ntohl( *(unsigned int*) &(ifr.ifr_addr.sa_data[2]) );
    else
      perror("Failed to get ip address for interface");
  }
  else {
    intf = ntohl(inet_addr(s_intf));
  }
  ::close(s);
  return intf;
}

int ucast_socket(unsigned short port, in_addr_t intf)
{
  int s=::socket(AF_INET, SOCK_DGRAM, 0);
  if (s==-1) {
    perror("Opening socket");
    return -1;
  }

  sockaddr_in saddr;
  saddr.sin_family      = AF_INET;
  saddr.sin_addr.s_addr = ntohl(intf);

  if (port!=0) {
    saddr.sin_port        = htons(port);

    if (::bind(s, (struct sockaddr*) &saddr, sizeof(saddr) ) == -1) {
      perror("binding socket");
      return -1;
    }
  }
  else { // ephemeral port
    port = 32768;
    while(1) {
      saddr.sin_port        = htons(port);
      if (::bind(s, (struct sockaddr*) &saddr, sizeof(saddr) ) == -1) {
        port++;
      }
      else
        break;
    }
  }

  socklen_t saddr_len = sizeof(saddr);
  getsockname(s,(sockaddr*)&saddr,&saddr_len);
  printf("ucast_socket %d bound to %x.%d\n",
         s, ntohl(saddr.sin_addr.s_addr), ntohs(saddr.sin_port));

  return s;
}

int mcast_socket(unsigned mcast, unsigned interface, unsigned short port)
{
  int s=::socket(AF_INET, SOCK_DGRAM, 0);
  if (s==-1) {
    perror("Opening socket");
    return -1;
  }

   int iYes = 1;
   if(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&iYes, sizeof(iYes)) == -1) {
     perror("Setting reuseaddr on socket");
     return -1;
   }

   if(setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char*)&iYes, sizeof(iYes)) == -1) {
     perror("Enabling broadcast");
     return -1;
   }

   unsigned n = Tag::nslots*Tag::nbays*Tag::nelements;
   int recv_bytes = n*(n*sizeof(Report)+sizeof(Report));
   if(setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char*)&recv_bytes, sizeof(recv_bytes)) == -1) {
     perror("Setting SO_RCVBUF");
     return -1;
   }

   sockaddr_in saddr;
   saddr.sin_family      = AF_INET;
   saddr.sin_addr.s_addr = htonl(mcast);
   saddr.sin_port        = htons(port);

   if (::bind(s, (struct sockaddr*) &saddr, sizeof(saddr) ) == -1) {
     perror("binding socket");
     return -1;
   }

   struct ip_mreq ipMreq;
   memset((char*)&ipMreq, 0, sizeof(ipMreq));
   ipMreq.imr_multiaddr.s_addr = htonl(mcast);
   ipMreq.imr_interface.s_addr = htonl(interface);
   if (setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&ipMreq,
                  sizeof(ipMreq)) < 0 ) {
     perror("setsockopt(...IP_ADD_MEMBERSHIP) failed");
     return -1;
   }

   return s;
}

static void* cmd_thread(void* arg)
{
  in_addr_t intf = get_interface("eth0");

  int s = mcast_socket(mcast, intf, port_command);
  if (s < 0) {
    printf("Error creating command socket\n");
    exit(1);
  }

  int s_reply = ucast_socket(0,intf);
  if (s_reply < 0) {
    printf("Error creating reply socket\n");
    exit(2);
  }

  struct iovec  iov;
  iov.iov_base = new char[1024];
  iov.iov_len  = 1024;
  struct sockaddr_in from;
  socklen_t fromsz = sizeof(from);

  char* rbuff = new char[2048];

  Tag src(*attr);

  while(1) {
    ssize_t nb = ::recvfrom(s, iov.iov_base, iov.iov_len, 0, 
                            (struct sockaddr*)&from, &fromsz);

    if (nb == sizeof(Command)) {
      const Command& cmd = *reinterpret_cast<const Command*>(iov.iov_base);

      if (_verbose>0)
        printf("received command for slotmask 0x%x [%d]\n",cmd._slotmask,src._slot);

      if (cmd._slotmask&(1<<src._slot)) {
        // execute command

        Response& response = *new (rbuff) Response(src);

        unsigned mask = cmd._slotmask;
        for(int islot=attr->cluster(); mask!=0; islot=(islot+1)&0x1f) {
          if (!(mask&(1<<islot))) continue;
          mask &= ~(1<<islot);
          unsigned bays = (1<<Tag::nbays)-1;
          for(int ibay=attr->bay(); bays!=0; ibay=(ibay+1)%Tag::nbays) {
            bays &= ~(1<<ibay);
            for(int ielem,i=0; (ielem=_elem[ibay][i])>=0; i++) {
              service::atca::Attributes dst(attr->group(),
                                            islot,
                                            ibay,
                                            ielem);
              // send mcast
              struct sockaddr_in saddr;
              saddr.sin_family      = AF_INET;
              saddr.sin_addr.s_addr = htonl(mcast);
              saddr.sin_port        = htons(port_discover); 
              Service svc;
              svc._src = src;
              svc._dst = Tag(dst);
              svc._reply = 1;

              if (_verbose>1)
                printf("send mcast %x.%d for %s/%d/%d/%d\n",
                       mcast,port_discover,
                       dst.group(),
                       dst.cluster(),
                       dst.bay(),
                       dst.element());

              if (::sendto(s_reply, &svc, sizeof(svc), 0, (struct sockaddr*)&saddr, sizeof(saddr))<0) {
                perror("sendto");
              }

              unsigned nresponse=-1U;
              unsigned echoes=0;

              Service reply;
              struct sockaddr_in echo_to;
              socklen_t echo_tosz = sizeof(echo_to);
                  
              while(1) {
                // wait for reply
                struct pollfd pfd;
                pfd.fd      = s_reply;
                pfd.events  = POLLIN;
                pfd.revents = 0;
                int nfd=1;
            
                int qp=::poll(&pfd,nfd,10);
                if (qp==0 && echoes==0)  // skip this node if no initial response
                  break;

                if (qp>0) {
                  int nb = ::recvfrom(s_reply, &reply, sizeof(reply), 0,
                                      (struct sockaddr*)&echo_to, &echo_tosz);
                  if (nb==sizeof(reply)) {
                    if (reply._src == svc._dst &&
                        reply._dst == svc._src) {
                      
                      if (_verbose>2)
                        printf("received reply from %s/%d/%d/%d\n",
                               reply._src._shelf,
                               reply._src._slot,
                               reply._src._bay,
                               reply._src._element);
                      
                      nresponse++;
                    }
                    else if (_verbose>0) {
                      printf("received reply destined for %s/%d/%d/%d\n",
                             reply._dst._shelf,
                             reply._dst._slot,
                             reply._dst._bay,
                             reply._dst._element);
                    }
                  }
                  else if (_verbose>0) {
                    printf("received unexpected datagram size %d on cmd socket\n",
                           nb);
                  }
                }

                if (echoes >= cmd._echoes) 
                  break;

                echoes++;

                if (::sendto(s_reply, &svc, sizeof(svc), 0, (struct sockaddr*)&echo_to, sizeof(echo_to))<0) {
                  perror("sendto");
                }    
              }
              response.append(Report(svc._dst,nresponse));
            }
          }
        }
        ::sendto(s, &response, response._sizeof(), 0, 
                 (struct sockaddr*)&from, fromsz);
      }
    }
    else if (nb > 0) {
      printf("received command of unexpected size(%zd)\n",nb);
    }
  }
}

static void* srv_thread(void* arg)
{
  in_addr_t intf = get_interface("eth0");
  int s = mcast_socket(mcast, intf, port_discover);
  if (s < 0) {
    printf("Error creating service socket\n");
    exit(3);
  }

  int s_u = ucast_socket(0,intf);
  if (s_u < 0) {
    printf("Error creating service socket\n");
    exit(3);
  }

  struct iovec  iov;
  iov.iov_base = new char[1024];
  iov.iov_len  = 1024;
  struct sockaddr_in from;
  socklen_t fromsz = sizeof(from);

  Service reply;
  reply._src = Tag(*attr);
  reply._reply = 0;

  Tag control(service::atca::Attributes(attr->group(),0,0,0));
  char* rbuff = new char[2048];
  Response* response = new (rbuff) Response(reply._src);

  unsigned slots=0;

  while(1) {
    struct pollfd pfd[2];
    pfd[0].fd      = s;
    pfd[0].events  = POLLIN;
    pfd[0].revents = 0;
    pfd[1].fd      = s_u;
    pfd[1].events  = POLLIN;
    pfd[1].revents = 0;
    int nfds=2;
    if (::poll(pfd,nfds,-1)>0) {
      for(int i=0; i<2; i++) {
        if (pfd[i].revents&POLLIN) {
          int nb = ::recvfrom(pfd[i].fd, iov.iov_base, iov.iov_len, 0,
                              (struct sockaddr*)&from, &fromsz);
          const Service& request = *reinterpret_cast<const Service*>(iov.iov_base);
          if (nb==sizeof(Service)) {
            if (_verbose>1)
              printf("received service from %s/%d/%d/%d for %s/%d/%d/%d [0x%x]\n",
                     request._src._shelf,
                     request._src._slot,
                     request._src._bay,
                     request._src._element,
                     request._dst._shelf,
                     request._dst._slot,
                     request._dst._bay,
                     request._dst._element,
                     request._dst.slots());

            if (request._src == control &&
                (request._dst.slots()&(1<<reply._src._slot))) {
              if (_verbose>0) {
                printf("%s request for slots %x from %x.%d: ndst %d\n",
                       request._reply ? "Report":"Reset",
                       request._dst.slots(),
                       ntohl(from.sin_addr.s_addr),
                       ntohs(from.sin_port),
                       response->_ndst);
                for(unsigned i=0; i<response->_ndst; i++) {
                  const Report& r = response->report(i);
                  if (request._dst.slots()&(1<<r._dst._slot))
                    printf("%d/%d/%d : %d\n",
                           r._dst._slot,
                           r._dst._bay,
                           r._dst._element,
                           r._n);
                }
              }
              if (request._reply==0) {
                slots = request._dst.slots();
                response = new (rbuff) Response(reply._src);
              }
              else
                ::sendto(s_u, response, response->_sizeof(), 0, 
                         (struct sockaddr*)&from, fromsz);
            }
            else if (request._dst == reply._src &&
                     request._reply != 0) {
              reply._dst = request._src;
              if (::sendto(s_u, &reply, sizeof(reply), 0, 
                         (struct sockaddr*)&from, fromsz)<0)
                perror("srvthread sendto");
              else if (i==0) {
                if (slots&(1<<request._src._slot)) {
                  if (_verbose>0)
                    printf("New report for %d/%d/%d from %x.%d\n",
                           request._src._slot,
                           request._src._bay,
                           request._src._element,
                           ntohl(from.sin_addr.s_addr),
                           ntohs(from.sin_port));
                  Report r(request._src,0);
                  response->append(r);
                }
              }
              else {
                bool lfound=false;
                for(int hint=response->_ndst-1; hint>=0; hint--) {
                  Report& r = const_cast<Report&>(response->report(hint));
                  if (r._dst == request._src) {
                    r._n++;
                    lfound=true;
                    break;
                  }
                }
                if (!lfound)
                  printf("received unexpected request from %s/%d/%d/%d on ucast socket\n",
                         request._src._shelf,
                         request._src._slot,
                         request._src._bay,
                         request._src._element);
              }
            }
            else if (i==0)
              ;
            else {
              printf("received unexpected request for %s/%d/%d/%d on ucast socket\n",
                     request._dst._shelf,
                     request._dst._slot,
                     request._dst._bay,
                     request._dst._element);
            }
          }
        }
      }
    }
  }
  return 0;
}

int main(int argc, char** argv)
{
  _verbose = 0;

  int c;
  while ((c=getopt(argc,argv,"?hv")) != -1) {
    switch(c) {
    case 'v': _verbose++; break;
    default: break;
    }
  }

  { Bsi bsi = LookupBsi();
    const char *group;
    uint32_t addr;
    uint32_t cluster;
    uint32_t bay;
    uint32_t element;
    
    if(!bsi) return -1;
      
    char* buffer = new char[BSI_GROUP_NAME_SIZE * sizeof(unsigned)];
    group = BsiReadGroup(bsi,buffer);
    addr = BsiRead32(bsi,BSI_CLUSTER_ADDR_OFFSET);
    cluster = BSI_CLUSTER_FROM_CLUSTER_ADDR(addr);
    bay = BSI_BAY_FROM_CLUSTER_ADDR(addr);
    element = BSI_ELEMENT_FROM_CLUSTER_ADDR(addr);
    
    attr = new service::atca::Attributes(group,cluster,bay,element);
  }

  pthread_t threadId;
  pthread_create(&threadId,NULL,cmd_thread,NULL);

  srv_thread(NULL);

  return 0;
}
