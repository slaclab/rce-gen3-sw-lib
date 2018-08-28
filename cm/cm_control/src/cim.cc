#define IMAGENUM 15
//#define NEW_BOARD

#include "bldInfo.h"

#include "rtems/cim_config.hh"
#include "debug/Debug.hh"
#include "shell/ShellCommon.hh"

#include "fci/Manager.hh"
#include "ffs/IdFile.hh"

#include "concurrency/OldThread.hh"
#include "concurrency/Semaphore.hh"
#include "concurrency/Procedure.hh"
#include "time/Time.hh"

#include "control/ChrDisplay.hh"
#include "control/CimConsoleTask.hh"
#include "control/CmFwUploadTask.hh"
#include "control/DisplayTask.hh"
#include "control/PortMonitor.hh"
//#include "control/CimBaseANTask.hh"
#include "ipmi/IpmDcr.hh"
#include "ipmi/IpmTask.hh"
#include "control/SwmDebug.hh"

#include "net/Manager.hh"

#include "phy/mv88e1149.hh"
#include "phy/BxPhy.hh"
#include "phy/XfpManager.hh"
#include "fm/FmConfig.hh"
#include "fm/FmAutoNegCallback.hh"

#include "shell/ShellCommands.hh"
#include "log/Server.hh"

#include "console/UploadManager.hh"
#include "console/DebugHandler.hh"

#include <rtems.h>
#include <bsp.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <net/if.h>
#include <netinet/if_ether.h>
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
  void rce_predriver_hook() {}
}

extern "C" {
  extern void rtems_bsdnet_loopattach();
  static struct rtems_bsdnet_ifconfig loopback_config = {
    (char*)"lo0",                  /* name */
    (int (*)(struct rtems_bsdnet_ifconfig *, int))rtems_bsdnet_loopattach,
                                   /* attach function */
    NULL,                          /* link to next interface */
    (char*)"127.0.0.1",            /* IP address */
    (char*)"255.0.0.0",            /* IP net mask */
  };

  struct rtems_bsdnet_config rtems_bsdnet_config = {
    //    &fm_config,              /* Network interface */
    &loopback_config,              /* Network interface */
    NULL,                          /* Use fixed network configuration */
    80,                            /* Default network task priority */
    0,                             /* Default mbuf capacity */
    0,                             /* Default mbuf cluster capacity */
    (char*)"npa01",                /* Host name */
    (char*)"slac.stanford.edu",    /* Domain name */
    NULL,                          /* Gateway */
    NULL,                          /* Log host */
    { NULL },                      /* Name server(s) */
    { NULL }                       /* NTP server(s) */
  };
}

static cm::net::Manager* fm;

static void
setHostMacAddress(FmConfig* cfg, const cm::ffs::IdFile& idf);

//============================================

#include "fm/FmTahoe.hh"
#include "fm/FmTahoeMgr.hh"

