

#include "network/BsdNet.hh"
#include "network/BsdNet_Config.h"

#include "container/List.hh"
#include "exception/Exception.hh"

extern "C" {
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <net/ethernet.h>
}
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <rtems/dhcp.h>

namespace cont = tool    :: container;
namespace exc  = tool    :: exception;

extern struct rtems_bsdnet_config rtems_bsdnet_config;

extern "C" int rtems_bsdnet_initialize_network (void);

namespace driver {

  namespace network {

    void initialize()
    {
    }

    void create(const char*   name,
                const uint8_t hwaddr[HwAddrLen],
                int         (*attach)(struct rtems_bsdnet_ifconfig *conf, int attaching),
                const char*   ip_dotted,
                const char*   netmask,
                int           rbuf_count,
                int           xbuf_count)
    {
      struct rtems_bsdnet_ifconfig* ifcfg = new struct rtems_bsdnet_ifconfig;
      memset(ifcfg, 0, sizeof(struct rtems_bsdnet_ifconfig));

      ifcfg->name             = const_cast<char*>(name);
      ifcfg->hardware_address = const_cast<uint8_t*>(hwaddr);
      ifcfg->attach           = attach;
      ifcfg->ip_address       = const_cast<char*>(ip_dotted);
      ifcfg->ip_netmask       = const_cast<char*>(netmask);
      ifcfg->rbuf_count       = rbuf_count;
      ifcfg->xbuf_count       = xbuf_count;

      struct rtems_bsdnet_ifconfig* iface = rtems_bsdnet_config.ifconfig;
      if (!iface) {
        rtems_bsdnet_config.ifconfig = ifcfg;
      } else {
        while (iface->next) {
          iface = iface->next;
        }
        iface->next = ifcfg;
      }
    }

    void configure(const BsdNet_Attributes* prefs)
    {
      rtems_bsdnet_config.hostname       = (char *)prefs->hostname;
      rtems_bsdnet_config.domainname     = (char *)prefs->domainname;
      rtems_bsdnet_config.gateway        = (char *)prefs->gateway;
      rtems_bsdnet_config.log_host       = (char *)prefs->log_host;      
      rtems_bsdnet_config.name_server[0] = (char *)prefs->name_server1;
      rtems_bsdnet_config.name_server[1] = (char *)prefs->name_server2;
      rtems_bsdnet_config.name_server[2] = (char *)prefs->name_server3;
      rtems_bsdnet_config.ntp_server[0]  = (char *)prefs->ntp_server1;
      rtems_bsdnet_config.ntp_server[1]  = (char *)prefs->ntp_server2;
      rtems_bsdnet_config.ntp_server[2]  = (char *)prefs->ntp_server3;

      if (prefs->use_dhcp)
        rtems_bsdnet_config.bootp = rtems_bsdnet_do_dhcp;
      else
        rtems_bsdnet_config.bootp = NULL;

      rtems_bsdnet_config.network_task_priority = prefs->priority;
      rtems_bsdnet_config.sb_efficiency = prefs->sb_efficiency;

      rtems_bsdnet_config.udp_tx_buf_size = prefs->udp_tx_buf_size;
      rtems_bsdnet_config.udp_rx_buf_size = prefs->udp_rx_buf_size;

      rtems_bsdnet_config.tcp_tx_buf_size = prefs->tcp_tx_buf_size;
      rtems_bsdnet_config.tcp_rx_buf_size = prefs->tcp_rx_buf_size;

      unsigned buf_count = 0;
      struct rtems_bsdnet_ifconfig* iface = rtems_bsdnet_config.ifconfig;
      while (iface) {
        buf_count += iface->rbuf_count;
        buf_count += iface->xbuf_count;
        iface = iface->next;
      }
      if (!prefs->mbuf_bytecount)
        rtems_bsdnet_config.mbuf_bytecount = buf_count*sizeof(struct mbuf);
      else
        rtems_bsdnet_config.mbuf_bytecount = prefs->mbuf_bytecount;
        
      if (!prefs->mbuf_cluster_bytecount)
        rtems_bsdnet_config.mbuf_cluster_bytecount = buf_count*(ETHERMTU_JUMBO/MCLBYTES+1)*MCLBYTES;
      else
        rtems_bsdnet_config.mbuf_cluster_bytecount = prefs->mbuf_cluster_bytecount;
        
      rtems_bsdnet_initialize_network();
      
      /* disable ntp */
      //rtems_bsdnet_synchronize_ntp(0, 0);
    }
  }  // bsdNet
} // driver
