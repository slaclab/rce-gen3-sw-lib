#include "bldInfo.h"

#include "control/DtmConsoleTask.hh"
#include "control/ConsoleHandle.hh"

#include "net/Manager.hh"
#include "fm/FmConfig.hh"
#include "phy/Vsc8234.hh"

#include "control/ChrDisplay.hh"

#include "fm/SwmRequest.hh"
#include "fm/SwmReply.hh"
#include "control/SwmDebug.hh"

#include "fm/FmTahoe.hh"
#include "fm/FmTahoeMgr.hh"

#include "ffs/IdFile.hh"

#include "cm_svc/CmHw.hh"

#include "debug/Debug.hh"
#include "debug/Print.hh"

#include "cpu/apu.hh"


extern "C" {
#include <rtems/rtems_bsdnet.h>
}

#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <list>

#define SCANUARG(var,name) \
  unsigned var; \
  if (!sscanf(args,"%x",&var)) { \
    o.printv("Error scanning %s\n",name); \
    continue; \
  }

static const int BSIZ=0x80000;
static int addscan(ether_addr_t& addr, const char*& args);

static const unsigned IICSADDR = 0x49<<1;

static unsigned _readApuCfg();
static void     _writeApuCfg(unsigned w);

static void     _readIicCmdA(char*);
static void     _writeIicCmdA(const char* args);

static void _fabstart       (cm::net::Manager*, AbsConsole&);
static void _fabstat        (AbsConsole&);

static void _z2scan         (cm::net::Manager*, AbsConsole&, const char*);
static void _show_int_brief (cm::net::Manager*, AbsConsole&);
static void _show_stat_brief(cm::net::Manager*, AbsConsole&);

static unsigned ntries;
static const unsigned MAX_TRIES=10000;

static void _sleep(unsigned t_ms)
{
  timespec ts;
  ts.tv_sec  = t_ms/1000;
  ts.tv_nsec = (t_ms%1000)*1000000; // (20 msec should be sufficient)
}

typedef unsigned (*VALFCN)(unsigned,unsigned);
static void     _iicmstest(const char* title,
                           uint8_t     dev,
                           unsigned    nbytes,
                           VALFCN      fcn);

static bool _iicping(unsigned);
static void _iicmflush();
static void _iicmdump ();

static unsigned REG_LO=0x0010;
static unsigned REG_HI=0x0180;
static const unsigned RAPU    = 0x105;

static unsigned _i2cs_reg=0;
//static unsigned _i2cs_nb;
static char     _i2cs_msg[1024] = {0};

static char qwbuf[32];
static unsigned* _qwbuf = (unsigned*)((unsigned(qwbuf)+0xf)&~0xf);
static uint32_t _cwbuf[16];
static unsigned _cwbufi=0;

static unsigned register_value(unsigned base, unsigned off)
{ return ((base+off)&0xff)|0x80; }

static unsigned register_clear(unsigned base, unsigned off)
{ return 0xff; }

#include "control/AbsConsole.hh"
class MyConsole : public AbsConsole {
public:
  MyConsole() {}
  ~MyConsole() {}
public:
  void printv(const char* msgformat, ...) {
    va_list va;
    va_start(va,msgformat);
    vprintf(msgformat,va);
    va_end(va);
  }
  void reserve(unsigned) {}
};


DtmConsoleTask::DtmConsoleTask(cm::net::Manager* m) :
  fm   (m),
  fmCfg(new FmConfig),
  reply(new char[BSIZ])
{
  memset(reply,0,BSIZ);

  MyConsole c;
  _dumpFm(c);
}

DtmConsoleTask::~DtmConsoleTask()
{
  delete[] reply;
  delete fmCfg;
}

