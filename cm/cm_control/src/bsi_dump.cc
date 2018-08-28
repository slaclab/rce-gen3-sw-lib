#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

#include "cm_svc/ClusterIpInfo.hh"

extern "C" {
#include "bsi/CMB_switch_cfg.h"
#include "bsi/Bsi_Cfg.h"
#include "bsi/Bsi.h"
#include "map/Lookup.h"
#include "map/MapBsi.h"
}

//  BSI Fabric Map bit shifts,masks
enum { s_slot=0, s_chan=4, s_port=8, s_stat=13 };
enum { m_slot=(1<<(s_chan-s_slot))-1 };
enum { m_chan=(1<<(s_port-s_chan))-1 };
enum { m_port=(1<<(s_stat-s_port))-1 };
enum { m_stat=(1<<(16-s_stat))-1 };
enum { ok_stat=0x4 };
enum { MAX_PORT=24, ALL_PORTS=(2<<MAX_PORT)-1 };
enum { FABRIC_MAP_WORDS=8 };  // 32b words per slot
enum { FABRIC_MAP_SLOTS=16 };

static const char* speedstr[] = { "NC","DIS","1GB","1GBL1","1GBL2","1GB-R","XAUI" };

static inline void print_ip(unsigned v) 
{
  printf("%d.%d.%d.%d",(v>>24)&0xff,(v>>16)&0xff,(v>>8)&0xff,(v>>0)&0xff);
}

static void usage(const char*);

int main(int argc, char** argv)
{
  Bsi bsi(LookupBsi());

  // Drain the FIFO
  {
    uint32_t data = BsiRead32(bsi,BSI_FIFO_OFFSET);
    unsigned n = 0;
    while(BSI_FIFO_GET_VALID(data)) {
      data = BsiRead32(bsi,BSI_FIFO_OFFSET);
      n++;
    }
  }

  if (argc<2) {
    //
    //  Dump all raw contents
    //
    {
      printf("== BSI raw data ==\n");
      for(unsigned port=0; port<MAX_PORT; port++) {
        unsigned val = BsiRead32(bsi,BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET + port);
        printf("[%02d] %08x\n",port,val);
      }
    }

    {
      printf("== BSI_CLUSTER_SWITCH_CONFIGURATION ==\n");
      printf("PORT  TYPE   GROUP\n");
      // Read from RAM
      for(unsigned port = 0; port<MAX_PORT; port++) {
    
        unsigned val = BsiRead32(bsi,BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET + port);

        unsigned speed    = val&0xff;
        unsigned trunk    = (val>>16)&0xff;
        if (speed >= CMB_SWITCH_PORT_TYPE_1000BASEX_LANE0)
          printf("%4d  %5.5s  %d\n",port,speedstr[speed],trunk);
      }
    }

    {
      printf("== BSI_FABRIC_MAP ==\n");
      Offset offset = BSI_FABRIC_MAP_OFFSET;
      for(unsigned i=0; i<FABRIC_MAP_SLOTS; i++) {
        for(unsigned j=0; j<FABRIC_MAP_WORDS; j++,offset++) {
          uint32_t v = BsiRead32(bsi, offset);
          printf(" %04x %04x", v&0xffff, v>>16);
        }
        printf("\n");
      }
      uint32_t v = BsiRead32(bsi, offset);
      printf("[%08x]\n",v);
    }

    {
      Offset offset=BSI_RCE_DATA_OFFSET;
      printf("== BSI_RCE_DATA ==\n");
      printf("%08x %08x %08x %08x %08x\n",
             BsiRead32(bsi,offset+0),
             BsiRead32(bsi,offset+1),
             BsiRead32(bsi,offset+2),
             BsiRead32(bsi,offset+3),
             BsiRead32(bsi,offset+4));
    }

    {
      Offset offset=BSI_CLUSTER_DATA_OFFSET;
      printf("== BSI_CLUSTER_DATA ==\n");
      printf("%08x %08x %08x\n",
             BsiRead32(bsi,offset+0),
             BsiRead32(bsi,offset+1),
             BsiRead32(bsi,offset+2));
      uint32_t buff[8];
      for(unsigned i=0; i<8; i++)
        buff[i] = BsiRead32(bsi,offset+3+i);
      printf("%s\n",(char*)buff);
      printf("%08x\n",BsiRead32(bsi,offset+12));
    }

    /*
      {
      Offset offset=BSI_CLUSTER_ELEMENT_INTERCONNECT_OFFSET;
      printf("== BSI_ELEMENT_INTERCONNECT ==\n");
      for(unsigned i=0; i<16; i++)
      printf("%08x ",BsiRead32(bsi,offset+i));
      printf("\n");
      }
    */

    printf("=== BSI_CLUSTER_IP_INFO ==\n");
    cm::svc::ClusterIpInfo info;
    printf("use_vlan %c\n", info.use_vlan()?'T':'F');
    printf("vlan_id %d\n",  info.vlan_id());
    printf("reject tagged/untagged/bv %c/%c/%c\n",
           info.reject_tagged  ()?'T':'F',
           info.reject_untagged()?'T':'F',
           info.reject_boundary_violation()?'T':'F');
    printf("ip "); print_ip(info.ip()); printf("\n");
    printf("nm "); print_ip(info.netmask()); printf("\n");
    printf("ip_beg "); print_ip(info.ip_begin()); printf("\n");
    printf("ip_end "); print_ip(info.ip_end()); printf("\n");
    printf("gateway "); print_ip(info.gateway()); printf("\n");
  }
  else {
    cm::svc::ClusterIpInfo info;

    const char* arg = argv[1];
    if (strcmp(arg,"dhcp")==0) {
      unsigned lslot = 0;
      while(!(lslot&0x80000000))
        lslot = BsiRead32(bsi,BSI_FABRIC_MAP_OFFSET + 
                          FABRIC_MAP_SLOTS*FABRIC_MAP_WORDS);
      printf("%d",(lslot>>30)&1);
    }
    else if (strcmp(arg,"ip")==0) {
      print_ip(info.ip());
    }
    else if (strcmp(arg,"ipnm")==0) {
      print_ip(info.ip());
      printf("/%d",info.netmask_len());
    }
    else if (strcmp(arg,"ipbegin")==0) {
      print_ip(info.ip_begin());
    }
    else if (strcmp(arg,"ipend")==0) {
      print_ip(info.ip_end());
    }
    else if (strcmp(arg,"netmask")==0) {
      print_ip(info.netmask());
    }
    else if (strcmp(arg,"subnet")==0) {
      print_ip(info.ip()&info.netmask());
    }
    else if (strcmp(arg,"broadcast")==0) {
      print_ip(info.ip()|~info.netmask());
    }
    else if (strcmp(arg,"gateway")==0) {
      print_ip(info.gateway());
    }
    else
      usage(argv[0]);
  }

  return 0;
}

void usage(const char* p)
{
  printf("Usage: %s [parameter]\n"
         "(no parameters)      dump switch configuration and fabric map\n"
         "parameter : ip       static IP address of interface (e.g. 192.168.0.1)\n"
         "          : ipnm     static IP adddress + netmask (e.g. 192.168.0.1/24)\n"
         "          : netmask  netmask (e.g. 255.255.255.0)\n"
         "          : bcast    broadcast (e.g. 192.168.0.255)\n"
         "          : ipbegin  base of dhcp ip range\n"
         "          : ipend    end of dhcp ip range\n"
         "          : gateway  gateway IP address\n",
         p);
}
