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
#include <time.h>
#include <list>

#include "atca/Attributes.hh"

#include "remote_service.hh"

using shelf::remote_service::Ack;
using shelf::remote_service::Command;
using shelf::remote_service::Tag;

static bool _verbose = false;

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

int _retry(int                skt, 
           const sockaddr_in& saddr,
           const Command&     cmd)
{
  int rval=0;
  if (::sendto(skt, &cmd, cmd._sizeof(), 0,
               (const sockaddr*)&saddr, sizeof(saddr))<0) {
    perror("send failed");
    return -1;
  }

  unsigned elements=cmd._elements;
  unsigned slot=1;
  while((cmd._slotmask&(1<<slot))==0) slot++;

  while(1) {
    struct pollfd pfd;
    pfd.fd      = skt;
    pfd.events  = POLLIN;
    pfd.revents = 0;
    int nfd=1;

    if (::poll(&pfd,nfd,1000)==0)
      break;
        
    Ack ack;
    sockaddr_in saddr;
    socklen_t   saddr_sz = sizeof(saddr);
    int nb = ::recvfrom(skt, &ack, sizeof(ack), 0,
                        (struct sockaddr*)&saddr, &saddr_sz);
    if (nb==sizeof(ack) &&
        ack._cmd._id == cmd._id) {
      if (_verbose)
        printf("received ack from %8.8s/%d/%d/%d [%x.%d]\n",
               ack._src._shelf,
               ack._src._slot,
               ack._src._bay,
               ack._src._element,
               ntohl(saddr.sin_addr.s_addr),
               ntohs(saddr.sin_port));
      unsigned index = ack._src._bay*Tag::nelements + ack._src._element;
      elements &= ~(1<<index);

      if (elements==0) break;
    }
  }
  if (elements) {
    printf("Retry failed for:\n");
    for(unsigned ielem=0; elements!=0; ielem++) 
      if (elements&(1<<ielem)) {
        elements&=~(1<<ielem);
        printf("%8.8s/%d/%d/%d\n", cmd._shelf, slot, ielem/Tag::nelements, ielem%Tag::nelements);
        rval=-1;
      }
  }
  return rval;
}

int _retry(int                skt, 
           const sockaddr_in& saddr,
           Command&           cmd,
           const std::list<unsigned>& dst)
{
  int rval=0;
  unsigned slot=0;
  unsigned elements=0;
  for(std::list<unsigned>::const_iterator it=dst.begin(); it!=dst.end(); it++) {
    unsigned islot = (*it)/(Tag::nbays*Tag::nelements);
    unsigned ielem = (*it)%(Tag::nbays*Tag::nelements);
    if (islot!=slot && elements!=0) {
      cmd._slotmask = 1<<slot;
      cmd._elements = elements;
      rval |= _retry(skt,saddr,cmd);
    }
    slot = islot;
    elements |= 1<<ielem;
  }
  if (elements!=0) {
    cmd._slotmask = 1<<slot;
    cmd._elements = elements;
    rval |= _retry(skt,saddr,cmd);
  }

  return rval;
}

int _send_command(int                skt, 
                  const sockaddr_in& saddr,
                  Command&           cmd)
{
  std::list<unsigned> nonacks;

  if (::sendto(skt, &cmd, cmd._sizeof(), 0,
               (const sockaddr*)&saddr, sizeof(saddr))<0) {
    perror("send failed");
    return -1;
  }

  uint32_t ack_mask[Tag::nslots+1];
  for(unsigned i=0; i<Tag::nslots; i++)
    ack_mask[i] = (cmd._slotmask&(1<<i)) ? cmd._elements : 0;

  while(1) {
    struct pollfd pfd;
    pfd.fd      = skt;
    pfd.events  = POLLIN;
    pfd.revents = 0;
    int nfd=1;

    if (::poll(&pfd,nfd,1000)==0)
      break;

    Ack ack;
    sockaddr_in saddr;
    socklen_t   saddr_sz = sizeof(saddr);
    int nb = ::recvfrom(skt, &ack, sizeof(ack), 0,
                        (struct sockaddr*)&saddr, &saddr_sz);
    if (nb==sizeof(ack) &&
        ack._cmd._id == cmd._id) {
      if (_verbose)
        printf("received ack from %8.8s/%d/%d/%d [%x.%d]\n",
               ack._src._shelf,
               ack._src._slot,
               ack._src._bay,
               ack._src._element,
               ntohl(saddr.sin_addr.s_addr),
               ntohs(saddr.sin_port));
      unsigned index = ack._src._bay*Tag::nelements + ack._src._element;
      ack_mask[ack._src._slot] &= ~(1<<index);

      unsigned mask=0;
      for(unsigned i=0; i<Tag::nslots; i++)
        mask |= ack_mask[i];
      
      if (mask==0)
        return 0;
    }
    else if (_verbose)
      printf("received nonack [size %d  id %d(%d)]\n",
             nb, ack._cmd._id, cmd._id);
  }
  
  if (cmd._type==Command::FileTransfer)
    printf("Command [%s,%llu,%u] failed to receive acks from:\n",
           cmd.fname(),cmd._offset,cmd._size);
  else
    printf("Command [%s] failed to receive acks from:\n",
           cmd.command());
  for(unsigned j=0; j<Tag::nslots; j++) {
    unsigned mask = ack_mask[j];
    for(int ibay=0; ibay<Tag::nbays; ibay++)
      for(int ielem,i=0; (ielem=_elem[ibay][i])>=0; i++)
        if (mask & (1<<(ibay*Tag::nelements+ielem))) {
          printf("%8.8s/%d/%d/%d\n", cmd._shelf, j, ibay, ielem);
          nonacks.push_back(ielem+Tag::nelements*(ibay+Tag::nbays*j));
        }
  }

  //
  //  retry
  //

  return _retry(skt, saddr, cmd, nonacks);
}