bool DtmConsoleTask::handle(service::console::RemoteConsole& console,
                            const char* cmd,
                            const char* args)
{
  ConsoleHandle o(console);

  try {
    do {

      if (!strcasecmp(cmd,"DISPLAY")) {
        ChrDisplay::instance()->writeStr(args,ChrDisplay::Rotate90);
      }
      else if (!strcasecmp(cmd,"WRITEID")) {
        char* name = strtok(0," ");
        char* vsn  = strtok(0," ");
        char* sno  = strtok(0," ");
        char* macB = strtok(0," ");
        char* macF = strtok(0," ");
        cm::ffs::IdFile* idf = new cm::ffs::IdFile(name,vsn,sno,macB,macF);
        o.printv("Wrote IDFILE : %s %s %s %s %s\n",
                 idf->name(), idf->version(), idf->serialNumber(),
                 idf->macBase(), idf->macFabric());
        delete idf;
      }
      else if (!strcasecmp(cmd,"READID")) {
        cm::ffs::IdFile* idf = new cm::ffs::IdFile;
        o.printv("Read IDFILE : %s %s %s %s %s\n",
                 idf->name(), idf->version(), idf->serialNumber(),
                 idf->macBase(), idf->macFabric());
        delete idf;
      }
      else if (!strcasecmp(cmd,"DUMPFM")) {
        _dumpFm(o);
      }
      else if (!strcasecmp(cmd,"REPLALL")) {
        _replAll(o);
      }
      else if (!strcasecmp(cmd,"READPHY")) {
        _basePhyMgmt(o);
      }
      else if (!strcasecmp(cmd,"SENDPKT")) {
        _genPacket(o,args);
      }
      else if (!strcasecmp(cmd,"ADDMAC")) {
        _addMac(o,args);
      }
      else if (!strcasecmp(cmd,"READMACTAB")) {
        _readMac(o,args);
      }
      else if (!strcasecmp(cmd,"RWREG")) {
        unsigned reg,val;
        if (sscanf(args,"%x %x",&reg,&val)!=2) {
          o.printv("Error scanning {register addr, value}\n");
          continue;
        }
        _rwReg(o,reg,val);
      }
      else if (!strncmp(cmd,"UP",2)) {
        unsigned portID,rxFlip=0,txFlip=0,drive=0;
        if (!sscanf(args,"%x %x %x %x",&portID,&rxFlip,&txFlip,&drive)) {
          o.printv("Error scanning port ID\n");
          continue;
        }
        FmPort port(portID);
        cm::fm::PortSpeed speed = cm::fm::None;
        if      (!strcasecmp(&cmd[2],"10G"))  speed = cm::fm::X10Gb;
        else if (!strcasecmp(&cmd[2],"4G"))   speed = cm::fm::X4Gb;
        else if (!strcasecmp(&cmd[2],"2_5G")) speed = cm::fm::X2_5Gb;
        else if (!strcasecmp(&cmd[2],"1G"))   speed = cm::fm::X1Gb;
        else if (!strcasecmp(&cmd[2],"100M")) speed = cm::fm::X100Mb;
        else if (!strcasecmp(&cmd[2],"10M"))  speed = cm::fm::X10Mb;

        fm->deviceMgr(0).configurePort(port,speed,rxFlip,txFlip,drive);
        _sleep(200);

        unsigned reply_len;
        SwmEplReadRequest* req = new SwmEplReadRequest(&fm->deviceMgr(0).dev().epl_phy[portID-1]);
        fm->process(*req, (void*)reply, reply_len);
        delete req;
      }
      else if (!strcasecmp(cmd,"DOWN")) {
        SCANUARG(portID,"port ID");
        FmPort port(portID);
        fm->deviceMgr(0).configurePort(port,cm::fm::None);
      }
      else if (!strcasecmp(cmd,"REBOOTFM")) {
        _reboot();
      }
      else if (!strcasecmp(cmd,"ETHSTATS")) {
        rtems_bsdnet_show_mbuf_stats();
        rtems_bsdnet_show_if_stats();
        rtems_bsdnet_show_ip_stats();
        rtems_bsdnet_show_udp_stats();
      }
      else if (!strcasecmp(cmd,"PORTTEST")) {
        _porttest();
      }
      else if (!strcasecmp(cmd,"CHAIN")) {
        _chain(o,args);
      }
      else if (!strcasecmp(cmd,"VSN")) {
        o.printv("HW Version: %s\n",cm::svc::CmHw::get_version_tag());
        time_t vsntim = cm::svc::CmHw::get_version_time();
        o.printv("HW Build time: %s\n",ctime(&vsntim));
        o.printv("SW Version: %s\n",cm::control::bldRevision());
        o.printv("SW Build time: %s\n",cm::control::bldDate());
      }
      else if (!strcasecmp(cmd,"GPIO")) {
        const unsigned GPIOR=0x212;
        unsigned wword;
        if (sscanf(args,"%x",&wword)!=1) {
          asm volatile("mtdcr %0, %1" : : "i"(GPIOR), "r"(wword));
        }
        else {
          unsigned v;
          asm volatile("mfdcr %0, %1" : "=r"(v) : "i"(GPIOR));
          o.printv("  %x\n",v);
        }
      }
      else if (!strcasecmp(cmd,"ENAPU")) {
        tool::cpu::APU().enable();

        o.printv("APU enabled\n");
      }
      else if (!strcasecmp(cmd,"APUCFGR")) {
        unsigned w = _readApuCfg();
        o.printv("APUCFG = %x\n",w);
      }
      else if (!strcasecmp(cmd,"APUCFGW")) {
        SCANUARG(cfg,"config");
        _writeApuCfg(cfg);
      }
      else if (!strcasecmp(cmd,"IICWRITEMA")) {
        _writeIicCmdA(args);
        continue;
      }
      else if (!strcasecmp(cmd,"IICREADMA")) {
        SCANUARG(ncmds,"ncommand");
        char buff[256];
        char* p = buff;
        p += sprintf(p,"Read IIC Cmds[%d]:",ncmds);
        for(unsigned i=0; i<ncmds; i++) {
          _readIicCmdA(p);
          p += strlen(p);
        }
        o.printv("%s\n",buff);
        continue;
      }
      else if (!strcasecmp(cmd,"IICMSTEST")) {
        SCANUARG(cmdval,"bytes");
        uint8_t dev = IICSADDR;
        for(unsigned nbytes=1; nbytes<cmdval; nbytes++) {
          _iicmstest("Clearing Registers",dev,nbytes,register_clear);
          _iicmstest("Writing Registers ",dev,nbytes,register_value);
        }
      }
      else if (!strcasecmp(cmd,"IICMFLUSH")) {
        _iicmflush();
      }
      else if (!strcasecmp(cmd,"IICBUSSCAN")) {
        char* p = reply;
        for(unsigned i=0; i<255; i++) {
          if (_iicping(i))
            p += sprintf(p,"%d",i);
          else
            *p++ = '.';
        }
        *p++ = 0;
        o.printv("%s\n",reply);
      }
      else if (!strcasecmp(cmd,"FABSTART")) {
        _fabstart(fm, o);
      }
      else if (!strcasecmp(cmd,"FABSTAT")) {
        _fabstat(o);
      }
      else if (!strcasecmp(cmd,"Z2SCAN")) {
        _z2scan        (fm, o, args);
      }
      else if (!strcasecmp(cmd,"Z2PUP")) {
        _show_int_brief(fm, o);
      }
      else if (!strcasecmp(cmd,"Z2STAT")) {
        _show_stat_brief(fm, o);
      }
    } while (0);
  } catch (tool::exception::Event& e) {}

  return true;
}

void DtmConsoleTask::_replAll(AbsConsole& o)
{
  FmTahoe* t = new(0) FmTahoe;
  unsigned reply_len;
  memset(reply,0,BSIZ);

  SwmAllPortEplRequest* req = new SwmAllPortEplRequest( t );
  fm->process(*req, (void*)reply, reply_len);
  if (reply_len) {
    const SwmAllPortEplReply& rrep = *(const SwmAllPortEplReply*)reply;
    for(unsigned j=0; j<24; j++) {
      o.printv("  Port 0x%x\n",j );
      o << rrep.result()[j];
    }
  }
  delete req;
}

