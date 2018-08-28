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
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>
#include <vector>

#include "shelf_ping.hh"

using shelf::ping::Command;
using shelf::ping::Response;
using shelf::ping::Results;
using shelf::ping::Report;
using shelf::ping::Service;
using shelf::ping::Tag;

static bool _verbose=false;

void show_usage(const char* p)
{
  printf("Usage: %s -i <interface name/ip> -s <shelf> -m <slotmask> [-r]\n"
         "Options: -r : fetch receive-side statistics\n",p);
}

unsigned get_results(int s,
                     unsigned slotmask,
                     Results* results)
{
  unsigned nslots=0;
  for(unsigned i=0, mask=slotmask; mask!=0; i++)
    if (mask&(1<<i)) {
      nslots++;
      mask&=~(1<<i);
    }

  unsigned nelem=0;
  for(int ibay=0; ibay<Tag::nbays; ibay++)
    for(int ielem,i=0; (ielem=_elem[ibay][i])>=0; i++,nelem++)
      ;

  nelem *= nslots;

  unsigned relem=0;

  char* rbuff = new char[2048];
  Response& response = *new(rbuff) Response;

  while(1) {
    struct pollfd pfd;
    pfd.fd      = s;
    pfd.events  = POLLIN;
    pfd.revents = 0;
    int nfds=1;
    int tmo = nslots*1000;
    int pq = ::poll(&pfd, nfds, tmo);
    if (pq==0) // timeout
      break;
    else if (pq<0)
      perror("poll");
    else {
      int nb = ::recv(s, rbuff, 2048, 0);
      if (nb>0) {
        const Tag& src = response._src;
        if (slotmask & (1<<src._slot)) {
          if (_verbose) {
            for(unsigned i=0; i<response._ndst; i++) {
              const Tag& dst = response.report(i)._dst;
              printf("%d/%d/%d - %d/%d/%d : %d\n",
                     src._slot, src._bay, src._element,
                     dst._slot, dst._bay, dst._element,
                     response.report(i)._n);
            }
          }

          unsigned index = (src._slot*Tag::nbays + src._bay)*Tag::nelements + src._element;
          *new(&results[index]) Results(response);
          if (++relem == nelem)
            break;
        }
      }
    }
  }

  delete[] rbuff;

  return relem;
}

unsigned get_results(int s,
                     const char* shelf,
                     unsigned slotmask,
                     Results* results)
{
  unsigned relem=0;

  unsigned nelem=0;
  for(int jbay=0; jbay<Tag::nbays; jbay++)
    for(int jelem,j=0; (jelem=_elem[jbay][j])>=0; j++)
      nelem++;

  sockaddr_in saddr;
  saddr.sin_family      = AF_INET;
  saddr.sin_addr.s_addr = htonl(mcast);
  saddr.sin_port        = htons(port_discover);

  Tag control(service::atca::Attributes(shelf,0,0,0));

  Service reset_srv;
  reset_srv._src = control;
  reset_srv._dst = Tag(shelf,slotmask);
  reset_srv._reply = 1;

  char* rbuff = new char[2048];
  Response& response = *new(rbuff) Response;

  for(int islot=1; slotmask!=0; islot=(islot+1)%Tag::nslots) {
    if (slotmask&(1<<islot)) {
      slotmask&=~(1<<islot);
      reset_srv._dst = Tag(shelf,1<<islot);
      if (_verbose)
        printf("query %x [%s/%x]\n",
               ntohl(saddr.sin_addr.s_addr),
               shelf, reset_srv._dst.slots());
      if (::sendto(s, &reset_srv, sizeof(reset_srv), 0, 
                   (struct sockaddr*)&saddr, sizeof(saddr))<0) {
        perror("Retrieve service sendto");
        printf("saddr %x.%d\n", ntohl(saddr.sin_addr.s_addr), ntohs(saddr.sin_port));
      }
      else {
        unsigned ielem=0;
        while(1) {
          struct pollfd pfd;
          pfd.fd      = s;
          pfd.events  = POLLIN;
          pfd.revents = 0;
          int nfds=1;
          int tmo = 100;

          if (::poll(&pfd, nfds, tmo)==0) {
            if (_verbose) printf("tmo\n");
            break;
          }

          int nb = ::recv(s, rbuff, 2048, 0);
          if (nb>0) {
            const Tag& src = response._src;
            if (_verbose) printf("recv report from %s/%d/%d/%d\n",
                                 src._shelf, src._slot, src._bay, src._element);
            if ((1<<src._slot) & reset_srv._dst.slots()) {
              if (_verbose) {
                for(unsigned i=0; i<response._ndst; i++) {
                  const Tag& dst = response.report(i)._dst;
                  printf("%d/%d/%d - %d/%d/%d : %d\n",
                         src._slot, src._bay, src._element,
                         dst._slot, dst._bay, dst._element,
                         response.report(i)._n);
                }
              }
              
              unsigned index = (src._slot*Tag::nbays + src._bay)*Tag::nelements + src._element;
              *new(&results[index]) Results(response);
              relem++;

              if (++ielem==nelem) 
                break;
            }
          }
        }
      }
    }
  }

  delete[] rbuff;

  return relem;
}

