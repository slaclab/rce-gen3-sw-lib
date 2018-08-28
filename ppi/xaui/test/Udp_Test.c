/*
** ++
**  Package: XAUI
**	
**
**  Abstract: XAUI Test Driver
**
**  Implementation of XAUI test driver. See the corresponding header (".h") file for a
**  more complete description of the functions implemented below.      
**
**  Author:
**      S. Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	000 - August 19, 2014
**
**  Revision History:
**	None.
**
** --
*/

#include <stdio.h>   // printf...
#include <stdlib.h>

#define __need_getopt_newlib
#include <getopt.h>

#include <rtems/shell.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#include "xaui/Xaui.h"
#include "conversion/impl/BSWP.ih"

#define XAUI_HDR_SIZE 128
#define BUFLEN 8*1024
#define SRC_PORT 1021
#define DST_PORT 9930
#define DATA_LEN 1462

#define LOOPBACK_NAME "lo"
 
#include "../src/Pmu.h"

/*
**
*/
    
typedef struct  __attribute__((__packed__)) {
  uint8_t   version;
  uint8_t   tos;      // Type Of Service
  uint16_t  length;   // Total datagram length;
  uint16_t  id;
  uint16_t  offset;
  uint8_t   ttl;
  uint8_t   protocol;
  uint16_t  checksum;
  uint32_t  src;
  uint32_t  dst;
  } IpHeader;

/*
**
*/
  
typedef struct {
  uint16_t src;
  uint16_t dst;
  uint16_t length;   // Total datagram length;
  uint16_t checksum;
  } UdpHeader;
  
/*
**
*/

typedef struct {
 Xaui_Header ethernet;
 IpHeader    ip;
 UdpHeader   udp;
 uint32_t    mbz;
 } UdpPacket;

char server_ip[32];

uint32_t client_ip;
uint32_t buf_len;

Xaui_TxDevice tx_device;

static volatile uint32_t *gtc = (uint32_t*)GTC_REGISTER;

uint64_t                 dma_st; 
uint64_t                 dma_et; 
uint64_t                 dma_elp;
uint64_t                 dma_cnt;

uint64_t _mac;

/*
** ++
**
**
** --
*/

const char USAGE_UDP[] =
  "\n  Udp Network Test\n\
  Usage: udp_test [OPTIONS] [args]\n\
    -i <ip_addr>    Set server ip\n\
    -b <buf_len>    Length of UDP data buffer\n\
    -d <addr>       Decode UDP packet at memory address\n\
    -m <mac>        Set destination MAC address (raw only)\n\
    -r <nframes>    Raw UDP test\n\
    -u <nframes>    Simple UDP test\n";

// Computing the internet checksum (RFC 1071).
// Note that the internet checksum does not preclude collisions.
uint16_t
checksum (uint16_t *addr, int len)
{
  int count = len;
  register uint32_t sum = 0;
  uint16_t answer = 0;

  // Sum up 2-byte values until none or only one byte left.
  while (count > 1) {
    sum += *(addr++);
    count -= 2;
  }

  // Add left-over byte, if any.
  if (count > 0) {
    sum += *(uint8_t *) addr;
  }

  // Fold 32-bit sum into 16 bits; we lose information by doing this,
  // increasing the chances of a collision.
  // sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits)
  while (sum >> 16) {
    sum = (sum & 0xffff) + (sum >> 16);
  }

  // Checksum is one's compliment of sum.
  answer = ~sum;

  return (answer);
}

/*
** ++
**
**
** --
*/

void _usage(void)
  {
  optind = 0;
  printf("%s",USAGE_UDP);
  }

unsigned _decode(uint32_t adr)
  {
  UdpPacket* packet = (UdpPacket*)adr;
  
  printf("Ethernet Header\n");
  printf("  dst      0x%llx\n",packet->ethernet.dst);
  printf("  type     0x%x\n",  packet->ethernet.type);
    
  printf("Ip Header\n");
  printf("  version  0x%x\n",packet->ip.version);
  printf("  tos      0x%x\n",packet->ip.tos);     
  printf("  length   0x%x\n",packet->ip.length);  
  printf("  id       0x%x\n",packet->ip.id);
  printf("  offset   0x%x\n",packet->ip.offset);
  printf("  ttl      0x%x\n",packet->ip.ttl);
  printf("  protocol 0x%x\n",packet->ip.protocol);
  printf("  checksum 0x%x\n",packet->ip.checksum);
  printf("  src      0x%x\n",packet->ip.src);
  printf("  dst      0x%x\n",packet->ip.dst);
  
  printf("Udp Header\n");
  printf("  src      0x%x\n",packet->udp.src);
  printf("  dst      0x%x\n",packet->udp.dst);
  printf("  length   0x%x\n",packet->udp.length);
  printf("  checksum 0x%x\n",packet->udp.checksum);
   
  }  
  
/*
** ++
**
**
** --
*/
 