void DtmConsoleTask::_dumpFm(AbsConsole& o)
{
  FmTahoe*      t = &fm->deviceMgr(0).dev();
  unsigned      reply_len;
  //  memset(reply,0,BSIZ);

  {
    SwmAllPortEplRequest* req = new SwmAllPortEplRequest( t );
    fm->process(*req, (void*)reply, reply_len);
    delete req;
    if (reply_len) {
      const SwmAllPortEplReply& rrep = *(const SwmAllPortEplReply*)reply;
      for(unsigned j=0; j<24; j++) {
        o.printv("  Port 0x%x\n",j );
        o << rrep.result()[j];
      }
    }
  }

  {
    SwmAllPortMibRequest* req = new SwmAllPortMibRequest( t );
    fm->process(*req, (void*)reply, reply_len);
    delete req;
    if (reply_len) {
      const SwmAllPortMibReply& rrep = *(const SwmAllPortMibReply*)reply;
      for(unsigned j=0; j<25; j++) {
        o.printv("  Port 0x%x\n",j );
        o << rrep.result().port(j);
      }
    }
  }

  {
    SwmGlobalMibRequest* req = new SwmGlobalMibRequest( t );
    fm->process(*req, reply, reply_len);
    delete req;
    if (reply_len) {
      const SwmGlobalMibReply& rrep = *(const SwmGlobalMibReply*)reply;
      o.printv("== MIB Global (0x%x) ==\n", rrep.reg());
      o << rrep.result();
    }
  }

  {
    SwmStreamStatusRequest* req = new SwmStreamStatusRequest( t );
    fm->process(*req, reply, reply_len);
    delete req;
    if (reply_len) {
      const SwmStreamStatusReply& rrep = *(const SwmStreamStatusReply*)reply;
      o.printv("== Stream Status ==\n");
      o << rrep.result();
    }
  }

#if 0
  {
    SwmMacTableReadRequest* req = new SwmMacTableReadRequest( t );
    fm->process(*req, reply, reply_len);
    delete req;
    if (reply_len) {
      const SwmMacTableReadReply& rrep = *(const SwmMacTableReadReply*)reply;
      o.printv("== MAC Table BEG ==\n");
      o << rrep.result();
      o.printv("== MAC Table END ==\n");
    }
  }
#endif

  {
    o.printv("== Registers ==\n");
    o.printv("sys_cfg_1 %08x\n",unsigned(t->sys_cfg_1));
    o.printv("sys_cfg_2 %08x\n",unsigned(t->sys_cfg_2));
    o.printv("sys_cfg_7 %08x\n",unsigned(t->sys_cfg_7));
  }

  printf("dumpFm end\n");
}

void DtmConsoleTask::_basePhyMgmt(AbsConsole& o)
{
  printf("basePhyMgmt begin\n");
  Vsc8234* phy = new (0)Vsc8234;
  phy->initialize();

  unsigned links=0;
  for(unsigned k=0; k<Vsc8234::NPORTS; k++) {
    static const unsigned reg[] = { 0,1,2,3,9,10,30 };
    for(unsigned j=0; j<sizeof(reg)/sizeof(unsigned); j++) {
      unsigned v = phy->dev[k].r[reg[j]];
      o.printv("dev[%d].r[%d] = %04x\n", k,reg[j],v);
    }
    if (phy->dev[k].linkStatus())
      links |= 1<<k;
  }
  o.printv("links = %x\n",links);
  printf("basePhyMgmt end\n");
}

void DtmConsoleTask::_genPacket(AbsConsole& o, const char* args)
{
  o.printv("genPacket begin\n");

  ether_addr_t src;
  src.ether_addr_octet[0] = 0x80;
  src.ether_addr_octet[1] = 0x00;
  src.ether_addr_octet[2] = 0x56;
  src.ether_addr_octet[3] = 0x00;
  src.ether_addr_octet[4] = 0x40;
  src.ether_addr_octet[5] = 0x23;

  ether_addr_t dst;
  if (!addscan(dst,args)) {
    dst.ether_addr_octet[0] = 0xff;
    dst.ether_addr_octet[1] = 0xff;
    dst.ether_addr_octet[2] = 0xff;
    dst.ether_addr_octet[3] = 0xff;
    dst.ether_addr_octet[4] = 0xff;
    dst.ether_addr_octet[5] = 0xff;
  }

  unsigned size = 1024;

  char* buf = new char[size+12];
  char* b = buf;
  memcpy(b,&dst,6); b += 6;
  memcpy(b,&src,6); b += 6;
  for(unsigned k=0; k<size; k++)
    *b++ = k&0xff;

  unsigned* ub = (unsigned*)buf;
  unsigned* ue = ub + (unsigned(b-buf)>>2);
  SwmPacket pkt(0);
  while( ub < ue )
    pkt.append(*ub++);

  fm->deviceMgr(0).dev().txPacket(pkt.data(), pkt.bytes(), FmPortMask(pkt.dst()));

  o.printv("genPacket end\n");
}

void DtmConsoleTask::_rwReg(AbsConsole& o,unsigned regaddr, unsigned val)
{
  printf("rwReg begin\n");

  FmMReg& reg = *(FmMReg*)(regaddr<<2);
  unsigned v0 = reg;

  reg = val;
  unsigned v1 = reg;

  reg = v0;
  unsigned v2 = reg;

  o.printv("reg(%x:%x) %x : %x : %x\n",regaddr,val,v0,v1,v2);

  printf("rwReg end\n");
}