void dump_results(unsigned echoes,
                  unsigned slotmask,
                  const Results* rt,
                  const Results* rc)
{
  printf("Key: { N = No Response,\n"
         "       M = Multicast request failed,\n"
         "       m = Reply to multicast failed,\n"
         "       U = Unicast request failed,\n"
         "       u = unicast reply failed,\n"
         "       . = Success }\n");

  { char title0[256];
    char title1[256];
    char title2[256];
    char* t0 = title0;
    char* t1 = title1;
    char* t2 = title2;
    unsigned imask=slotmask;
    for(int is=0; imask!=0; is++) {
      if (!(imask&(1<<is))) continue;
      imask &= ~(1<<is);
      t0 += sprintf(t0,"    %2d    ",is);
      for(int ibay=0; ibay<Tag::nbays; ibay++) {
        int i=0;
        for(int ielem; (ielem=_elem[ibay][i])>=0; i++,t2++)
          sprintf(t2,"%1d",ielem);
        sprintf(t1,"%-*d",i,ibay);
        t1+=i;
      }
      *t1++ = ' ';
      *t2++ = ' ';
    }
    *t1++ = 0;
    *t2++ = 0;
    printf("%-12.12s%s\n","  Slot"       ,title0);
    printf("%-12.12s%s\n","  Bay"        ,title1);
    printf("%-12.12s%s\n"," Element"     ,title2);
  }

  unsigned imask=slotmask;
  for(int is=0; imask!=0; is++) {
    if (!(imask&(1<<is))) continue;
    imask &= ~(1<<is);
    for(int ibay=0; ibay<Tag::nbays; ibay++) {
      for(int ielem,i=0; (ielem=_elem[ibay][i])>=0; i++) {
        unsigned src_index = (is*Tag::nbays + ibay)*Tag::nelements + ielem;
        if (rt[src_index]._reply==0) {
          printf("%2d/%1d/%1d (N)\n",is,ibay,ielem);
        }
        else {
          printf("%2d/%1d/%1d :    ",is,ibay,ielem);
          unsigned jmask=slotmask;
          for(int js=0; jmask!=0; js++) {
            if (!(jmask&(1<<js))) continue;
            jmask &= ~(1<<js);
            for(int jbay=0; jbay<Tag::nbays; jbay++) {
              for(int jelem,j=0; (jelem=_elem[jbay][j])>=0; j++) {
                unsigned dst_index = (js*Tag::nbays + jbay)*Tag::nelements + jelem;
                unsigned vrt = rt[src_index]._result[dst_index];
                if (rc==0) {
                  if (vrt==-1U)
                    printf("M");
                  else if (vrt<echoes)
                    printf("U");
                  else
                    printf(".");
                }
                else if (rc[dst_index]._reply==0)
                  printf("N");
                else {
                  unsigned vrc = rc[dst_index]._result[src_index];
                  if (vrc==-1U)
                    printf("M");
                  else if (vrt==-1U)
                    printf("m");
                  else if (vrt<vrc)
                    printf("u");
                  else if (vrt<echoes)
                    printf("U");
                  else
                    printf(".");
                }
              }
            }
            printf(" ");
          }
          printf("\n");
        }
      }
    }
  }

  imask=slotmask;
  for(int is=0; imask!=0; is++) {
    if (!(imask&(1<<is))) continue;
    imask &= ~(1<<is);
    for(int ibay=0; ibay<Tag::nbays; ibay++) {
      for(int ielem,i=0; (ielem=_elem[ibay][i])>=0; i++) {
        unsigned src_index = (is*Tag::nbays + ibay)*Tag::nelements + ielem;
        if (rt[src_index]._reply==0) {
        }
        else {
          unsigned jmask=slotmask;
          for(int js=0; jmask!=0; js++) {
            if (!(jmask&(1<<js))) continue;
            jmask &= ~(1<<js);
            for(int jbay=0; jbay<Tag::nbays; jbay++) {
              for(int jelem,j=0; (jelem=_elem[jbay][j])>=0; j++) {
                unsigned dst_index = (js*Tag::nbays + jbay)*Tag::nelements + jelem;
                unsigned vrt = rt[src_index]._result[dst_index];
                if (rc==0) {
                  if (vrt==-1U)
                    ;
                  else if (vrt<echoes)
                    printf("%2d/%1d/%1d -> %2d/%1d/%1d -> %2d/%1d/%1d [%d]\n",
                           is,ibay,ielem,
                           js,jbay,jelem,
                           is,ibay,ielem,
                           vrt);
                  else
                    ;
                }
                else if (rc[dst_index]._reply==0)
                  ;
                else {
                  unsigned vrc = rc[dst_index]._result[src_index];
                  if (vrc==-1U)
                    printf("%2d/%1d/%1d -> %2d/%1d/%1d [?] -> %2d/%1d/%1d [%d]\n",
                           is,ibay,ielem,
                           js,jbay,jelem,
                           is,ibay,ielem,
                           vrt);
                  else if (vrt==-1U)
                    printf("%2d/%1d/%1d -> %2d/%1d/%1d [%d] -> %2d/%1d/%1d [?]\n",
                           is,ibay,ielem,
                           js,jbay,jelem,
                           vrc,
                           is,ibay,ielem);
                  else if (vrt!=echoes || vrc!=echoes)
                    printf("%2d/%1d/%1d -> %2d/%1d/%1d [%d] -> %2d/%1d/%1d [%d]\n",
                           is,ibay,ielem,
                           js,jbay,jelem,
                           vrc,
                           is,ibay,ielem,
                           vrt);
                  else
                    ;
                }
              }
            }
          }
        }
      }
    }
  }
}