void show_usage(const char* p)
{
  printf("Usage: %s -s <shelf> -m <slotmask> [-i <if_name>] [-f <local_file>,<remote_file> | -c <command>] [-v]\n",p);
  exit(0);
}

int main(int argc, char** argv)
{
  const char* s_intf = "eth0";
  const char* shelf = 0;
  unsigned slotmask = ((1<<14)-1)<<1;
  const char* local_file=0;
  const char* remote_file=0;
  const char* command=0;

  unsigned elements=0;
  for(int ibay=0; ibay<Tag::nbays; ibay++)
    for(int ielem,i=0; (ielem=_elem[ibay][i])>=0; i++)
      elements |= 1<<(ibay*Tag::nelements+ielem);

  _verbose = false;

  int c;
  while ((c=getopt(argc,argv,"i:s:m:f:c:T?hv")) != -1) {
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
    case 'T':
      elements = (1<<16); // 1<<(dtm_bay*Tag::nelements+dtm_element)
      break;
    case 'f':
      local_file  = optarg;
      remote_file = strchr(optarg,',');
      if (!remote_file) {
        printf("<remote_file> not specified\n");
        show_usage(argv[0]);
      }
      else {
        *const_cast<char*>(remote_file++) = 0;
        if (strchr(remote_file,',')) {
          printf("Too many filenames in -f argument\n");
          show_usage(argv[0]);
        }
      }
      break;
    case 'c':
      command=optarg;
      break;
    case 'v': _verbose=true; break;
    default: show_usage(argv[0]); break;
    }
  }

  if (!shelf || !s_intf) {
    show_usage(argv[0]);
    return 0;
  }

  in_addr_t intf = get_interface(s_intf);
  int s = ucast_socket(0,intf);
  if (s < 0) {
    printf("Error creating reply socket\n");
    exit(2);
  }

  sockaddr_in saddr;
  saddr.sin_family      = AF_INET;
  saddr.sin_addr.s_addr = htonl(mcast);
  saddr.sin_port        = htons(port_remotesvc);

  struct timespec tv;
  clock_gettime(CLOCK_REALTIME,&tv);
  unsigned id = (tv.tv_nsec>>14)&0xffff;

  const size_t MTU=1470;
  //  const size_t MTU=1000;
  char* buff = new char[MTU];

  int rval=0;

  if (local_file) {

    FILE* f = fopen(local_file,"r");
    if (!f) {
      perror("Opening local file");
      exit(1);
    }

    size_t offset=0;
    size_t mtu   =(MTU-(sizeof(Command)+strlen(remote_file)+1))&~0x3;
    while(!feof(f)) {
      Command& cmd = *new(buff) Command(id,shelf,slotmask,elements,
                                        remote_file,offset);
      size_t sz = fread(cmd.payload(), 1, mtu, f);
      if (sz==0) break;
      if (_verbose)
        printf("sending filetransfer command [%s,%zu,%zu][%x,%x]\n",
               remote_file,offset,sz,slotmask,elements);
      cmd._size  = sz;
      offset    += sz;
      if (_send_command(s, saddr, cmd)<0) {
        printf("File transfer [%s -> %s] failed\n",
               local_file, remote_file);
        exit(1);
      }
    }

    fclose(f);
  }
  else if (command) {
    Command& cmd = *new(buff) Command(id,shelf,slotmask,elements,
                                      command);
    if (_send_command(s, saddr, cmd)<0) {
      printf("Remote command [%s] failed\n", command);
      exit(1);
    }
  }

  delete[] buff;

  return rval;
}