void DtmConsoleTask::_reboot()
{
  printf("reboot begin\n");

  FmTahoe& _dev = fm->deviceMgr(0).dev();
  FmConfig& config = *new FmConfig;

  config.print();

  volatile unsigned v;

  //  Hardware initialization goes here
  _dev.resetChip();
  _sleep(20);

  _dev.releaseChip();
  _sleep(20);

  //  Poll boot status until zero or timeout
  { unsigned k=1024;
    do {
      v = _dev.boot_status;
    } while( v && --k );
    if (!k) {
      service::debug::printv("FmTahoeMgr boot_status poll1 timeout (%x)",v);
      return;
    }
  }

  //  Begin boot state machine
  //  ( Enable DFT/JTAG with 0xd05 )
  //  Errata item #10 "Contention between 1Gb Mode and LED Interface
  //    Port Status
  //   _dev.chip_mode = 0xd04;
  _dev.chip_mode = 0xd00;

   service::debug::printv("FmTahoeMgr(%d) waiting for boot complete",_dev.id());

   //  Poll until boot is complete
   { unsigned k=20480;
     do {
       v = _dev.boot_status;
     } while( v && --k );
     if (!k) {
       service::debug::printv("FmTahoeMgr boot status didn't clear in 20480 reads");
       return;
     }
     else
       service::debug::printv("FmTahoeMgr boot status cleared in %d reads",20480-k);
   }

   //  Read vital product data register
   {
     v = _dev.vpd;
     if ( (v & 0x0fffffff) != 0xae1842b )
       service::debug::printv("Read incorrect product code %08x (%08x)",v,0xae1842b);
   }

   //  Ref design writes and reads from shadow fusebox here as a test

   //  Program frame handler clock for 300MHz
   //  (some contradiction in the description of bit 0 for this register)
   //  pllout<<15 | m << 4 | n << 11 | p << 2
   //  p=0; m=1b; n=3  (297 MHz)
   _dev.pll_fh_ctrl = 0x19b0;

   _sleep(1000);

   //  Poll until frame handler pll locks
   { unsigned k = 10240;
     do {
       v = _dev.pll_fh_stat;
     } while ( v==0 && --k );
     if (!k) {
       service::debug::printv("FmTahoeMgr timeout waiting for FH_PLL to lock");
       return;
     }
     else
       service::debug::printv("FmTahoeMgr fh pll locked");
   }

   //  Release pll_fh
   //   service::debug::printv("FmTahoeMgr releasing FH reset");
   _dev.soft_reset = 0x0;

   if ( (v = _dev.sys_cfg_1) != 0x4ff) {
     service::debug::printv("FmTahoeMgr fail: fm_sys_cfg_1 = 0x%x != 0x4ff",v);
     return;
   }

   if ( (v = _dev.port_reset) != 0x1fffffe ) {
     service::debug::printv("FmTahoeMgr fail: port reset = 0x%x != 0x1fffffe",v);
     return;
   }

   const unsigned _testPort = 5;
   FmEplRegs& p1 = _dev.epl_phy[_testPort];
   if ( (v = p1.pcs_cfg_1) != 0 ) {
     service::debug::printv("FmTahoeMgr fail: ports in reset but EPL reg reads= 0x%x",v);
     return;
   }

   p1.pcs_cfg_1 = 0xffffffff;
   if ( (v = p1.pcs_cfg_1) != 0 ) {
     service::debug::printv("FmTahoeMgr fail: ports in reset but EPL reg allowed write");
     return;
   }

   const unsigned ports312MHz = config.portCfg.ports10Gb | config.portCfg.ports2_5Gb;
   const unsigned _defaultPorts =
     0x1 |
     config.portCfg.ports10Gb  |
     config.portCfg.ports4Gb  |
     config.portCfg.ports2_5Gb |
     config.portCfg.ports1Gb   |
     config.portCfg.ports100Mb |
     config.portCfg.ports10Mb  ;

   v = 0x1fffffe & ~ports312MHz;
   //   service::debug::printv("Setting clock selection 0x%x",v);
   _dev.port_clk_sel = v;

   //   service::debug::printv("Bringing all EPLs out of reset");
   _dev.port_reset = 0x1fffffe & ~_defaultPorts;
   //   _dev.port_reset = 0;

   for(unsigned k=1; k<=24; k++) {
     _dev.epl_phy[k-1].serdes_ctrl_1 = 0x88884444;
     const unsigned pmask = (1<<k);
     int lb     = config.portCfg.portsLoopback & (1<<k);
     int rxFlip = config.portCfg.portsRxFlip   & (1<<k);
     int txFlip = config.portCfg.portsTxFlip   & (1<<k);
     if (config.portCfg.ports10Gb & pmask)
       _dev.epl_phy[k-1].setup10Gb(lb, rxFlip, txFlip);
     else if (config.portCfg.ports4Gb & pmask)
       _dev.epl_phy[k-1].setup4Gb(lb, rxFlip, txFlip);
     else if (config.portCfg.ports2_5Gb & pmask)
       _dev.epl_phy[k-1].setup2_5Gb(lb, rxFlip, txFlip);
     else if (config.portCfg.ports1Gb & pmask)
       _dev.epl_phy[k-1].setup1Gb(lb, rxFlip, txFlip);
     else if (config.portCfg.ports100Mb & pmask)
       _dev.epl_phy[k-1].setup100Mb(lb, rxFlip, txFlip);
     else if (config.portCfg.ports10Mb & pmask)
       _dev.epl_phy[k-1].setup10Mb(lb, rxFlip, txFlip);
     else
       _dev.epl_phy[k-1].disable();
   }

   //  Set default watermarks (jitter [bug#6892] and pause [bug#6896])
   //   _dev.jitter_cfg = 0x00181818;
   //  Increase jitter watermark to avoid rare tx uflow
   _dev.jitter_cfg = 0x00383818;
  _dev.queue_cfg_3 = 0xffffffff;  // no WRED
   for(unsigned k=0; k<=24; k++) {
     //     if (_defaultPorts & (1<<k))
     _dev.queue_cfg_2[k] = 0x10; // rx private = 16kB
     //     _dev.global_pause_wm[k] = 0x0fff0fff;  // large pause watermark?
     _dev.global_pause_wm[k] = 0x01200144;
     _dev.rx_pause_wm    [k] = 0xe500ef;  // pause hog watermarks for jumbo frames
   }

   //  Set in FmMacTable.cc
   //   _dev.ma_table_cfg = 1;    // best-effort lookup

   //  Setup forwarding tables
   //   service::debug::printv("FmTahoeMgr writing FID entries");
   for(unsigned k=0; k<FmSwitchConfig::Entries; k++)
     _dev.fid_table[k] = config.swCfg.fid_table[k];

   //   service::debug::printv("FmTahoeMgr writing VID entries");
   for(unsigned k=0; k<FmSwitchConfig::Entries; k++)
     _dev.vid_table[k] = config.swCfg.vid_table[k];

   // Turn off learning
//    for(unsigned k=0; k<24; k++) {
//      v = _dev.port_cfg_1[k];
//      _dev.port_cfg_1[k] = v &~ 0x100000;
//    }

   //  MAC table entry interrupts
   const unsigned mask = ( FmTahoe::Frame_Ctrl_Ip::Entry_Aged |
                           FmTahoe::Frame_Ctrl_Ip::Entry_Learned |
                           FmTahoe::Frame_Ctrl_Ip::Bin_Full |
                           FmTahoe::Frame_Ctrl_Ip::FIFO_Full );

   _dev.frame_ctrl_im = _dev.frame_ctrl_im & ~mask;
   //   _dev.frame_ctrl_im = 0;
   _dev.mgr_im = 0;
   _dev.perr_im = 0;
   _dev.lci_im = FmTahoe::Lci_Ip::RxEOT | FmTahoe::Lci_Ip::RxRdy;
   _dev.port_vlan_im_1 = 0;
   _dev.port_vlan_im_2 = 0;
   _dev.port_mac_sec_im = 0;
   _dev.trigger_im = 0;

   //  Clear interrupt status registers
   v = _dev.global_epl_int_detect;
   v = _dev.lci_ip;
   v = _dev.interrupt_detect;

   //
   //  Setup the CPU interface
   //
   v = _dev.lci_cfg;
   v |= FmTahoe::Lci_Cfg::BigEndian;  //  Setup the CPU as big-endian
   v |= FmTahoe::Lci_Cfg::TxCompCRC;  //  Compute the CRC for tx packets
   v |= FmTahoe::Lci_Cfg::RxEnable;   //  Enable the receipt of packets
  _dev.lci_cfg = v;

  _dev.sys_cfg_7          = 0x7530;   // age entries once/2.4ms, whole table/40s
  _dev.global_pause_wm[0] = 0xffffffff;
  _dev.rx_pause_wm[0]     = 0xffffffff;  // no pause for CPU
  //  _dev.queue_cfg_2[0]     = 0x3ff;       // large private queue for CPU
  _dev.queue_cfg_5        = 1000;
  for(int i=0; i<=24; i++)
    _dev.queue_cfg_1[i] = 0x03ff02ff;  // allow hog ports   { [9:0] = 0xff (works), 0x3ff (broken) }

  _dev.frame_time_out     = 0x989680;  // 5 minutes

  //  The default is to send broadcasts and control protocol addresses to the CPU
  //  _dev.sys_cfg_1 |= 0x40;  // send bcasts to cpu
  //  _dev.sys_cfg_1 |= (1<<14);  // do not flood multicasts

  //  setSAFMatrix();

  //
  //  Configure the triggers
  //
  //  for(unsigned k=0; k<16; k++)
  //    configureTrigger(k,cfg.trigCfg[k]);

  *fmCfg = config;
  delete& config;

  printf("reboot end\n");
}

