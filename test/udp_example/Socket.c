/*
** ++
**  Package:
**	
**
**  Abstract:
**
**  TBD     
**
**  Author:
**      Michael Huffer, SLAC (mehsys@slac.stanford.edu)
**
**  Creation Date:
**	000 - October 09, 2013
**
**  Revision History:
**	None.
**
** --
*/

#include <strings.h>
#include <stdio.h>   // printf...
#include <stdint.h>

#include "rtems.h"

#include "conversion/impl/BSWP.ih"
#include "elf/linker.h"
#include "memory/cpu/resources.h"
#include "memory/mem.h"
#include "sas/Sas.h"
#include "xaui/Xaui.h"
#include "net/Interface.hh"

#include "PseudoARP.hh"
#include "Socket.h"

#define XAUI_HDR_SIZE 128 

#if defined(__cplusplus)
extern "C" {
#endif

extern examples::PseudoARP* common_pseudoarp_instance;

/*
** Forward declarations for local functions...
*/

static uint32_t _vector(         SAS_Frame,     SAS_Arg, SAS_IbMbx);
static uint32_t _process_inbound(SAS_Message,   SAS_Arg, SAS_Mbx);
static uint32_t _process_outbound(SAS_Message,  SAS_Arg, SAS_ObMbx);

static void     _register(unsigned port, unsigned numof_buffers);
static uint16_t _checksum(IpHeader*);

/*
** TBD...
*/

#define PSEUDO_IP 0x0600

#define MAX_PORTS 32

static void* _active[MAX_PORTS];

uint32_t     _max_data_in_header;
 
/*
** TBD...
*/

static const char Xaui[] = "Xaui"; // Name of XAUI plugin

Socket::Socket(unsigned numof_buffers)
 {
  
 SAS_Session session = SAS_Open();
  
 _max_data_in_header = XAUI_HDR_SIZE - sizeof(UdpPacket);
 
 SAS_Mbx     IbMbx   = SAS_Bind(_process_inbound, &_inbound_datagram, session);
 SAS_MbxId   port    = SAS_Id(IbMbx);
  
 _inbound_datagram = (uint32_t*)0; 
 _inbound_session  = session;
 _inbound_mbx      = IbMbx;
 
 SAS_ObMbx ObMbx   = SAS_ObBind(Xaui, _process_outbound, (void*)0, session);

 _outbound_session  = session;
 _outbound_mbx      = ObMbx;
 
 _own_mac          = Xaui_Mac();
 _own_port         = port;
 
 printf("%s: using RX port %d\n",__func__,_own_port);

 service::net::Interface interface;

 _own_adr          = interface.ipAddr();
 
 _arp              = common_pseudoarp_instance;
   
 _register(port, numof_buffers);
 
 }

/*
**
*/

static uint32_t _process_inbound(SAS_Message message, SAS_Arg arg, SAS_Mbx mbx)
 {
 
 uint32_t** result   = (uint32_t**)arg;
  
 if(SAS_Error(message))
  {
  printf("Inbound error detected\n");
  *result = 0;
  }
  else
  {
  uint32_t*  datagram = (uint32_t *)message; 
  *result = datagram;
  }

 return SAS_DISABLE | SAS_ABORT;
 }

/*
**
*/

static uint32_t _process_outbound(SAS_Message message, SAS_Arg arg, SAS_ObMbx mbx)
 {
 if(SAS_Error(message)) printf("Transmit error detected\n");
 return SAS_DISABLE | SAS_ABORT;
 }

/*
**
*/

static void _register(unsigned port, unsigned numof_buffers)
 {
 
 void* buffer =  (void*)mem_Region_alloc(MEM_REGION_UNCACHED, numof_buffers << SIZEOF_DATAGRAM); 
 
 if(!buffer) return;
 
 _active[port] = mem_rsOpen(numof_buffers, buffer, SIZEOF_DATAGRAM);
  
 return; 
 } 

/*
**
*/

Datagram* Socket::wait()
 {
   
 SAS_EnableWait(_inbound_session);
 
 return (Datagram *)_inbound_datagram; 
 }

void Socket::close()
 {
 if(_inbound_session) SAS_Close(_inbound_session);
 if(_outbound_session) SAS_Close(_outbound_session);
 }

/*
**
*/

#define HEADER_SIZE ((sizeof(UdpPacket) + sizeof(uint32_t)) /8)

void Socket::sendTo(uint32_t dst_adr, uint32_t dst_port, uint32_t* datagram, uint32_t sizeof_datagram)
 {

 if(!sizeof_datagram) return;
  
 SAS_ObMbx mbx   = _outbound_mbx;
 SAS_Frame frame = SAS_ObAlloc(mbx); 
 
 if(!frame) return;
  
 SAS_Fd* fd = SAS_ObFd(frame,mbx);
 
 UdpPacket* packet = (UdpPacket*)fd->header;
 
 packet->ethernet.dst   = _arp->lookup(dst_adr);
 packet->ethernet.type  = BSWP__swap16(PSEUDO_IP);
  
 packet->ip.version    = IPV4_VERSION;
 packet->ip.protocol   = IPV4_UDP;
 packet->ip.checksum   = 0;
 packet->ip.id         = 0;
 packet->ip.offset     = 0;
 packet->ip.length     = sizeof(IpHeader) + sizeof(UdpHeader) + (sizeof_datagram << 2);
 packet->ip.tos        = 0;
 packet->ip.src        = _own_adr;
 packet->ip.dst        = dst_adr;
 packet->ip.checksum   = _checksum(&packet->ip);

 packet->udp.src       = _own_port;
 packet->udp.dst       = dst_port;
 packet->udp.length    = sizeof(UdpHeader) + (sizeof_datagram << 2);
 packet->udp.checksum  = 0;
 
 uint32_t* payload = datagram;

 *(uint32_t*)(packet + 1) = *payload; 
 
 payload += 4;

 fd->payload = (void*)payload;
 fd->size    = (sizeof_datagram - 1) << 2;
 fd->mid      = SAS_ObId(mbx);
 fd->message = (void*)0;
 
 rtems_cache_flush_multiple_data_lines(fd->payload,
                                       fd->size);
 
 SAS_ObPost(SAS_OB_PAYLOAD_RUNDOWN, SAS_ObSet(frame, XAUI_FRAME_TYPE, HEADER_SIZE), mbx);
 
 SAS_EnableWait(_outbound_session);
 
 return; 
 }

/*
**
*/

void Datagram::free() {mem_rsFree((void*)_content[3], (void*)this);}
 
/*
** TBD...
*/

int lnk_prelude(void* prefs, void* elf)
 {
    
 Xaui_Bind(PSEUDO_IP, (SAS_IbHandler)_vector, &_active);
 
 return 0;
 
 }
 
/*
**
*/

uint16_t _checksum(IpHeader* header)
 {
 uint16_t *next = (uint16_t*)header;
 uint32_t sum   = 0;
 int      remaining = sizeof(IpHeader) / 2;
 
 do sum += *next++;while(--remaining);
 
 sum += (sum >> 16);
 
 return ~sum;
 }

/*
**
*/

static void _mac(uint64_t mac, uint16_t *dst)
 {
 dst[2] = (mac >> 32) & 0xffff;  
 dst[1] = (mac >> 16) & 0xffff;  
 dst[0] =  mac & 0xffff;           
 }

/*
**
*/

static void _dump(UdpPacket *packet)
 {
 uint32_t *header;
 header = (uint32_t*)packet;
 
 printf("\nSocket %s: hdr  0x%08x\n",__func__,(int)header);
 printf("Socket %s: hdr[0]  0x%08x\n",__func__,(int)header[0]);
 printf("Socket %s: hdr[1]  0x%08x\n",__func__,(int)header[1]);
 printf("Socket %s: hdr[2]  0x%08x\n",__func__,(int)header[2]);
 printf("Socket %s: hdr[3]  0x%08x\n",__func__,(int)header[3]);
 printf("Socket %s: hdr[4]  0x%08x\n",__func__,(int)header[4]);
 printf("Socket %s: hdr[5]  0x%08x\n",__func__,(int)header[5]);
 printf("Socket %s: hdr[6]  0x%08x\n",__func__,(int)header[6]);
 printf("Socket %s: hdr[7]  0x%08x\n",__func__,(int)header[7]); 
 printf("Socket %s: ip  src 0x%08x dst 0x%08x\n",__func__,packet->ip.src,packet->ip.dst);
 printf("Socket %s: udp src 0x%08x dst 0x%08x\n",__func__,packet->udp.src,packet->udp.dst);
 }
    
/*
**
*/

#define IHL 5

#define UDP_MINIMUM 8

static uint32_t _vector(SAS_Frame frame, SAS_Arg arg, SAS_IbMbx mbx)
 {
 if(SAS_IbError(frame)) goto rundown;
 SAS_Fd *fd;
 fd = SAS_IbFd(frame, mbx);
 
 UdpPacket* packet;
 packet = (UdpPacket*)fd->header;
 
 if(_checksum(&packet->ip))             goto rundown;
 
 if(packet->ip.version != IPV4_VERSION) goto rundown;

 if(packet->ip.protocol != IPV4_UDP)    goto rundown; 
 
 int length;
 length = (int)packet->udp.length - sizeof(UdpHeader);
  
 if(length < 0) goto rundown;
 
 int max_in_header;
 max_in_header = _max_data_in_header;
    
 int remaining;
 remaining = SAS_IbPayload(frame) ? max_in_header : length; 
 
 length -= remaining;

 if(length < 0)                goto rundown;
 if(remaining > max_in_header) goto rundown;

 uint32_t port;
 port = packet->udp.dst;
 
 if(port > MAX_PORTS) goto rundown;
 
 void** active;
 active = (void**)arg;
 
 void*  buffer;
 buffer = active[port];
 
 if(!buffer) goto rundown;
 
 uint32_t* datagram;
 datagram = (uint32_t*)mem_rsAlloc(buffer);
 
 if(!datagram) goto rundown;

 datagram[0] = length;
 datagram[1] = packet->ip.src;  
 datagram[2] = packet->udp.src;  
 datagram[3] = port;
 datagram[4] = (uint32_t)buffer;
 
 uint32_t* src;
 src = (uint32_t*)(packet+1);
   
 uint32_t* dst;
 dst = &datagram[4];
 
 /* no dma for first 4 bytes of datagram */
 length -= 4;
  
 while(remaining--) *dst++ = *src++;
  
 fd->payload = (void*)dst;
 fd->size    = length;
 fd->mid     = port;
 fd->message = (void*)datagram;  
 
 return SAS_IB_PAYLOAD_RUNDOWN;

 rundown:
     
 return SAS_IbPayload(frame) ? SAS_IB_FLUSH : SAS_IB_FREE;
 } 

#if defined(__cplusplus)
} // extern "C"
#endif
