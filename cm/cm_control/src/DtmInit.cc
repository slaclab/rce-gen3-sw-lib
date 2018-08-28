//#define NEW_BOARD
#define DO_DHCP
//#define RYAN_COB
//#define RTM_TEST


#include "control/DtmInit.hh"

#include "debug/Debug.hh"
#include "debug/Print.hh"
#include "shell/ShellCommon.hh"

#include "concurrency/OldThread.hh"

#include "fci/Manager.hh"
#include "ffs/IdFile.hh"
#include "cmb/bsi.hh"

#include "control/ChrDisplay.hh"

#include "net/Manager.hh"

#include "fm/FmConfig.hh"
#include "fm/FmTahoe.hh"
#include "fm/FmMacTable.hh"

#include "console/UploadManager.hh"
#include "console/DebugHandler.hh"

#include "control/DtmConsoleTask.hh"
#include "control/DtmConfigTask.hh"
#include "control/DtmShell.hh"

#include "shell/ShellCommands.hh"

#include <rtems.h>
#include <bsp.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <net/if.h>
//#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/**** DHCP and BSDNET ****/
extern "C" {
#include <rtems/rtems_bsdnet.h>
#include <rtems/rtems_bsdnet_internal.h>
#include <sys/socket.h>         // needed by net/if.h
#include <net/if.h>             // needed by netinet/if_ether.h
#include <net/if_var.h>         // needed by netinet/if_ether.h
#include <rtems/dhcp.h>
}



extern "C" {
  extern void rtems_bsdnet_loopattach();
  static struct rtems_bsdnet_ifconfig loopback_config = {
    (char*)"lo0",           /* name */
    (int (*)(struct rtems_bsdnet_ifconfig *, int))rtems_bsdnet_loopattach,
                            /* attach function */
    NULL,                   /* link to next interface */
    (char*)"127.0.0.1",     /* IP address */
    (char*)"255.0.0.0",     /* IP net mask */
  };

  struct rtems_bsdnet_config rtems_bsdnet_config = {
    //&fm_config,             /* Network interface */
    &loopback_config,       /* Network interface */
    NULL,                   /* Use fixed network configuration */
    80,                     /* Default network task priority */
    0,                      /* Default mbuf capacity */
    0,                      /* Default mbuf cluster capacity */
    (char*)"dtm01",         /* Host name */
    (char*)"lab1.reg",      /* Domain name */
    NULL,                   /* Gateway */
    NULL,                   /* Log host */
    { NULL },               /* Name server(s) */
    { NULL }                /* NTP server(s) */
  };
}


//============================================

#if 0
static void setMacAddr(unsigned char* mac_addr, const char* mac_str)
{
  unsigned addr[6];
  sscanf(mac_str,"%x:%x:%x:%x:%x:%x",
	 &addr[0],
	 &addr[1],
	 &addr[2],
	 &addr[3],
	 &addr[4],
	 &addr[5]);
  for(int k=0; k<6; k++)
    mac_addr[k] = addr[k];
}

static void
setHostMacAddress(FmConfig* cfg, const cm::ffs::IdFile& idf)
{
  //  setMacAddr(cfg[0].hostCfg.mac_addr,idf.macBase());
  setMacAddr(cfg[0].hostCfg.mac_addr,idf.macFabric());
}
#endif