void DtmConsoleTask::_porttest()
{
  FmTahoe* t = new(0) FmTahoe;
  FmPortMask up_ports(0);

  //  turn off all non-connected ports
  unsigned r = t->port_reset;
  printf("port_reset %08x\n",r);

  for(unsigned j=1; j<=24; j++) {
    if (!(r & (1<<j)) ) {
      unsigned u = t->epl_phy[j-1].pcs_ip;
      printf("pcs_ip[%d] %08x\n",j,u);
      if (!(u & 0x2000))
        fm->deviceMgr(0).configurePort(FmPort(j), cm::fm::None);
      else
        up_ports |= 1<<j;
    }
  }

  _dumpSerdes(up_ports);

  for(unsigned j=1; j<=24; j++) {
    if (!up_ports.contains(FmPort(j))) {
      fm->deviceMgr(0).configurePort(FmPort(j), cm::fm::X10Gb);

      FmPortMask ports(up_ports);
      ports |= (1<<j);
      _dumpSerdes(ports);

      fm->deviceMgr(0).configurePort(FmPort(j), cm::fm::None);
    }
  }

  //  Restore ports
  for(unsigned j=1; j<=24; j++) {
    if (!(r & (1<<j))) {
      fm->deviceMgr(0).configurePort(FmPort(j), cm::fm::X10Gb);
    }
  }
}

void DtmConsoleTask::_dumpSerdes(const FmPortMask& up_ports)
{
  FmTahoe* t = new(0) FmTahoe;

  //  clear counters of all connected ports
  for(unsigned j=1; j<=24; j++) {
    if (up_ports.contains(FmPort(j))) {
      unsigned u = t->epl_phy[j-1].serdes_ip;
    }
  }

  //  run for 100ms
  _sleep(100);

  printf("== UP PORTS %08x ==\n", unsigned(up_ports));

  //  read counters of all connected ports
  for(unsigned j=1; j<=24; j++) {
    if (up_ports.contains(FmPort(j))) {
      unsigned u = t->epl_phy[j-1].serdes_ip;
      printf("port[%d] : %08x\n",j,u);
    }
  }
}

void DtmConsoleTask::_addMac(AbsConsole& o, const char* args)
{
  ether_addr_t addr;
  unsigned portmask, vlan, trigId;
  char lockChar;
  if (!addscan(addr,args) ||
      sscanf(args,"%x %x %x %c",
             &portmask, &vlan, &trigId, &lockChar )!=4) {
    o.printv("Error scanning MAC entry\n");
    return;
  }
  SwmMacTableEntry e(addr,
                     FmPortMask(portmask),
                     vlan,
                     trigId,
                     lockChar=='S'? 1:0,1);

  FmTahoe* t = new(0) FmTahoe;
  unsigned reply_len;
  memset(reply,0,BSIZ);

  SwmMacEntryWriteRequest* req = new SwmMacEntryWriteRequest( t, e );
  fm->process(*req, reply, reply_len);
  delete req;
  if (reply_len) {
  }
}

void DtmConsoleTask::_readMac(AbsConsole& o, const char* args)
{
  FmTahoe* t = new(0) FmTahoe;
  unsigned reply_len;
  memset(reply,0,BSIZ);

  SwmMacTableReadRequest* req = new SwmMacTableReadRequest( t );
  fm->process(*req, reply, reply_len);
  delete req;
  if (reply_len) {
    const SwmMacTableReadReply& rrep = *(const SwmMacTableReadReply*)reply;
    o.printv("== MAC Table BEG ==\n");
    o << rrep.result();
    o.printv("== MAC Table END ==\n");
  }
}

