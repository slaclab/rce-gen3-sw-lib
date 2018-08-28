// -*-Mode: C++;-*-
/*!@file     configure.cc
*
* @brief     Initialization code for starting networking.
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      June 26, 2012 -- Created
*
* $Revision: 4078 $
*
* @verbatim                    Copyright 2012
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#if 0
#define DEBUG
#endif

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/systm.h>
#include <netinet/in.h>
#include <rtems/bsdnet/servers.h>

#include <rtems/rtems_bsdnet.h>
#include <rtems/mkrootfs.h>


#include "elf/linker.h"
#include "debug/print.h"

#include "network/configure.hh"
#include "EthernetConfig.hh"
#include "ethPio/Ethernet.hh"
#include "network/BsdNet.hh"
#include "network/BsdNet_Config.h"

#include "debug/print.h"
#include "string/ip2dot.hh"
#include "string/id2node.hh"

namespace exc = tool    :: exception;
namespace net = driver  :: network;
namespace str = tool    :: string;

struct rtems_bsdnet_config rtems_bsdnet_config = {.ifconfig=NULL}; // Set the list of ifconfigs to empty.

int lnk_options __attribute__((visibility("default"))) =  LNK_INSTALL;

static const char BSDNET_ATTRS[] = "BSDNET_ATTRS";
static const char _attrs_error[] = "BsdNet attributes lookup failed for symbol %s\n";

extern "C"
  {
  uint32_t BsdNet_Configure(BsdNet_Attributes *prefs, void *attach) 
    {
      // configure interfaces     
      net::configure(prefs,0,attach);
      return 0;
    }
  }

namespace driver {

  namespace network {

    void configure(BsdNet_Attributes *prefs, unsigned iface, void *attach)
      throw (exc::Error)
    {
      EthernetConfig ethConfig;
      ethConfig.retrieve(attach);
      configure(prefs,ethConfig, iface);
    }

    void configure(BsdNet_Attributes *prefs, const EthernetConfig& ethConfig, unsigned iface)
      throw (exc::Error)
    {
      const unsigned numfaces = ethConfig.numInterfaces();
      if (!numfaces) {
	throw exc::Error("No ethernet interfaces found");
      }
#ifdef DEBUG
      else
        dbg_printv("Found %d ethernet interface%c\n",
                    numfaces, (numfaces > 1) ? 's' : ' ');
#endif

      for (unsigned i=0; i<numfaces; i++) {
	const net::Params&   params = ethConfig.interface(i);
#ifdef DEBUG
	const unsigned char* hw     = params.hwaddr();
	dbg_printv("Found params interface [%d] %s, "
                    "mac %02x:%02x:%02x:%02x:%02x:%02x\n",
                    i, params.name(), hw[0],hw[1],hw[2],hw[3],hw[4],hw[5]);
#endif

        // In principle, a BSD interface could be set up for each ethernet interface
	if (i == iface) {
#ifdef DEBUG
          dbg_printv("About to create the BSD interface for %s\n", params.name());
#endif
          net::create(params.name(),        // Interface name
                      params.hwaddr(),      // MAC address
                      params.attach(),      // Driver attach function
                      "0.0.0.0",            // IP address
                      "0.0.0.0",            // netmask
                      IfaceBufCount,        // rx buf count
                      IfaceBufCount);       // tx buf count
        }
      }

      // If we are to update the files create the root file structure.
      static bool upd = true;
      if (upd) {
        if (rtems_create_root_fs() < 0) {
          dbg_printv("Error creating the root filesystem.\n"
                     "File not created.\n");
          upd = 0;
        }
      }
      
      // configure BsdNet
      net::configure(prefs);
    }

  }

}
