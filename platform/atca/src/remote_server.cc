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

#include "atca/Attributes.hh"

#include "bsi/Bsi_Cfg.h"
#include "bsi/Bsi.h"
#include "map/Lookup.h"

#include "remote_service.hh"

using shelf::remote_service::Ack;
using shelf::remote_service::Command;
using shelf::remote_service::Tag;

static bool _verbose = false;

void show_usage(const char* p)
{
  printf("Usage: %s [-v]\n",p);
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

   sockaddr_in saddr;
   saddr.sin_family      = AF_INET;
   saddr.sin_addr.s_addr = htonl(mcast);
   saddr.sin_port        = htons(port);

   if (::bind(s, (struct sockaddr*) &saddr, sizeof(saddr) ) == -1) {
     perror("binding socket");
     return -1;
   }

#ifdef USE_MCAST
   struct ip_mreq ipMreq;
   memset((char*)&ipMreq, 0, sizeof(ipMreq));
   ipMreq.imr_multiaddr.s_addr = htonl(mcast);
   ipMreq.imr_interface.s_addr = htonl(interface);
   if (setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&ipMreq,
                  sizeof(ipMreq)) < 0 ) {
     perror("setsockopt(...IP_ADD_MEMBERSHIP) failed");
     return -1;
   }
#endif

   return s;
}

int main(int argc, char** argv)
{
  _verbose = false;

  const char* s_intf = "eth0";

  int c;
  while ((c=getopt(argc,argv,"i:?hv")) != -1) {
    switch(c) {
    case 'i': s_intf=optarg; break;
    case 'v': _verbose=true; break;
    default: show_usage(argv[0]); break;
    }
  }

  Bsi bsi = LookupBsi();
  const char *group;
  uint32_t addr;
  uint32_t cluster;
  uint32_t bay;
  uint32_t element;
    
  if(!bsi) return -1;
      
  char* buffer = new char[BSI_GROUP_NAME_SIZE * sizeof(unsigned)];
  group   = BsiReadGroup(bsi,buffer);
  addr    = BsiRead32(bsi,BSI_CLUSTER_ADDR_OFFSET);
  cluster = BSI_CLUSTER_FROM_CLUSTER_ADDR(addr);
  bay     = BSI_BAY_FROM_CLUSTER_ADDR(addr);
  element = BSI_ELEMENT_FROM_CLUSTER_ADDR(addr);
    
  service::atca::Attributes attr(group,cluster,bay,element);

  in_addr_t intf = get_interface(s_intf);

  int s = mcast_socket(mcast, intf, port_remotesvc);
  if (s < 0) {
    printf("Error creating mcast socket\n");
    exit(1);
  }

  int s_reply = ucast_socket(0,intf);
  if (s_reply < 0) {
    printf("Error creating reply socket\n");
    exit(2);
  }

  struct iovec  iov;
  iov.iov_base = new char[9000];
  iov.iov_len  = 9000;
  struct sockaddr_in from;
  socklen_t fromsz = sizeof(from);

  Tag src(attr);

  unsigned elemask = 1<<(src._bay*Tag::nelements+src._element);

  const Command& cmd = *reinterpret_cast<const Command*>(iov.iov_base);

  while(1) {
    ssize_t nb = ::recvfrom(s, iov.iov_base, iov.iov_len, 0, 
                            (struct sockaddr*)&from, &fromsz);

    if (cmd._slotmask&(1<<attr.cluster()) &&
        cmd._elements&elemask) {
      switch(cmd.type()) {
      case Command::FileTransfer:
        {
          FILE* f = fopen(cmd.fname(),"a");
          if (!f) {
            if (_verbose)
              perror("opening file");
            break;
          }
          
          //fpos_t pos;
          //fgetpos(f,&pos);
          long pos = ftell(f);
          if (cmd._offset==uint64_t(pos) &&
              fwrite(cmd.payload(),1,cmd._size,f)==cmd._size) {
            Ack ack(cmd,src);
            ::sendto(s_reply, &ack, sizeof(ack), 0,
                     (struct sockaddr*)&from, sizeof(from));
            if (_verbose && cmd._offset==0) {
              printf("opened file [%s]\n",cmd.fname());
            }
          }
          else if (_verbose) {
            printf("failed to write segment [%s,%llu,%u pos=%ld]\n",
                   cmd.fname(),cmd._offset,cmd._size,pos);
          }
          fclose(f);
        } break;
      case Command::ShellCommand:
        if (_verbose) 
          printf("Executing [%s] from %x.%d\n",
                 cmd.command(),
                 ntohl(from.sin_addr.s_addr),
                 ntohs(from.sin_port));
        if (::system(cmd.command())>=0) {
          Ack ack(cmd,src);
          ::sendto(s_reply, &ack, sizeof(ack), 0,
                   (struct sockaddr*)&from, sizeof(from));
        } 
        else if (_verbose)
          printf("Command [%s] failed\n",cmd.command());
        break;
      default:
        break;
      }
    }
    else if (_verbose)
      printf("received command %u bytes [%x,%x] from %x.%d\n",
             nb, cmd._slotmask, cmd._elements,
             ntohl(from.sin_addr.s_addr),
             ntohs(from.sin_port));
  }

  delete[] (char*)iov.iov_base;

  return 0;
}