void DtmConsoleTask::_chain(AbsConsole& o, const char*)
{
  // <enable> <port1> <port2> ...

  unsigned uargs[26];

  unsigned nargs = 0;
  const char* args = strtok(0," ");
  while ( args ) {
    sscanf(args,"%x",&uargs[nargs++]);
    args = strtok(0," ");
  }

  if (nargs<2) return;

  unsigned nports = nargs-2;

  FmTahoe* t = new(0) FmTahoe;
  for(unsigned k=0; k<nports; k++) {
   if (uargs[0]==0) {
      o.printv("%x->ALL\n", uargs[k+1], unsigned(FmPortMask::allPorts()));
      t->port_cfg_2[uargs[k+1]] = FmPortMask::allPorts();
    }
    else {
      o.printv("%x->%x\n", uargs[k+1], uargs[k+2]);
      t->port_cfg_2[uargs[k+1]] = 1<<(uargs[k+2]);
    }
  }
}

int addscan(ether_addr_t& addr, const char*& args)
{
  unsigned a[6];
  if (sscanf(args,"%x:%x:%x:%x:%x:%x",
             &a[0],
             &a[1],
             &a[2],
             &a[3],
             &a[4],
             &a[5])!=6)
    return 0;

  args = strtok(0," ");
  args = &args[strlen(args)+1];

  for(unsigned k=0;k<6;k++)
    addr.ether_addr_octet[k]=a[k];

  return 1;
}

unsigned _readApuCfg()
{
  unsigned w;
  asm volatile("mfdcr %0, %1" : "=r"(w) : "i"(RAPU));
  asm volatile("eieio");
  return w;
}

void _writeApuCfg(unsigned w)
{
  asm volatile("mtdcr %0, %1" : : "i"(RAPU), "r"(w));
  asm volatile("eieio");
}

void _readIicCmdA(char* p)
{
  uint8_t* wbuf = reinterpret_cast<uint8_t*>(_qwbuf);

  asm volatile("stqfcmx 0,%0,%1" : : "r"(0), "r"(wbuf));

  unsigned nbyt = wbuf[15^3];
  p += sprintf(p,"(%02x)",nbyt);

  for(unsigned i=0; i<nbyt; i++)
    p += sprintf(p," %02x",wbuf[i^3]);
}

void _writeIicCmdA(const char* args)
{
  uint8_t* cmd = reinterpret_cast<uint8_t*>(_qwbuf);

  uint8_t nbyt;
  args = strtok(0," ");
  if (!sscanf(args,"%hhx",&nbyt))
    return;

  for(int i=0; i<nbyt; i++)
    if (!sscanf(strtok(0," "),"%hhx",&cmd[i^3]))
      continue;

  cmd[15^3] = nbyt;

  //  I suspect that the PPC405 errata describes a problem I've
  //  seen where the APU can generate neither a WRITEBACKOK nor
  //  FLUSH signal (hanging the FCM). Described software fixes
  //  use a "nop" instruction before/after the "lqfcmx".
  asm volatile("nop");
  asm volatile("lqfcmx 0,%0,%1" : : "r"(0), "r"(cmd));
  asm volatile("nop");
}

static void _iicmflush()
{
  uint32_t* cmd = reinterpret_cast<uint32_t*>(_qwbuf);
  unsigned nbyt=0x80;
  unsigned ntries;
  memset(cmd,0,16);
  for(ntries=0; ntries<100000 && nbyt>=0x80; ntries++) {
    asm volatile("nop");
    asm volatile("stqfcmx 0,%0,%1" : : "r"(0), "r"(cmd));
    asm volatile("nop");
    nbyt = cmd[3]>>24;

    printf("pop  %08lx:%08lx:%08lx:%08lx [%d]: ",
           cmd[0],cmd[1],cmd[2],cmd[3],ntries);
    printf(" %02lx:%02lx:%02lx:%02lx\n",
           (cmd[3]>>18)&0x3f,
           (cmd[3]>>12)&0x3f,
           (cmd[3]>> 6)&0x3f,
           (cmd[3]>> 0)&0x3f);
  }
}

//#define DBUG

static void _iicmapu()
{
  uint32_t* cmd = reinterpret_cast<uint32_t*>(_qwbuf);
#ifdef DBUG
  printf("push %08x:%08x:%08x:%08x\n",cmd[0],cmd[1],cmd[2],cmd[3]);
#endif
  asm volatile("eieio");
  asm volatile("nop");
  asm volatile("lqfcmx 0,%0,%1" : : "r"(0), "r"(cmd));
  asm volatile("nop");
  unsigned nbyt=0;
  memset(cmd,0,16);
  for(ntries=0; ntries<MAX_TRIES && nbyt<0x80; ntries++) {
    asm volatile("eieio");
    asm volatile("nop");
    asm volatile("stqfcmx 0,%0,%1" : : "r"(0), "r"(cmd));
    asm volatile("nop");
    nbyt = cmd[3]>>24;
  }
#ifdef DBUG
  printf("pop  %08x:%08x:%08x:%08x [%d]: ",
         cmd[0],cmd[1],cmd[2],cmd[3],ntries);
  printf(" %02x:%02x:%02x:%02x\n",
         (cmd[3]>>18)&0x3f,
         (cmd[3]>>12)&0x3f,
         (cmd[3]>> 6)&0x3f,
         (cmd[3]>> 0)&0x3f);
#endif
  _cwbuf[_cwbufi] = cmd[3];
  _cwbufi = (_cwbufi+1)&0xf;
}