unsigned _raw(unsigned count)
  {
  int i;
  char *buf;
  int dropped = 0;
  
  for(i=0;i<count;i++)  
    {
        
    UdpPacket* packet;
    while(1)
      {
      packet = (UdpPacket*)Xaui_Alloc(tx_device, sizeof(UdpPacket)+DATA_LEN);     
      if(!packet)      
        dropped++;
      else
        break;
      }      

    packet->ethernet.dst  = BSWP__swap64(_mac);
    packet->ethernet.type = 0x8;    

    packet->ip.version    = 0x45;
    packet->ip.tos        = 0x0;
    packet->ip.length     = htons(sizeof(IpHeader)+sizeof(UdpHeader)+DATA_LEN);
    packet->ip.id         = 0x8646;
    packet->ip.offset     = 0x0;
    packet->ip.ttl        = 0x40;
    packet->ip.protocol   = 0x11;
    packet->ip.src        = client_ip;
    packet->ip.dst        = inet_addr(server_ip);
    packet->ip.checksum   = 0;

    packet->ip.checksum   = checksum((unsigned short*)&packet->ip,sizeof(IpHeader));

    packet->udp.src       = htons(SRC_PORT);
    packet->udp.dst       = htons(DST_PORT);
    packet->udp.length    = htons(sizeof(UdpHeader)+DATA_LEN);
    packet->udp.checksum  = 0;

    buf = (char*)&packet->mbz;
    sprintf(buf, "This is packet %d\n", i);
    Xaui_Post(tx_device, (Xaui_Header*)packet, sizeof(UdpPacket)+DATA_LEN);
    }
  
  printf("Posted cli 0x%x srv 0x%x %d %d byte frames, %d retries\n",client_ip,server_ip,count,sizeof(UdpPacket)+DATA_LEN,dropped);
  return 0;
  
  }

/*
** ++
**
**
** --
*/
     
unsigned _udp(uint32_t len)
  {
  struct sockaddr_in si_other;
  int s, i, slen=sizeof(si_other);
  char *buf;
  
  buf = malloc(buf_len);

  if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
   {
   printf("error in recvfrom()\n");
   return 1;
   }


  memset((char *) &si_other, 0, sizeof(si_other));
  si_other.sin_family = AF_INET;
  si_other.sin_port = htons(DST_PORT);
  if (inet_aton(server_ip, &si_other.sin_addr)==0) {
    fprintf(stderr, "inet_aton() failed\n");
    return 1;
  }

  READ_GTC(dma_st);

  for (i=0; i<len; i++) {
    //printf("Sending packet %d to %s\n", i,server_ip);
    sprintf(buf, "This is packet %d\n", i);
    if (sendto(s, buf, buf_len, 0, (const struct sockaddr*)&si_other, slen)==-1)
      {
      printf("error in sendto()\n");
      return 1;
      }
    }

  READ_GTC(dma_et);
  
  if(dma_st>dma_et)
    dma_elp += (0xffffffff-dma_st) + dma_et;
  else
    dma_elp += dma_et-dma_st;
  dma_cnt++; 
          

  printf("Sent %d %d byte packets (%d) to %s in %llu\n", len,buf_len,(len*buf_len),server_ip,dma_elp);

  close(s);  
  free(buf);
  
  dma_elp = 0;
  return 0;
  }

/*
** ++
**
**
** --
*/
 
unsigned _setup(void)
  {
  buf_len = BUFLEN;
  
  tx_device = Xaui_TxOpen(64);
  if(!tx_device) return 1;
  Xaui_TxBind(tx_device,NULL,NULL);

  struct ifaddrs *ifAddr, *ifa;
  
  if (getifaddrs(&ifAddr) == -1)
    {
    return 1;
    }
  for (ifa = ifAddr; ifa; ifa = ifa->ifa_next)
    {
    if ( !ifa->ifa_addr )
      continue;
    if (AF_INET == ifa->ifa_addr->sa_family &&
        strncmp(ifa->ifa_name, LOOPBACK_NAME,strlen(LOOPBACK_NAME)))
      {
      client_ip = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
      break;
      }      
    }    

  return 0;
  }

/*
** ++
**
**
** --
*/
   
void _ip(char *addr)
  {
  memcpy(&server_ip,addr,(strlen(addr) > 32 ? 32 : strlen(addr)));
  }

/*
** ++
**
**
** --
*/
   
void _buflen(uint32_t len)
  {
  buf_len = len;
  }
    
/*
** ++
**
** This function executes the UDP shell command
**
** --
*/
  
void _main(int argc, char** argv)
  {
  int theOpt;

  if(argc == 1) return _usage();

  struct getopt_data getopt_reent;
  memset(&getopt_reent, 0, sizeof(getopt_data));
  
  while ( (theOpt = getopt_r(argc,argv,
                             "b:i:hu:d:r:m:", 
                             &getopt_reent)) != EOF )
    {  
    switch (theOpt)
      {
      case 'b': _buflen(strtoul(getopt_reent.optarg, 0, 0));  break;      
      case 'i': _ip(getopt_reent.optarg);  break;
      case 'u': _udp(strtoul(getopt_reent.optarg, 0, 0));  break;
      case 'd': _decode(strtoul(getopt_reent.optarg, 0, 0));  break;
      case 'r': _raw(strtoul(getopt_reent.optarg, 0, 0)); break;
      case 'm': _mac = strtoull(getopt_reent.optarg, 0, 0); break;
      case 'h':
      default:  return _usage();
      }
    }
  }

/*
** ++
**
** This function registers UDP commands with the rtems shell.
**
** --
*/

void _addCommands(void)
  {
  rtems_shell_cmd_t * retCmd;
  // Call the long form for adding this command.  This
  // allocates the structure space inside RTEMS

  retCmd = rtems_shell_add_cmd("udp_test",
                               "ppi",                             
                               USAGE_UDP,                             
                               (rtems_shell_command_t)_main);

  rtems_shell_add_cmd_struct( retCmd );
  }
  
/*
** ++
**
* This routine executes the constructor 
* for the UDP test shareable library.
*
** --
*/

int lnk_prelude(void *prefs, void *elfHdr) 
  {  
  /* register shell commands */
  _addCommands();
  
  return _setup();  
  }