void cm::control::dtmInitialize()
{
  printf("bootFm begin\n");

  FmConfig* fmCfg = new FmConfig[1];
  sprintf(fmCfg[0].hostCfg.if_name,"eth0");
#ifdef DO_DHCP
  fmCfg[0].hostCfg.ip_addr     = 0;          // DHCP will configure
  fmCfg[0].hostCfg.ip_netmask  = 0;
#else
  fmCfg[0].hostCfg.ip_addr     = 0xc0a80064;
  fmCfg[0].hostCfg.ip_netmask  = 0xffffff00;
#endif
#if 0
  fmCfg[0].protocolCfg.igmpVsn = FmProtocolConfig::NoIGMP;
  fmCfg[0].portCfg.ports10Mb   = FmPortMask(0);
  fmCfg[0].portCfg.ports100Mb  = FmPortMask(0);
  fmCfg[0].portCfg.ports2_5Gb  = FmPortMask(0);
  fmCfg[0].portCfg.ports4Gb    = FmPortMask(0);
#ifdef RYAN_COB
  fmCfg[0].portCfg.ports1Gb    = FmPortMask((1<<3)  | // DPM0.0
                                            (1<<11) | // DPM1.0
                                            (1<<2)  | // DPM2.0
                                            (1<<7));  // DPM3.0
  fmCfg[0].portCfg.ports10Gb   = FmPortMask((1<<8)  | // Zone2 Slot1
                                            (1<<19)); // RTM port 1
#else
#ifdef RTM_TEST
  fmCfg[0].portCfg.ports1Gb    = FmPortMask(1<<19);   // RTM port 1
  fmCfg[0].portCfg.ports10Gb   = FmPortMask((1<<13) | // RTM port 0
                                            (1<<8)  | // Zone2 Slot1
                                            (1<<3));  // DPM0.0
#else
  fmCfg[0].portCfg.ports1Gb    = FmPortMask(0);
  fmCfg[0].portCfg.ports10Gb   = FmPortMask((1<<8)  | // Zone2 Slot1
                                            (1<<19) | // RTM port 1
                                            (1<<3));  // DPM0.0
#endif
#endif

  //
  //  Derive MAC addresses from board ID
  //
  cm::ffs::IdFile* idf = new cm::ffs::IdFile;
  printf("Read Board ID '%s' '%s' '%s'\n",
         idf->name(), idf->version(), idf->serialNumber());
  setHostMacAddress(fmCfg,*idf);

  cm::net::Manager::board(idf ? idf->version() : "C00");
#else
  service::cmb::BSI::instance()->mac(fmCfg[0].hostCfg.mac_addr);

  fmCfg[0].protocolCfg.igmpVsn = FmProtocolConfig::NoIGMP;

  {
    fmCfg[0].portCfg.ports10Mb           = FmPortMask(0);
    fmCfg[0].portCfg.ports100Mb          = FmPortMask(0);
    fmCfg[0].portCfg.ports1Gb            = FmPortMask(0);
    fmCfg[0].portCfg.ports2_5Gb          = FmPortMask(0);
    fmCfg[0].portCfg.ports4Gb            = FmPortMask(0);
    fmCfg[0].portCfg.ports10Gb           = FmPortMask(0);

    for(unsigned i=0; i<24; i++) {

      unsigned portcfg = service::cmb::BSI::instance()->switchCfg(i);
      printf("BSI port %d type %d\n",i,portcfg);

      FmPortMask mask(1<<(i+1));
      unsigned portcfg_t = (portcfg&0xff);

      if (i==7)         // Fix Zone 2 Slot1/2 to XAUI for now
        portcfg_t = 6;

      switch(portcfg_t) {
      case 6:
        fmCfg[0].portCfg.ports10Gb   |= mask;
        break;
      case 5:
        fmCfg[0].portCfg.portsRxFlip |= mask;
        fmCfg[0].portCfg.portsTxFlip |= mask;
      case 2:
        fmCfg[0].portCfg.ports1Gb    |= mask;
        break;
      case 1:
      case 0:
        break;
      default:
        printf("Unabled to handle port %d configuration type 0x%x\n",
               i, portcfg);
        break;
      }
    }
    fmCfg[0].portCfg.ports10Gb &= ~0x10040;  // exclude channels that cause interference
  }

  //  exclude zone2 -> zone2 forwarding
  {
    SwmPortFwdTable& table = fmCfg[0].swCfg.portfwd_table;
    FmPortMask z2Ports(0x5775540);
    FmPortMask nz2Ports(FmPortMask::allPorts() ^ z2Ports);
    for(unsigned k=0; k<FmPort::MAX_PORT; k++)
      if (z2Ports & (1<<(k+1)))
        table.forward[k] = nz2Ports;
      else
        table.forward[k] = FmPortMask::allPorts();
  }

  //  This is only setup for configuration via DHCP
  cm::net::Manager::board("D01");
#endif

  printf("\nFF00 configuration\n");
  fmCfg[0].print();

  unsigned nDevices=1;

  cm::net::Manager* fm = new cm::net::Manager(nDevices, fmCfg);

  printf("FmLocalMgr initialized\n");

#if 0

  //=================================
  //  Base Interface 1000-BaseT PHY
  //=================================
  BxPhy::create(fm->deviceMgr(0))->manage(FmPortMask(-1UL));

  //    CimBaseANTask* p0 = new CimBaseANTask(fm->deviceMgr(0));
  //    new OldThread("basean_task", 100, 0x4000, *p0);
#endif

  //
  //  BSD Network
  //
  printf("Network initializing\n");
  rtems_bsdnet_initialize_network();
  printf("Network initialized\n");

#ifdef DO_DHCP
  ChrDisplay::instance()->writeStr("DHCP", ChrDisplay::Rotate90);
  fm->upload_boot_parameters(0);
#endif

  rtems_bsdnet_synchronize_ntp(0,0);

  ChrDisplay::instance()->writeStr("COB", ChrDisplay::Rotate90);

  //  delete[] fmCfg;

  printf("bootFm end\n");

  new service::console::UploadManager;
  new service::console::DebugHandler;

  //  new DtmConfigTask(fm->deviceMgr(0));

  cm::shell::addCommands();
  cm::shell::addCommands(fm);

  printf("init_executive done\n");

  service::fci::Manager::create();  // Need to call create once for RceConsoleHandler
  new DtmConsoleTask(fm);
}