void _iicmstest(const char* title,
                uint8_t     dev,
                unsigned    nbytes,
                VALFCN      fcn)
{
  uint8_t* cmd = reinterpret_cast<uint8_t*>(_qwbuf);
  _i2cs_msg[0] = 0;

  printf("%s : %d bytes per op\n",title,nbytes);
  for(uint16_t i=REG_LO; i<=REG_HI; i+=nbytes) {
    unsigned nb = (REG_HI-i < nbytes) ? REG_HI-i : nbytes;
    _i2cs_reg = i;

    cmd[0^3] = dev;
    cmd[1^3] = i&0xff;
    cmd[2^3] = i>>8;
    for(unsigned j=0; j<nb; j++)
      cmd[(j+3)^3] = fcn(i,j);
    cmd[15^3] = nb+3;
    _iicmapu();
  }

  _i2cs_reg = 0;
  if (_i2cs_msg[0])
    printf("%s\n",_i2cs_msg);

  for(int k=0; k<2; k++) {
    printf("Verifying %d\n",k+1);

    cmd[0^3]  = dev;
    cmd[1^3]  = REG_LO&0xff;
    cmd[2^3]  = REG_LO>>8;
    cmd[15^3] = 3;
    _iicmapu();

    for(uint16_t i=REG_LO; i<=REG_HI-nbytes; i+=nbytes) {
      unsigned nb = (REG_HI-i < nbytes) ? REG_HI-i : nbytes;

      cmd[0^3]  = dev|1;
      cmd[15^3] = nb+1;
      _iicmapu();

      for(unsigned j=0; j<nb; j++) {
        unsigned ra = (_qwbuf[3]>>18);
        unsigned wa = (_qwbuf[3]>>12);
        if (cmd[(j+1)^3] != fcn(i,j) ||
            ((ra+1)&0x3f) != (wa&0x3f)) {
          printf("Read[%x] %02x(%02x) [%d]",
                 i+j,cmd[(j+1)^3],fcn(i,j), ntries);
          printf(" : %08x %08x %08x %08x",
                 _qwbuf[0],_qwbuf[1],_qwbuf[2],_qwbuf[3]);
          printf(" %02x:%02x:%02x:%02x\n",
                 (_qwbuf[3]>>18)&0x3f,
                 (_qwbuf[3]>>12)&0x3f,
                 (_qwbuf[3]>> 6)&0x3f,
                 (_qwbuf[3]>> 0)&0x3f);
          for(unsigned i=0; i<16; i++) {
            unsigned v = _cwbuf[(_cwbufi+i)&0xf];
            printf("\t%02x:%02x:%02x:%02x\n",
                   (v>>18)&0x3f,
                   (v>>12)&0x3f,
                   (v>> 6)&0x3f,
                   (v>> 0)&0x3f);
          }
          _iicmdump();
          _iicmflush();
          return;
        }
      }
    }
  }

  printf("Verifying 3\n");
  for(uint16_t i=REG_LO; i<=REG_HI-nbytes; i+=nbytes) {
    unsigned nb = (REG_HI-i < nbytes) ? REG_HI-i : nbytes;

    cmd[0^3]  = dev;
    cmd[1^3]  = i&0xff;
    cmd[2^3]  = i>>8;
    cmd[15^3] = 3;
    _iicmapu();

    cmd[0^3]  = dev|1;
    cmd[15^3] = nb+1;
    _iicmapu();

    for(unsigned j=0; j<nb; j++) {
      unsigned ra = (_qwbuf[3]>>18);
      unsigned wa = (_qwbuf[3]>>12);
      if (cmd[(j+1)^3] != fcn(i,j) ||
          ((ra+1)&0x3f) != (wa&0x3f)) {
        printf("Read[%x] %02x(%02x) [%d]",
               i+j,cmd[(j+1)^3],fcn(i,j), ntries);
        printf(" : %08x %08x %08x %08x",
               _qwbuf[0],_qwbuf[1],_qwbuf[2],_qwbuf[3]);
        printf(" %02x:%02x:%02x:%02x\n",
               (_qwbuf[3]>>18)&0x3f,
               (_qwbuf[3]>>12)&0x3f,
               (_qwbuf[3]>> 6)&0x3f,
               (_qwbuf[3]>> 0)&0x3f);
        for(unsigned i=0; i<16; i++) {
          unsigned v = _cwbuf[(_cwbufi+i)&0xf];
          printf("\t%02x:%02x:%02x:%02x\n",
                 (v>>18)&0x3f,
                 (v>>12)&0x3f,
                 (v>> 6)&0x3f,
                 (v>> 0)&0x3f);
        }
        _iicmdump();
        _iicmflush();
        return;
      }
    }
  }

#if 0
  printf("Verifying 3\n");
  toRepeat.sort();
  toRepeat.unique();
  for(std::list<uint16_t>::const_iterator it=toRepeat.begin();
      it!=toRepeat.end(); it++) {
    uint16_t i = (*it);
    cmd[0^3] = dev|1;
    cmd[1^3] = i&0xff;
    cmd[2^3] = i>>8;


    i2cBus->write(dev, 2, toWrite);

    toRead[0] = 0;
    i2cBus->read(dev, 1, toRead);
    if (toRead[0] != fcn(i,0)) {
      printf("Read[%x] %02x(%02x)\n",i,toRead[0],fcn(i,0));
    }
  }
#endif
}

bool _iicping(unsigned addr)
{
  uint8_t dev = IICSADDR;
  uint8_t* cmd = reinterpret_cast<uint8_t*>(_qwbuf);
  _i2cs_msg[0] = 0;

  cmd[0^3]  = dev|1;
  cmd[15^3] = 1;
  _iicmapu();

  return false;
}

void _iicmdump()
{
  const int APUDMP = 0x234;
  printf("      : 0....5....0....5....0....5....0T\n");
  for(unsigned i=0; i<512; i++) {
    unsigned v;
    asm volatile("mfdcr %0, %1" : "=r"(v) : "i"(APUDMP));
    asm volatile("eieio");
    printf("[%03d] : ",i);
    for(unsigned j=0; j<32; j++)
      printf("%c", ((v>>j)&1) ? 'X':'.');
    printf("\n");
  }

  unsigned v = 0;
  asm volatile("mtdcr %0, %1" : : "i"(APUDMP), "r"(v));
  asm volatile("eieio");
}

void _z2scan(cm::net::Manager* fm, AbsConsole& o, const char* args)
{
  unsigned speed  = strtoul(args=strtok(0," "),NULL,16);
  unsigned rxflip = strtoul(args=strtok(0," "),NULL,16);
  unsigned txflip = strtoul(args=strtok(0," "),NULL,16);

  FmPortMask mask(0);
  std::list<int> ports;
  while( (args=strtok(0," ")) ) {
    unsigned p = strtoul(args,NULL,16);
    ports.push_back(p);
    mask |= 1<<p;
  }

  for(int k=0; k<=FmPort::MAX_PORT; k++) {
    unsigned m;
    if (mask.contains(FmPort(k))) {
      for(std::list<int>::const_iterator it=ports.begin(); it!=ports.end(); it++) {
        if (*it == k) {
          if (++it == ports.end())
            m = 1<<*(ports.begin());
          else
            m = 1<<*it;
          break;
        }
      }
    }
    else
      m = unsigned(FmPortMask::allPorts()) ^ unsigned(mask);

    o.printv("port %x -> %05x\n",k,m);
    fm->deviceMgr(0).dev().port_cfg_2[k] = FmPortMask(m);
  }

  for(std::list<int>::const_iterator it=ports.begin(); it!=ports.end(); it++) {
    FmPort port(*it);
    //    cm::fm::PortSpeed speed = cm::fm::X10Gb;
    fm->deviceMgr(0).configurePort(port,(cm::fm::PortSpeed)speed,rxflip,txflip,0);
  }
}