int ucast_socket(unsigned short port, const char* s_intf)
{
  int s=::socket(AF_INET, SOCK_DGRAM, 0);
  if (s==-1) {
    perror("Opening socket");
    return -1;
  }

  in_addr_t intf = 0;
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
  int recv_bytes = n*(n*sizeof(Report)+sizeof(Response));
  if(setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char*)&recv_bytes, sizeof(recv_bytes)) == -1) {
    perror("Setting SO_RCVBUF");
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

  return s;
}

std::vector<unsigned> get_node_ips(int s, const char* shelf, unsigned mask)
{
  Tag control(service::atca::Attributes("",0,0,0));

  std::vector<unsigned> node_ip(Tag::nslots*Tag::nbays*Tag::nelements);
  for(int islot=1; mask!=0; islot=(islot+1)&0x1f) {
    if (!(mask&(1<<islot))) continue;
    mask &= ~(1<<islot);
    unsigned bays = (1<<Tag::nbays)-1;
    for(int ibay=0; bays!=0; ibay=(ibay+1)%Tag::nbays) {
      bays &= ~(1<<ibay);
      for(int ielem,i=0; (ielem=_elem[ibay][i])>=0; i++) {
        unsigned kelem = ielem + Tag::nelements*(ibay + Tag::nbays*islot);
        service::atca::Attributes dst(shelf,
                                      islot,
                                      ibay,
                                      ielem);
        // send mcast
        struct sockaddr_in saddr;
        saddr.sin_family      = AF_INET;
        saddr.sin_addr.s_addr = htonl(mcast);
        saddr.sin_port        = htons(port_discover); 
        Service svc;
        svc._src = control;
        svc._dst = Tag(dst);
        svc._reply = 1;
          
        if (::sendto(s, &svc, sizeof(svc), 0, (struct sockaddr*)&saddr, sizeof(saddr))<0) {
          perror("sendto");
        }
          
        // wait for reply
        struct pollfd pfd;
        pfd.fd      = s;
        pfd.events  = POLLIN;
        pfd.revents = 0;
        int nfd=1;

        if (::poll(&pfd,nfd,100)==0) {
          printf("Failed to get a response from %s/%d/%d/%d\n",
                 shelf,islot,ibay,ielem);
          node_ip[kelem] = 0;
        }
        else {
          Service reply;
          struct sockaddr_in echo_to;
          socklen_t echo_tosz = sizeof(echo_to);
            
          int nb = ::recvfrom(s, &reply, sizeof(reply), 0,
                              (struct sockaddr*)&echo_to, &echo_tosz);
          if (nb==sizeof(reply)) {
            if (reply._src == dst &&
                reply._dst == control) {
              node_ip[kelem] = ntohl(echo_to.sin_addr.s_addr);
              if (_verbose)
                printf("get_node_ip[%d](%d/%d/%d) = %x\n",
                       kelem, islot, ibay, ielem,
                       ntohl(echo_to.sin_addr.s_addr));
            }
          }
        }
      }
    }
  }
  return node_ip;
}