extern "C" void init_executive()
{
  service::debug::clearMessages();
  //  opb_intc_init();

  //  Read Board Identification
#ifdef NEW_BOARD
  cm::ffs::IdFile* idf = new cm::ffs::IdFile("CMHUB","C00","255",
                                             "08:00:56:00:40:FF",
                                             "08:00:56:00:41:FF");
#else
  cm::ffs::IdFile* idf = new cm::ffs::IdFile;
#endif
  printf("Read Board ID %s %s %s\n",
         idf->name(), idf->version(), idf->serialNumber());

  //
  //  Code initialization specific to this board revision
  //
  cm::net::Manager::board(idf ? idf->version() : "C00");

  ChrDisplay::instance()->writeStr(idf->name(),ChrDisplay::Rotate90);

  //=================
  //  FM2224
  //=================

  //
  //  How to configure:
  //
  //    Image 0   : software encoded : minimal
  //    Image 1   : software encoded : maximal
  //    Image 2   : flash memory : user file A
  //    Image 3   : flash memory : user file B
  //    Image F   : network upload (dhcp)
  //

#if IMAGENUM == 0 || IMAGENUM == 1 || IMAGENUM == 15
  const int MAX_DEVICES = 2;
  FmConfig* fmCfg = new FmConfig[MAX_DEVICES];

  sprintf(fmCfg[0].hostCfg.if_name,"eth0");

  if (!strcmp(idf->version(),"C00")) {
  }
  else {
    //  Lanes are reversed on the XFP ports
    fmCfg[0].portCfg.portsRxFlip = FmPortMask(0x800000);
    fmCfg[0].portCfg.portsTxFlip = FmPortMask(0x800000);
  }

#if IMAGENUM == 0
  //
  //  This is the safe image at vector 0
  //
  //  fmCfg[0].hostCfg.ip_addr     = 0xc0a80002;
  fmCfg[0].hostCfg.ip_addr     = 0xac150626;
  fmCfg[0].hostCfg.ip_netmask  = 0xffffff00;
  fmCfg[0].protocolCfg.igmpVsn  = FmProtocolConfig::IGMPv3;

  memcpy(&fmCfg[1],&fmCfg[0],sizeof(FmConfig));
  fmCfg[1].hostCfg.ip_addr     = 0xc0a80102;

#else
#if IMAGENUM == 1
  //
  //  This is the fully enabled image at vector 1
  //
  fmCfg[0].hostCfg.ip_addr     = 0xc0a80002;
  fmCfg[0].hostCfg.ip_netmask  = 0xffffff00;
  fmCfg[0].protocolCfg.igmpVsn  = FmProtocolConfig::IGMPv3;

  if (!strcmp(idf->version(),"C00")) {
    fmCfg[0].portCfg.ports2_5Gb  = 0x2aabfe;
  }
  else {
    fmCfg[0].portCfg.ports10Gb   = 0x800000;
    //
    //  Configure for 1000-BaseT
    //
    fmCfg[0].portCfg.ports1Gb     = 0x08abfe;
    fmCfg[0].portCfg.portsRxFlip |= 0x08abfe;
    fmCfg[0].portCfg.portsTxFlip |= 0x08abfe;
  }
  fmCfg[0].portCfg.ports10Gb   = 0xd55400;

  memcpy(&fmCfg[1],&fmCfg[0],sizeof(FmConfig));

  fmCfg[1].hostCfg.ip_addr     = 0xc0a80102;
  fmCfg[1].portCfg.ports2_5Gb  = 0;
  fmCfg[1].portCfg.ports10Gb   = 0xfffffe;

#else
#if IMAGENUM == 15
  //
  //  This is the externally enabled image at vector F
  //
  fmCfg[0].hostCfg.ip_addr     = 0;          // DHCP will configure
  fmCfg[0].hostCfg.ip_netmask  = 0;
  fmCfg[0].protocolCfg.igmpVsn = FmProtocolConfig::NoIGMP;

  memcpy(&fmCfg[1],&fmCfg[0],sizeof(FmConfig));

  fmCfg[1].portCfg.ports10Gb    = FmPortMask(0x0020000);  // FCh1
  if (strcmp(idf->version(),"C00")) {
    fmCfg[0].portCfg.ports10Gb |= FmPortMask(0x800000);  // XFP
    fmCfg[1].portCfg.ports10Gb |= FmPortMask(0x800000);  // XFP
  }
  else
    fmCfg[0].portCfg.ports2_5Gb = FmPortMask(0x0000100);  // BCh2

#endif // IMAGENUM == 15
#endif // IMAGENUM == 1
#endif // IMAGENUM == 0

  sprintf(fmCfg[1].hostCfg.if_name,"eth1");
  fmCfg[1].hostCfg.mac_addr[4] = 0x7F;

#else
  //
  //  Configure from flash memory
  //
  service::fci::Manager* mgr = new service::fci::Manager;
  unsigned bootFlags = mgr->userFlags();
  service::fci::File f(bootFlags);
  unsigned file_size = mgr->fileSize(f);
  printf("Reading cfg file %d of size 0x%x\n",
 f.fileNum(),file_size);

  char* configArea = new char[file_size];
  mgr->read(f.fileNum(),configArea);
  delete mgr;

  FmConfig* fmCfg = reinterpret_cast<FmConfig*>(configArea);
#endif  // IMAGENUM==0 | 1 | 15

  //
  //  Derive MAC addresses from board ID
  //
  setHostMacAddress(fmCfg,*idf);

  //
  //  Remove the IP addresses from the interfaces (for DHCP)
  //
  //  fmCfg[0].hostCfg.ip_addr = 0;
  //  fmCfg[1].hostCfg.ip_addr = 0;

  printf("\nFF00 configuration\n");
  fmCfg[0].print();
  printf("\nFF01 configuration\n");
  fmCfg[1].print();

  unsigned nDevices=2;

  fm = new cm::net::Manager(nDevices,fmCfg);

  printf("FmLocalMgr initialized\n");

  //===============================
  //  Marvell PHY (Auxiliary RJ45)
  //===============================
#if 1
  PhyAutoNegCallback* anegCb[4];
  if (!strcmp(idf->version(),"C00")) {
    anegCb[0] = new FmAutoNegCallback(fm->deviceMgr(0),FmPort(24));
    anegCb[1] = new FmAutoNegCallback(fm->deviceMgr(1),FmPort(24));
  }
  else {
    anegCb[0] = new FmAutoNegCallback(fm->deviceMgr(1),FmPort(24));
    anegCb[1] = new FmAutoNegCallback(fm->deviceMgr(0),FmPort(24));
  }
  anegCb[2] = 0;
  anegCb[3] = 0;
  mv88e1149::instance()->enable_autonegotiation(anegCb);
#else
  {
    mv88e1149* fp_eth = mv88e1149::instance();
    fp_eth->setSpeed(0,PhyAutoNegCallback::an100Mb);
    fp_eth->setSpeed(1,PhyAutoNegCallback::an100Mb);
    fp_eth->setSpeed(2,PhyAutoNegCallback::an100Mb);
    fp_eth->setSpeed(3,PhyAutoNegCallback::an100Mb);
  }
#endif

  if (strcmp(idf->version(),"C00")) {
    //=================================
    //  Base Interface 1000-BaseT PHYs
    //=================================
    BxPhy::create(fm->deviceMgr(0))->manage(FmPortMask(-1UL));

    //    CimBaseANTask* p0 = new CimBaseANTask(fm->deviceMgr(0));
    //    new tool::concurrency::OldThread("basean_task", 100, 0x4000, *p0);

    //===============================
    //  Marvell PHY (Auxiliary XFP)
    //===============================
    XfpManager<FrontPanel>::instance();
  }

  //===============================
  //  Marvell PHY (XFP RTM)
  //===============================
  //  XfpManager<RearModule>::instance();

  //
  //  BSD Network
  //

  printf("Network initializing\n");
  rtems_bsdnet_initialize_network();
  printf("Network initialized\n");

#if IMAGENUM == 15
  { timespec ts;     //  Need time for autonegotiation to complete
    ts.tv_nsec = 0;  //  before launching DHCP requests
    ts.tv_sec  = 2;
    nanosleep(&ts,0); }
  ChrDisplay::instance()->writeStr("DHCP", ChrDisplay::Rotate90);
  fm->upload_boot_parameters(1);
#endif

  rtems_bsdnet_synchronize_ntp(0,0);

  // Create working tasks
  new tool::concurrency::OldThread("display_task", 100, 0x1000,
                                *new DisplayTask(*idf,fm->deviceMgr(0),fm->deviceMgr(1)));

  cm::ipm::IpmHandle* ipm0 = new cm::ipm::IpmDcr<cm::ipm::IPMB_0>(cm::ipm::IRQ_0);
  //  cm::ipm::IpmHandle* ipm1 = new cm::ipm::IpmDcr<cm::ipm::IPMB_1>(cm::ipm::IRQ_1);
  cm::ipm::IpmHandle* ipm1 = new cm::ipm::IpmDcr<cm::ipm::IPMB_1>(cm::ipm::IRQ_0);
  cm::ipm::IpmTask*   ipmt = new cm::ipm::IpmTask(*ipm0,*ipm1);
  new tool::concurrency::OldThread("ipm_task0", 104, 0x1000, *ipmt);

  new CimConsoleTask(*fm,*ipmt);
  new service::console::UploadManager;
  //  new service::console::DebugHandler;

//   CmFwUploadTask* p4 = new CmFwUploadTask;
//   new tool::concurrency::OldThread("fw_upload_task", 103, 0x1000, *p4);

//   new tool::concurrency::OldThread("port_monitor", 101, 0x1000,
//                                 *new PortMonitor(fm->deviceMgr(0),
//                                                  fm->deviceMgr(1)));

//   new tool::concurrency::OldThread("cimlog", 101, 0x2000,
//                                 *new cm::log::Server(strtoul(idf->version()+1,0,10),
//                                                               strtoul(idf->serialNumber(),0,10),
//                                                               fm->deviceMgr(0),
//                                                               fm->deviceMgr(1)));

  delete idf;

  // Add our custom commands
  cm::shell::addCommands();

  // Allow users to log in via telnet and execute interactive commands.
  service::shell::initialize();

  printf("init_executive done");
}

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
  setMacAddr(cfg[0].hostCfg.mac_addr,idf.macBase());
  setMacAddr(cfg[1].hostCfg.mac_addr,idf.macFabric());
}

#include <rtems/shellconfig.h>