static const char* line_sep = "---------------------";

#define _enabled(p) fm->deviceMgr(0).portSpeed(FmPort(p))!=cm::fm::None

typedef struct {
  const char* name;
  unsigned    port;
} DtmInterface_t;

static DtmInterface_t _interfaces[] = { { "sl 1/2",  8 },
                                        { "sl 3"  ,  6 },
                                        { "sl 4"  , 12 },
                                        { "sl 5"  , 10 },
                                        { "sl 6"  , 14 },
                                        { "sl 7"  , 20 },
                                        { "sl 8"  , 22 },
                                        { "sl 9"  , 16 },
                                        { "sl 10" , 24 },
                                        { "sl 11" , 18 },
                                        { "sl 12" , 23 },
                                        { "sl 13" , 17 },
                                        { "sl 14" , 21 },
                                        { NULL, 0 } };

void _show_int_brief(cm::net::Manager* fm, AbsConsole& o)
{
  static const char* titles[] = { "interface",
                                  "enabled",
                                  "status",
                                  NULL };

  const FmPortMask up_ports = fm->deviceMgr(0).portsUp();

  for(unsigned i=0; titles[i]!=NULL; i++)
    o.printv("%s%c",titles[i],titles[i+1] ? '\t':'\n');
  for(unsigned i=0; titles[i]!=NULL; i++)
    o.printv("%.*s%c",strlen(titles[i]),line_sep,titles[i+1] ? '\t':'\n');
  for(unsigned i=0; _interfaces[i].name!=NULL; i++) {
    o.printv("%-*s\t", strlen(titles[0]), _interfaces[i].name);
    o.printv("%*s\t" , strlen(titles[1]), (_enabled(_interfaces[i].port)) ? "T":"F");
    o.printv("%*s\n" , strlen(titles[2]), (up_ports.contains(FmPort(_interfaces[i].port))) ? "UP":"DOWN");
  }
}

void _show_stat_brief(cm::net::Manager* fm, AbsConsole& o)
{
  static const char* titles[] = { "interface",
                                  "in packets",
                                  "out packets",
                                  "in errors",
                                  "out errors",
                                  NULL };

  char* reply = new char[0x80000];
  unsigned reply_len;
  SwmAllPortMibRequest* req = new SwmAllPortMibRequest( &fm->deviceMgr(0).dev() );
  fm->process(*req, (void*)reply, reply_len);
  delete req;
  if (!reply_len) {
    o.printv("Error fetching port statistics\n");
    delete[] reply;
    return;
  }

  const SwmAllPortMibReply& rrep = *(const SwmAllPortMibReply*)reply;

  for(unsigned i=0; titles[i]!=NULL; i++)
    o.printv("%s%c",titles[i],titles[i+1] ? '\t':'\n');
  for(unsigned i=0; titles[i]!=NULL; i++)
    o.printv("%.*s%c",strlen(titles[i]),line_sep,titles[i+1] ? '\t':'\n');
  for(unsigned i=0; _interfaces[i].name!=NULL; i++) {
    const SwmPortMibCounters& c = rrep.result().port(_interfaces[i].port);
    o.printv("%-*s\t", strlen(titles[0]), _interfaces[i].name);
    unsigned long long rxPkts = c.rxUcast + c.rxBcast + c.rxMcast;
    o.printv("%*lld\t" , strlen(titles[1]), rxPkts);
    unsigned long long txPkts = c.txUnicast + c.txBroadcast + c.txMulticast;
    o.printv("%*lld\t" , strlen(titles[2]), txPkts);
    unsigned long long rxErrs = c.rxFcsErrors + c.rxSymbolErrors;
    o.printv("%*lld\t" , strlen(titles[3]), rxErrs);
    unsigned long long txErrs = c.txTimeoutDrops + c.txErrorDrops;
    o.printv("%*lld\n" , strlen(titles[4]), txErrs);
  }
  delete[] reply;
}

static void _fabstart(cm::net::Manager* fm, AbsConsole& o)
{
  FmPort port(4);
  cm::fm::PortSpeed speed = cm::fm::X1Gb;
  fm->deviceMgr(0).configurePort(port,speed,0,0,0);

  const unsigned ADDR = 0x230;
  unsigned v = (1<<16);
  asm volatile("mtdcr %0, %1" : : "i"(ADDR), "r"(v));

  _sleep(10);

  v = 0;
  asm volatile("mtdcr %0, %1" : : "i"(ADDR), "r"(v));

  _sleep(10);
  _fabstat(o);

  //  Send out a gratuitous arp?
}

static void _fabstat(AbsConsole& o)
{
  const unsigned ADDR0 = 0x232;
  const unsigned ADDR1 = 0x233;

  unsigned v;
  asm volatile("mfdcr %0, %1" : "=r"(v) : "i"(ADDR0));
  o.printv("  reset     %d\n", (v>>31)&1);
  o.printv("  mgt_lockd %d %d\n", (v>>29)&1,(v>>28)&1);
  o.printv("  sync_acq  %d %d\n", (v>>26)&1,(v>>24)&1);
  o.printv("  mgt_reset %d %d\n", (v>>7)&1, (v>>15)&1);
  o.printv("  rx_rdy    %d %d\n", (v>>5)&1, (v>>13)&1);
  o.printv("  tx_rdy    %d %d\n", (v>>4)&1, (v>>12)&1);
  o.printv("  rx_lock   %d %d\n", (v>>2)&1, (v>>10)&1);
  o.printv("  tx_lock   %d %d\n", (v>>0)&1, (v>> 8)&1);

  asm volatile("mfdcr %0, %1" : "=r"(v) : "i"(ADDR1));
  o.printv("  rxEndAddr %08x\n",v);
}