int main(int argc, char** argv)
{
  const char* shelf = 0;
  const char* s_intf = "eth0";
  unsigned slotmask = ((1<<14)-1)<<1;
  unsigned echoes = 0;
  bool lfetch_recv_stats=false;
  _verbose=false;

  int c;
  while ((c=getopt(argc,argv,"?hi:s:m:e:rv")) != -1) {
    switch(c) {
    case 'i':
      s_intf = optarg;
      break;
    case 's':
      shelf = optarg;
      break;
    case 'm':
      slotmask = strtoul(optarg,NULL,0);
      break;
    case 'e':
      echoes = strtoul(optarg,NULL,0);
      break;
    case 'r':
      lfetch_recv_stats=true;
      break;
    case 'v':
      _verbose=true;
      break;
    default:
      show_usage(argv[0]);
      exit(0);
    }
  }

  if (!shelf || !s_intf) {
    show_usage(argv[0]);
    return 0;
  }

  int s = ucast_socket(0,s_intf);
  if (s<0)
    return -1;

  Tag control(service::atca::Attributes(shelf,0,0,0));

  sockaddr_in saddr;
  saddr.sin_family      = AF_INET;
  saddr.sin_addr.s_addr = htonl(mcast);

  Service reset_srv;
  reset_srv._src = control;
  reset_srv._dst = Tag(shelf,slotmask);

  std::vector<unsigned> node_ip;

  if (lfetch_recv_stats) {
    //
    //  First reset the receiver statistics
    //
    saddr.sin_port        = htons(port_discover);

    reset_srv._reply =  0;
    if (::sendto(s, &reset_srv, sizeof(reset_srv), 0, 
                 (struct sockaddr*)&saddr, sizeof(saddr))<0) {
      perror("Reset service sendto");
      printf("saddr %x.%d\n", ntohl(saddr.sin_addr.s_addr), ntohs(saddr.sin_port));
    }

    node_ip = get_node_ips(s, shelf, slotmask);

    //
    //  Confirm reset
    //
    {
      Results* confirm = new Results[Tag::nslots*Tag::nbays*Tag::nelements];
      unsigned q=get_results (s,slotmask,confirm);
      //      dump_results(echoes,slotmask,confirm,0);
      //      printf("Reset %d elements\n",q);
      delete[] confirm; 
    }
  }

  //
  //  Command all nodes to begin interconnect tests
  //
  saddr.sin_port        = htons(port_command);

  Command cmd(shelf,slotmask,echoes);
  if (::sendto(s, &cmd, sizeof(cmd), 0, 
               (struct sockaddr*)&saddr, sizeof(saddr))<0) {
    perror("Command sendto");
    printf("saddr %x.%d\n", ntohl(saddr.sin_addr.s_addr), ntohs(saddr.sin_port));
  }

  //
  //  Retrieve roundtrip statistics
  //
  Results* rt = new Results[Tag::nslots*Tag::nbays*Tag::nelements];
  { unsigned q=get_results (s,slotmask,rt);
    if (_verbose)
      printf("Received %d roundtrip reports\n",q);
    if (!lfetch_recv_stats)
      dump_results(echoes,slotmask,rt,0);
  }

  //
  //  Retrieve receiver statistics
  //
  if (lfetch_recv_stats) {
    /**
    saddr.sin_port   = htons(port_discover);
    reset_srv._reply =  1;
    if (::sendto(s, &reset_srv, sizeof(reset_srv), 0, 
                 (struct sockaddr*)&saddr, sizeof(saddr))<0) {
      perror("Reset service sendto");
      printf("saddr %x.%d\n", ntohl(saddr.sin_addr.s_addr), ntohs(saddr.sin_port));
    }

    Results* rc = new Results[Tag::nslots*Tag::nbays*Tag::nelements];
    { unsigned q=get_results (s,slotmask,rc);
      printf("Received %d receiver reports\n",q);
    }
    **/
    Results* rc = new Results[Tag::nslots*Tag::nbays*Tag::nelements];
    { unsigned q=get_results (s,shelf,slotmask,rc);
      if (_verbose) 
        printf("Received %d receiver reports\n",q);
    }
    dump_results(echoes,slotmask,rt,rc);
  }

  delete[] rt;

  return 0;
}
